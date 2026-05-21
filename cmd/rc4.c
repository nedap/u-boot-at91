// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Jaap de Jong <jaap.dejong@nedap.com>
 */

#include <command.h>
#include <cyclic.h>
#include <mapmem.h>
#include <vsprintf.h>
#include <linux/types.h>

struct rc4_state {
	u8 perm[256];
	u8 index1;
	u8 index2;
};

static void rc4_swap(u8 *a, u8 *b)
{
	u8 temp = *a;

	*a = *b;
	*b = temp;
}

static void rc4_init(struct rc4_state *state, const u8 *key, int keylen)
{
	u8 j = 0;
	int i;

	for (i = 0; i < 256; i++)
		state->perm[i] = (u8)i;

	state->index1 = 0;
	state->index2 = 0;

	for (i = 0; i < 256; i++) {
		j += state->perm[i] + key[i % keylen];
		rc4_swap(&state->perm[i], &state->perm[j]);
	}
}

static void rc4_crypt(struct rc4_state *state, u8 *buf, ulong buflen)
{
	u8 j;

	while (buflen--) {
		state->index1++;
		state->index2 += state->perm[state->index1];

		rc4_swap(&state->perm[state->index1], &state->perm[state->index2]);

		j = state->perm[state->index1] + state->perm[state->index2];
		*buf++ ^= state->perm[j];

		if ((buflen & 0x7fffff) == 0)
			schedule();
	}
}

static int do_rc4(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	static const u8 key[] = {
		0x1c, 0xcd, 0x74, 0x2a, 0x2c, 0xd9, 0xc5, 0xe7,
		0x55, 0x3d, 0x8b, 0x4b, 0x81, 0xc9, 0xe3, 0x4e,
	};
	struct rc4_state state;
	ulong addr;
	ulong size;
	u8 *buf;

	if (argc != 3)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);
	size = hextoul(argv[2], NULL);
	buf = map_sysmem(addr, size);

	rc4_init(&state, key, sizeof(key));
	rc4_crypt(&state, buf, size);

	unmap_sysmem(buf);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	rc4, 3, 1, do_rc4,
	"encrypt/decrypt memory",
	"<addr> <size>"
);
