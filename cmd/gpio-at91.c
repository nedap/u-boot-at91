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
#include <asm/arch/at91_pio.h>

int cmd_gpio (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	unsigned short mode;
	unsigned short port;
	unsigned short pin;
	unsigned short val;

	if (argc != 4) return cmd_usage(cmdtp);

	switch (argv[1][0]) {
		case 'r': mode = 1; break;
		case 'w': mode = 0; break;
		default:  return cmd_usage(cmdtp);
	}

	const char *str_pin = argv[2];
	if (*str_pin == 'p') ++str_pin;
	switch (*str_pin) {
		case 'a': port = 0; break;
		case 'b': port = 1; break;
		case 'c': port = 2; break;
		case 'd': port = 3; break;
		case 'e': port = 4; break;
		default:  return cmd_usage(cmdtp);
	}

	pin = (unsigned short) simple_strtoul(str_pin + 1, NULL, 10);
	if (pin > 31) return cmd_usage(cmdtp);

	val = (unsigned short) simple_strtoul (argv[3], NULL, 10);
	if (val > 1) return cmd_usage(cmdtp);

	if (mode == 0) {
		at91_set_pio_output(port, pin, val);	
	} else {
		at91_set_pio_input(port, pin, val);
		val = at91_get_pio_value(port, pin);
		printf ("%d\n", val);
	}

	return 0;
}

U_BOOT_CMD (gpio, 4, 1, cmd_gpio,
	"read/write gpio input/output pin",
	"gpio <read|write> <port><pin> <value>\n"
	"	- read/write the specified pin (e.g. pd10)\n"
	"	- example:\n"
	"	-	gpio read pd30 0");

