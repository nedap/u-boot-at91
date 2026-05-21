/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Minimal AT91SAM9G45 USART2 register definitions for the Nedap FPGA
 * maintenance commands.
 */

#ifndef __ATMEL_USART2_H__
#define __ATMEL_USART2_H__

#include <asm/io.h>
#include <asm/arch/hardware.h>

#define USART2_ID			2
#define USART2_BASE			ATMEL_BASE_USART2

#define USART2_CR			0x0000
#define USART2_MR			0x0004
#define USART2_CSR			0x0014
#define USART2_RHR			0x0018
#define USART2_THR			0x001c
#define USART2_BRGR			0x0020

#define USART2_RSTRX_OFFSET		2
#define USART2_RSTTX_OFFSET		3
#define USART2_RXEN_OFFSET		4
#define USART2_TXEN_OFFSET		6

#define USART2_USART_MODE_OFFSET	0
#define USART2_USART_MODE_SIZE		4
#define USART2_USCLKS_OFFSET		4
#define USART2_USCLKS_SIZE		2
#define USART2_CHRL_OFFSET		6
#define USART2_CHRL_SIZE		2
#define USART2_PAR_OFFSET		9
#define USART2_PAR_SIZE		3
#define USART2_NBSTOP_OFFSET		12
#define USART2_NBSTOP_SIZE		2

#define USART2_RXRDY_OFFSET		0
#define USART2_TXRDY_OFFSET		1

#define USART2_CD_OFFSET		0
#define USART2_CD_SIZE			16

#define USART2_USART_MODE_NORMAL	0
#define USART2_USCLKS_MCK		0
#define USART2_CHRL_8			3
#define USART2_PAR_NONE		4
#define USART2_NBSTOP_1		0

#define USART2_BIT(name) \
	(1U << USART2_##name##_OFFSET)
#define USART2_BF(name, value) \
	(((value) & ((1U << USART2_##name##_SIZE) - 1)) << \
	 USART2_##name##_OFFSET)

#define usart2_readl(reg) \
	readl((void *)(USART2_BASE + USART2_##reg))
#define usart2_writel(reg, value) \
	writel((value), (void *)(USART2_BASE + USART2_##reg))

#endif /* __ATMEL_USART2_H__ */
