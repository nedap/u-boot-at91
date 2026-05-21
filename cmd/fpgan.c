// SPDX-License-Identifier: GPL-2.0+
/*
 * Nedap Renos FPGA helper commands.
 *
 * Copyright (C) 2012 Jaap de Jong <jaap.dejong@nedap.com>
 */

#include <command.h>
#include <env.h>
#include <linux/delay.h>
#include <time.h>
#include <vsprintf.h>
#include <watchdog.h>
#include <atmel_usart2.h>

static int fpga_puts(const char *s, char *r, int length)
{
	const long max_ratio = 7;
	const long delay = 2500;
	long i = delay;
	int len = 0;
	char c;

	usart2_readl(RHR);

	while (1) {
		if (i < max_ratio * delay && *s &&
		    (usart2_readl(CSR) & USART2_BIT(TXRDY)))
			usart2_writel(THR, *s++);

		if (usart2_readl(CSR) & USART2_BIT(RXRDY)) {
			len++;
			c = usart2_readl(RHR);
			if (!r) {
				printf("%c", c);
			} else if (length > 1) {
				*r++ = c;
				*r = 0;
				length--;
			}
			i += delay;
		} else if (i-- == 0) {
			return len;
		}

		if ((i & 0xff) == 0)
			schedule();
	}
}

static void fpga_flush_rx(void)
{
	unsigned long last_rx = get_timer(0);

	while (get_timer(last_rx) < 20) {
		if (usart2_readl(CSR) & USART2_BIT(RXRDY)) {
			usart2_readl(RHR);
			last_rx = get_timer(0);
		}
	}
}

static int cmd_fpgatst(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	const int answer_length = 7;
	const int data_index = 3;
	const char fpga_version[] = "0e";
	const char sync_master[] = "w320001";
	const char test_adc[] = "r31";
	const char ok_level[] = "6000";
	const char read_cmd[] = "r";
	const int first_spectrum = 196;
	const int last_spectrum = 219;
	char snd[answer_length];
	char rcv[answer_length + 3];
	int i;
	int n;
	int j;
	int attempt;

	if (argc != 1)
		return cmd_usage(cmdtp);

	printf("RF test: ");

	attempt = 1;
	while (1) {
		do {
			if (attempt++ > 500)
				goto error;
			n = fpga_puts(read_cmd, rcv, sizeof(rcv));
		} while (n != 1 || rcv[0] != read_cmd[0]);
		n = fpga_puts(fpga_version, rcv, sizeof(rcv));
		if (n == answer_length - 1 &&
		    strncmp(rcv, fpga_version, strlen(fpga_version)) == 0)
			break;
	}

	attempt = 1;
	while (1) {
		if (attempt++ > 50)
			goto error;
		n = fpga_puts(test_adc, rcv, sizeof(rcv));
		for (j = 0; j < n && rcv[j] != read_cmd[0]; ++j)
			;
		if (n - j == answer_length)
			break;
	}

	i = 0;
	for (j += data_index; j < n; ++j)
		i += (rcv[j] != '0');
	if (i < 3) {
		printf("no ADC found\n\n");
		return 0;
	}

	fpga_puts(sync_master, rcv, sizeof(rcv));
	if (strcmp(rcv, sync_master) != 0)
		goto error;

	attempt = 1;
	sprintf(snd, "%s%02x", read_cmd, first_spectrum);
	while (1) {
		if (attempt++ > 50)
			goto error;
		n = fpga_puts(snd, rcv, sizeof(rcv));
		for (j = 0; j < n && rcv[j] != read_cmd[0]; ++j)
			;
		if (strncmp(&rcv[j + data_index], ok_level,
			    strlen(ok_level)) != 0)
			break;
	}

	attempt = 1;
	while (1) {
		if (attempt++ > 5)
			goto error;
		for (i = first_spectrum; i <= last_spectrum; ++i) {
			sprintf(snd, "%s%02x", read_cmd, i);
			n = fpga_puts(snd, rcv, sizeof(rcv));
			for (j = 0; j < n && rcv[j] != read_cmd[0]; ++j)
				;
			if (strncmp(&rcv[j + data_index], ok_level,
				    strlen(ok_level)) >= 0) {
				printf("OK\n\n");
				return 0;
			}
		}
	}

error:
	printf("ERROR\n\n");
	return 1;
}

U_BOOT_CMD(fpgatst, 1, 1, cmd_fpgatst,
	   "test fpga",
	   "fpgatst");

static int cmd_fpgaw(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	if (argc != 2)
		return cmd_usage(cmdtp);

	fpga_puts(argv[1], NULL, 0);
	printf("\n");

	return 0;
}

U_BOOT_CMD(fpgaw, 2, 1, cmd_fpgaw,
	   "write to fpga port",
	   "fpgaw <value>");

static int cmd_fpgaser(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	unsigned long size = 0;
	const char *p;
	char cmd[10];

	if (argc != 1)
		return cmd_usage(cmdtp);

	p = env_get("serial#");
	if (p) {
		while (*p) {
			if (size++)
				sprintf(cmd, "+%c%c", *p, *(p + 1));
			else
				sprintf(cmd, "s20%c%c", *p, *(p + 1));
			fpga_puts(cmd, NULL, 0);
			p += 2;
		}
	}

	printf("\n");

	return 0;
}

U_BOOT_CMD(fpgaser, 1, 1, cmd_fpgaser,
	   "write serial# to fpga",
	   "fpgaser");

