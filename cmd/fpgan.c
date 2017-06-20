/*
 * (C) Copyright 2012
 * Jaap de Jong, jaap.dejong@nedap.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/arch/gpio.h>
#include "atmel_usart2.h"

int fpga_puts (char * s, char * r, int length)
{
	const long MAX_RATIO = 7;
	const long DELAY = 2500;
	long i = DELAY;
	int len = 0;
	char c;

	usart2_readl(RHR);
	while (1) {
		if (i < MAX_RATIO * DELAY && *s && (usart2_readl(CSR) & USART2_BIT(TXRDY))) usart2_writel(THR, *s++);
		if (usart2_readl(CSR) & USART2_BIT(RXRDY)) {
			++len;
			c = usart2_readl(RHR);
			if (r == NULL) {
				printf("%c", c);
			} else if (length > 0) {
				if (length <= 2) length = 0;
				else --length;
				*(r++) = c;
				*(r) = 0;
			}
			i += DELAY;
		} else {
			if (i-- == 0) return len;
		}
	}
}

int cmd_fpgatst (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	const int ANSWER_LENGTH = 7;			// every answer from the fpga should be this long
	const int DATA_INDEX = 3;				// data that is send is at location 3..6
	const char FPGA_VERSION[] = "0e";		// 'r' is added seperatly
	const char SYNC_MASTER[] = "w320001";	// turn the unit to be the sync master
	const char TEST_ADC[] = "r31";			// read the adc test register
	const char NO_ADC[] = "0000";			// no adc found = '0000'; adc found = '0001'..'0FFF'
	const char OK_LEVEL[] = "6000";			// spectrum level should be above '6000'
	const char READ[] = "r";				// the read flag for FPGA_VERSION and SPECTRUM
	const int FIRST_SPECTRUM = 196;			// first spectrum register c4 (c0..c3 are skipped because fixed at 6000)
	const int LAST_SPECTRUM = 219;			// last spectrum register db (dc..df are skipped because fixed at 6000)
	char snd[ANSWER_LENGTH];				// command to send fixed length
	char rcv[ANSWER_LENGTH+3];				// data to receive has some extra space for alarm-push characters ('[' and ']')
	int i;
	int n;
	int j;
	int attempt;

	if (argc != 1) return cmd_usage(cmdtp);

	printf("RF test: ");

	// wait until a proper echo is received
	// first send 'r' until it is echoed; at that point a next 'r' (or 2) are already in the pipeline and the
	// first attempt to read the FPGA_VERSION will fail, but is harmless
	attempt = 1;
	while (1) {
		do {
			if (attempt++ > 500) goto error;
			n = fpga_puts(READ, rcv, sizeof(rcv));
		} while (n != 1 || rcv[0] != READ[0]);
		n = fpga_puts(FPGA_VERSION, rcv, sizeof(rcv));
		if (n == ANSWER_LENGTH - 1 && strncmp(rcv, FPGA_VERSION, strlen(FPGA_VERSION)) == 0) break;
	}

	// check if adc is placed
	// pcb's without adc will answer with '0000'; pcb's with adc will answer '0001'..'0FFF'
	// (although never seen anything else then '0FFF')
	// if no adc is found, further testing is not needed
	attempt = 1;
	while (1) {
		if (attempt++ > 50) goto error;
		n = fpga_puts(TEST_ADC, rcv, sizeof(rcv));
		for (j = 0; rcv[j] != READ[0] && j < n; ++j) ;
		if (n - j == ANSWER_LENGTH) break;
	}
//	if (strncmp(&rcv[j + DATA_INDEX], NO_ADC, strlen(NO_ADC)) == 0) {
	i = 0;
	for (j += DATA_INDEX; j < n; ++j) i += (rcv[j] != '0');
	if (i < 3) {
		printf("no ADC found\n\n");
		return 0;
	}

	// must set sync_master to determine fpga problem
	fpga_puts(SYNC_MASTER, rcv, sizeof(rcv));
	if (strcmp(rcv, SYNC_MASTER) != 0) goto error;

	// wait until first spectum value != '6000'
	// it will take some time to settle
	attempt = 1;
	sprintf(snd, "%s%02x", READ, FIRST_SPECTRUM);
	while (1) {
		if (attempt++ > 50) goto error;
		n = fpga_puts(snd, rcv, sizeof(rcv));
		for (j = 0; rcv[j] != READ[0] && j < n; ++j) ;
		if (strncmp(&rcv[j + DATA_INDEX], OK_LEVEL, strlen(OK_LEVEL)) != 0) break;
	}

	// check all spectrum values '6000' or above
	// if all spectrum value are below '6000' then we have a false startup of the fpga
	// the fpga needs to be reconfigured and retested afterwards
	attempt = 1;
	while (1) {
		if (attempt++ > 5) goto error;
		for (i = FIRST_SPECTRUM; i <= LAST_SPECTRUM; ++i) {
			sprintf(snd, "%s%02x", READ, i);
			n = fpga_puts(snd, rcv, sizeof(rcv));
			for (j = 0; rcv[j] != READ[0] && j < n; ++j) ;
			if (strncmp(&rcv[j + DATA_INDEX], OK_LEVEL, strlen(OK_LEVEL)) >= 0) {
				printf("OK\n\n");
				return 0;
			}
		}
	}

error:
	printf("ERROR\n\n");
	return 1;
}

U_BOOT_CMD (fpgatst, 1, 1, cmd_fpgatst,
	"test fpga",
	"fpgatst\n"
	"	- test if rf engine is running\n"
	"	- example:\n"
	"	-	fpgatst");

int cmd_fpgaw (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2) return cmd_usage(cmdtp);

	fpga_puts(argv[1], NULL, 0);
	printf("\n");

	return 0;
}

U_BOOT_CMD (fpgaw, 2, 1, cmd_fpgaw,
	"write to fpga port",
	"fpgaw <value>\n"
	"	- write to the fpga\n"
	"	- example:\n"
	"	-	fpgaw f38+++++++");

int cmd_fpgaser (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long size = 0;
	char *p;
	char cmd[10];

	p = getenv("serial#");
	if (p) {
		while (*p) {
			if (size++) sprintf(cmd, "+%c%c", *p, *(p+1)); else sprintf(cmd, "s20%c%c", *p, *(p+1));
			fpga_puts(cmd, NULL, 0);
			p += 2;
		}
	}

	printf("\n");

	return 0;
}

U_BOOT_CMD (fpgaser, 1, 1, cmd_fpgaser,
	"write serial# to fpga",
	"fpgaser\n"
	"	- write serial# to the fpga\n"
	"	- example:\n"
	"	-	fpgaser");

int cmd_fpgaset (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	#define isFieldSeparator(c)	(c == ',')
	#define isSeparator(c)		(c == ';')
	#define isLineEnd(c)		(c == '\n')
	#define isMetaChar(c)		(isFieldSeparator(c) || c == ' ' || c == 0x09 || isSeparator(c) || isLineEnd(c))
	#define isMode(s)			(strncmp(p, s, strlen(s)) == 0)
	#define isType(s)			(strncmp(p, s, strlen(s)) == 0)
	#define getFpga(c)			{ if (size-- == 0) return 1; else c = *(p++); }
	#define nextSeparator(c)	do getFpga(c) while (!isSeparator(c))
	#define nextLineEnd(c)		do getFpga(c) while (!isLineEnd(c))

	unsigned long size = 0;
	char *p;

	unsigned long addr;
	char c = 0;

	int loc, oLoc = loc + 1;
	int isInt;
	char *range;
	int val;

	char cmd[10];

	if (argc != 3) return cmd_usage(cmdtp);

	addr = simple_strtoul(argv[1], NULL, 16);
	size = simple_strtoul(argv[2], NULL, 16);

	p = (char *) addr;

	while (size) {

		// Loc
		nextSeparator(c);
		loc = simple_strtoul(p, NULL, 10);
		if (loc == 0) goto nextline;

		// Fl
		nextSeparator(c);
		if (!(isMode("rw") || isMode("w") || isMode("x"))) goto nextline;

		// Type
		nextSeparator(c);
		isInt = isType("int");

		// Range
		nextSeparator(c);
		range = p;

		// CurrentVal or DefaultVal
		nextSeparator(c);
		if (isMetaChar(*p))	nextSeparator(c);

		if (isInt) {
			val = simple_strtoul(p, NULL, 10);
		} else {
			char * q = p;
			int plen = 0;

			// determine length
			while (!isMetaChar(*q)) {
				++q;
				++plen;
			}

			// determine value
			val = 0;
			while (1) {
				if (strncmp(p, range, plen) == 0) break;
				while (!isMetaChar(*range)) ++range;
				if (isFieldSeparator(*range)) {
					++range;
					++val;
				} else {
				    val = -1;
				    break;
				}
			}
			if (val == -1) goto nextline;
		}

        // output loc & value
		if (loc == oLoc + 1) sprintf(cmd, "+%04X", val); else sprintf(cmd, "w%02X%04X", loc, val);
		fpga_puts(cmd, NULL, 0);
		oLoc = loc;

nextline:
		// EOL
		nextLineEnd(c);

	}

	printf("\n");

	return 0;
}

U_BOOT_CMD (fpgaset, 3, 1, cmd_fpgaset,
	"write settings to fpga",
	"fpgaset <addr> <size>\n"
	"	- write to the fpga\n"
	"	- example:\n"
	"	- 	ubifsload 70000000 /home/root/xc3sprog/fpgaparams\n"
	"	-	fpgaset 70000000 ${filesize}");