static void fpga_write_serial(const char *serial)
{
	unsigned long size = 0;
	char cmd[10];
	const char *p = serial;

	while (*p && *(p + 1)) {
		if (size++)
			sprintf(cmd, "+%c%c", *p, *(p + 1));
		else
			sprintf(cmd, "s20%c%c", *p, *(p + 1));
		fpga_puts(cmd, NULL, 0);
		p += 2;
	}
}

static int fpga_verify_serial(const char *serial)
{
	const char verify_serial[] = "f38";
	char rcv[32];
	int n;
	int i;

	fpga_flush_rx();
	fpga_write_serial(serial);
	mdelay(1000);
	fpga_flush_rx();
	n = fpga_puts(verify_serial, rcv, sizeof(rcv));
	for (i = 0; i + 4 < n; ++i) {
		if (rcv[i] == 'f' && rcv[i + 1] == '3' &&
		    rcv[i + 2] == '8' && rcv[i + 3] == '0')
			return (rcv[i + 4] == '1') ? 0 : 1;
	}

	return 1;
}

static int cmd_fpgagetser(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	const char read_serial[] = "f10+++++++++++++++";
	char rcv[64];
	char serial[40];
	int n;
	int i;
	int j;
	int seen_prefix;
	int attempt;

	if (argc != 1)
		return cmd_usage(cmdtp);

	mdelay(1000);
	for (attempt = 0; attempt < 3; ++attempt) {
		j = 0;
		seen_prefix = 0;
		fpga_flush_rx();
		n = fpga_puts(read_serial, rcv, sizeof(rcv));
		for (i = 0; i + 2 < n; ++i) {
			if (rcv[i] == 'f' && rcv[i + 1] == '1' &&
			    rcv[i + 2] == '0') {
				i += 3;
				seen_prefix = 1;
				break;
			}
		}

		if (!seen_prefix) {
			mdelay(250);
			continue;
		}

		for (; i < n && j < sizeof(serial) - 1; ++i) {
			char c = rcv[i];

			if ((c >= '0' && c <= '9') ||
			    (c >= 'A' && c <= 'Z') ||
			    (c >= 'a' && c <= 'z')) {
				serial[j++] = c;
				continue;
			}

			if (c == '+')
				continue;
			if (j > 0)
				break;
		}

		serial[j] = 0;
		if (j == 0) {
			mdelay(250);
			continue;
		}

		if (fpga_verify_serial(serial) != 0) {
			if (attempt == 2)
				printf("ERROR: recovered serial# failed FPGA verify\n");
			mdelay(250);
			continue;
		}

		env_set("serial#", serial);
		printf("Recovered serial#: %s\n", serial);
		return 0;
	}

	printf("ERROR: could not read serial# from FPGA\n");
	return 1;
}

U_BOOT_CMD(fpgagetser, 1, 1, cmd_fpgagetser,
	   "read serial# from fpga into the environment",
	   "fpgagetser");

static int cmd_fpgaset(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
#define is_field_separator(c)	((c) == ',')
#define is_separator(c)		((c) == ';')
#define is_line_end(c)		((c) == '\n')
#define is_meta_char(c) \
	(is_field_separator(c) || (c) == ' ' || (c) == 0x09 || \
	 is_separator(c) || is_line_end(c))
#define is_mode(s)		(strncmp(p, s, strlen(s)) == 0)
#define is_type(s)		(strncmp(p, s, strlen(s)) == 0)
#define get_fpga_char(c) \
	do { if (size-- == 0) return 1; else (c) = *(p++); } while (0)
#define next_separator(c)	do { get_fpga_char(c); } while (!is_separator(c))
#define next_line_end(c)	do { get_fpga_char(c); } while (!is_line_end(c))

	unsigned long size;
	char *p;
	unsigned long addr;
	char c = 0;
	int loc;
	int old_loc = -1;
	int is_int;
	char *range;
	int val;
	char cmd[10];

	if (argc != 3)
		return cmd_usage(cmdtp);

	addr = hextoul(argv[1], NULL);
	size = hextoul(argv[2], NULL);
	p = (char *)addr;

	while (size) {
		next_separator(c);
		loc = dectoul(p, NULL);
		if (loc == 0)
			goto nextline;

		next_separator(c);
		if (!(is_mode("rw") || is_mode("w") || is_mode("x")))
			goto nextline;

		next_separator(c);
		is_int = is_type("int");

		next_separator(c);
		range = p;

		next_separator(c);
		if (is_meta_char(*p))
			next_separator(c);

		if (is_int) {
			val = dectoul(p, NULL);
		} else {
			char *q = p;
			int plen = 0;

			while (!is_meta_char(*q)) {
				q++;
				plen++;
			}

			val = 0;
			while (1) {
				if (strncmp(p, range, plen) == 0)
					break;
				while (!is_meta_char(*range))
					range++;
				if (is_field_separator(*range)) {
					range++;
					val++;
				} else {
					val = -1;
					break;
				}
			}
			if (val == -1)
				goto nextline;
		}

		if (loc == old_loc + 1)
			sprintf(cmd, "+%04X", val);
		else
			sprintf(cmd, "w%02X%04X", loc, val);
		fpga_puts(cmd, NULL, 0);
		old_loc = loc;

nextline:
		next_line_end(c);
	}

	printf("\n");

	return 0;
}

U_BOOT_CMD(fpgaset, 3, 1, cmd_fpgaset,
	   "write settings to fpga",
	   "fpgaset <addr> <size>");
