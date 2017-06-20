/*
 * Register definitions for the Atmel USART2 module.
 *
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __DRIVERS_ATMEL_USART2_H__
#define __DRIVERS_ATMEL_USART2_H__

#define USART2_ID				2
#define USART2_BASE				0xfff94000

/* USART2 register offsets */
#define USART2_CR				0x0000
#define USART2_MR				0x0004
#define USART2_IER				0x0008
#define USART2_IDR				0x000c
#define USART2_IMR				0x0010
#define USART2_CSR				0x0014
#define USART2_RHR				0x0018
#define USART2_THR				0x001c
#define USART2_BRGR				0x0020
#define USART2_RTOR				0x0024
#define USART2_TTGR				0x0028
#define USART2_FIDI				0x0040
#define USART2_NER				0x0044
#define USART2_XXR				0x0048
#define USART2_IFR				0x004c
#define USART2_RPR				0x0100
#define USART2_RCR				0x0104
#define USART2_TPR				0x0108
#define USART2_TCR				0x010c
#define USART2_RNPR				0x0110
#define USART2_RNCR				0x0114
#define USART2_TNPR				0x0118
#define USART2_TNCR				0x011c
#define USART2_PTCR				0x0120
#define USART2_PTSR				0x0124

/* Bitfields in CR */
#define USART2_RSTRX_OFFSET			2
#define USART2_RSTRX_SIZE			1
#define USART2_RSTTX_OFFSET			3
#define USART2_RSTTX_SIZE			1
#define USART2_RXEN_OFFSET			4
#define USART2_RXEN_SIZE			1
#define USART2_RXDIS_OFFSET			5
#define USART2_RXDIS_SIZE			1
#define USART2_TXEN_OFFSET			6
#define USART2_TXEN_SIZE			1
#define USART2_TXDIS_OFFSET			7
#define USART2_TXDIS_SIZE			1
#define USART2_RSTSTA_OFFSET			8
#define USART2_RSTSTA_SIZE			1
#define USART2_STTBRK_OFFSET			9
#define USART2_STTBRK_SIZE			1
#define USART2_STPBRK_OFFSET			10
#define USART2_STPBRK_SIZE			1
#define USART2_STTTO_OFFSET			11
#define USART2_STTTO_SIZE			1
#define USART2_SENDA_OFFSET			12
#define USART2_SENDA_SIZE			1
#define USART2_RSTIT_OFFSET			13
#define USART2_RSTIT_SIZE			1
#define USART2_RSTNACK_OFFSET			14
#define USART2_RSTNACK_SIZE			1
#define USART2_RETTO_OFFSET			15
#define USART2_RETTO_SIZE			1
#define USART2_DTREN_OFFSET			16
#define USART2_DTREN_SIZE			1
#define USART2_DTRDIS_OFFSET			17
#define USART2_DTRDIS_SIZE			1
#define USART2_RTSEN_OFFSET			18
#define USART2_RTSEN_SIZE			1
#define USART2_RTSDIS_OFFSET			19
#define USART2_RTSDIS_SIZE			1
#define USART2_COMM_TX_OFFSET			30
#define USART2_COMM_TX_SIZE			1
#define USART2_COMM_RX_OFFSET			31
#define USART2_COMM_RX_SIZE			1

/* Bitfields in MR */
#define USART2_USART_MODE_OFFSET		0
#define USART2_USART_MODE_SIZE			4
#define USART2_USCLKS_OFFSET			4
#define USART2_USCLKS_SIZE			2
#define USART2_CHRL_OFFSET			6
#define USART2_CHRL_SIZE			2
#define USART2_SYNC_OFFSET			8
#define USART2_SYNC_SIZE			1
#define USART2_PAR_OFFSET			9
#define USART2_PAR_SIZE				3
#define USART2_NBSTOP_OFFSET			12
#define USART2_NBSTOP_SIZE			2
#define USART2_CHMODE_OFFSET			14
#define USART2_CHMODE_SIZE			2
#define USART2_MSBF_OFFSET			16
#define USART2_MSBF_SIZE			1
#define USART2_MODE9_OFFSET			17
#define USART2_MODE9_SIZE			1
#define USART2_CLKO_OFFSET			18
#define USART2_CLKO_SIZE			1
#define USART2_OVER_OFFSET			19
#define USART2_OVER_SIZE			1
#define USART2_INACK_OFFSET			20
#define USART2_INACK_SIZE			1
#define USART2_DSNACK_OFFSET			21
#define USART2_DSNACK_SIZE			1
#define USART2_MAX_ITERATION_OFFSET		24
#define USART2_MAX_ITERATION_SIZE		3
#define USART2_FILTER_OFFSET			28
#define USART2_FILTER_SIZE			1

/* Bitfields in CSR */
#define USART2_RXRDY_OFFSET			0
#define USART2_RXRDY_SIZE			1
#define USART2_TXRDY_OFFSET			1
#define USART2_TXRDY_SIZE			1
#define USART2_RXBRK_OFFSET			2
#define USART2_RXBRK_SIZE			1
#define USART2_ENDRX_OFFSET			3
#define USART2_ENDRX_SIZE			1
#define USART2_ENDTX_OFFSET			4
#define USART2_ENDTX_SIZE			1
#define USART2_OVRE_OFFSET			5
#define USART2_OVRE_SIZE			1
#define USART2_FRAME_OFFSET			6
#define USART2_FRAME_SIZE			1
#define USART2_PARE_OFFSET			7
#define USART2_PARE_SIZE			1
#define USART2_TIMEOUT_OFFSET			8
#define USART2_TIMEOUT_SIZE			1
#define USART2_TXEMPTY_OFFSET			9
#define USART2_TXEMPTY_SIZE			1
#define USART2_ITERATION_OFFSET			10
#define USART2_ITERATION_SIZE			1
#define USART2_TXBUFE_OFFSET			11
#define USART2_TXBUFE_SIZE			1
#define USART2_RXBUFF_OFFSET			12
#define USART2_RXBUFF_SIZE			1
#define USART2_NACK_OFFSET			13
#define USART2_NACK_SIZE			1
#define USART2_RIIC_OFFSET			16
#define USART2_RIIC_SIZE			1
#define USART2_DSRIC_OFFSET			17
#define USART2_DSRIC_SIZE			1
#define USART2_DCDIC_OFFSET			18
#define USART2_DCDIC_SIZE			1
#define USART2_CTSIC_OFFSET			19
#define USART2_CTSIC_SIZE			1
#define USART2_RI_OFFSET			20
#define USART2_RI_SIZE				1
#define USART2_DSR_OFFSET			21
#define USART2_DSR_SIZE				1
#define USART2_DCD_OFFSET			22
#define USART2_DCD_SIZE				1
#define USART2_CTS_OFFSET			23
#define USART2_CTS_SIZE				1

/* Bitfields in RHR */
#define USART2_RXCHR_OFFSET			0
#define USART2_RXCHR_SIZE			9

/* Bitfields in THR */
#define USART2_TXCHR_OFFSET			0
#define USART2_TXCHR_SIZE			9

/* Bitfields in BRGR */
#define USART2_CD_OFFSET			0
#define USART2_CD_SIZE				16

/* Bitfields in RTOR */
#define USART2_TO_OFFSET			0
#define USART2_TO_SIZE				16

/* Bitfields in TTGR */
#define USART2_TG_OFFSET			0
#define USART2_TG_SIZE				8

/* Bitfields in FIDI */
#define USART2_FI_DI_RATIO_OFFSET		0
#define USART2_FI_DI_RATIO_SIZE			11

/* Bitfields in NER */
#define USART2_NB_ERRORS_OFFSET			0
#define USART2_NB_ERRORS_SIZE			8

/* Bitfields in XXR */
#define USART2_XOFF_OFFSET			0
#define USART2_XOFF_SIZE			8
#define USART2_XON_OFFSET			8
#define USART2_XON_SIZE				8

/* Bitfields in IFR */
#define USART2_IRDA_FILTER_OFFSET		0
#define USART2_IRDA_FILTER_SIZE			8

/* Bitfields in RCR */
#define USART2_RXCTR_OFFSET			0
#define USART2_RXCTR_SIZE			16

/* Bitfields in TCR */
#define USART2_TXCTR_OFFSET			0
#define USART2_TXCTR_SIZE			16

/* Bitfields in RNCR */
#define USART2_RXNCR_OFFSET			0
#define USART2_RXNCR_SIZE			16

/* Bitfields in TNCR */
#define USART2_TXNCR_OFFSET			0
#define USART2_TXNCR_SIZE			16

/* Bitfields in PTCR */
#define USART2_RXTEN_OFFSET			0
#define USART2_RXTEN_SIZE			1
#define USART2_RXTDIS_OFFSET			1
#define USART2_RXTDIS_SIZE			1
#define USART2_TXTEN_OFFSET			8
#define USART2_TXTEN_SIZE			1
#define USART2_TXTDIS_OFFSET			9
#define USART2_TXTDIS_SIZE			1

/* Constants for USART_MODE */
#define USART2_USART_MODE_NORMAL		0
#define USART2_USART_MODE_RS485			1
#define USART2_USART_MODE_HARDWARE		2
#define USART2_USART_MODE_MODEM			3
#define USART2_USART_MODE_ISO7816_T0		4
#define USART2_USART_MODE_ISO7816_T1		6
#define USART2_USART_MODE_IRDA			8

/* Constants for USCLKS */
#define USART2_USCLKS_MCK			0
#define USART2_USCLKS_MCK_DIV			1
#define USART2_USCLKS_SCK			3

/* Constants for CHRL */
#define USART2_CHRL_5				0
#define USART2_CHRL_6				1
#define USART2_CHRL_7				2
#define USART2_CHRL_8				3

/* Constants for PAR */
#define USART2_PAR_EVEN				0
#define USART2_PAR_ODD				1
#define USART2_PAR_SPACE			2
#define USART2_PAR_MARK				3
#define USART2_PAR_NONE				4
#define USART2_PAR_MULTI			6

/* Constants for NBSTOP */
#define USART2_NBSTOP_1				0
#define USART2_NBSTOP_1_5			1
#define USART2_NBSTOP_2				2

/* Constants for CHMODE */
#define USART2_CHMODE_NORMAL			0
#define USART2_CHMODE_ECHO			1
#define USART2_CHMODE_LOCAL_LOOP		2
#define USART2_CHMODE_REMOTE_LOOP		3

/* Constants for MSBF */
#define USART2_MSBF_LSBF			0
#define USART2_MSBF_MSBF			1

/* Constants for OVER */
#define USART2_OVER_X16				0
#define USART2_OVER_X8				1

/* Constants for CD */
#define USART2_CD_DISABLE			0
#define USART2_CD_BYPASS			1

/* Constants for TO */
#define USART2_TO_DISABLE			0

/* Constants for TG */
#define USART2_TG_DISABLE			0

/* Constants for FI_DI_RATIO */
#define USART2_FI_DI_RATIO_DISABLE		0

/* Bit manipulation macros */
#define USART2_BIT(name)				\
	(1 << USART2_##name##_OFFSET)
#define USART2_BF(name,value)				\
	(((value) & ((1 << USART2_##name##_SIZE) - 1))	\
	 << USART2_##name##_OFFSET)
#define USART2_BFEXT(name,value)			\
	(((value) >> USART2_##name##_OFFSET)		\
	 & ((1 << USART2_##name##_SIZE) - 1))
#define USART2_BFINS(name,value,old)			\
	(((old) & ~(((1 << USART2_##name##_SIZE) - 1)	\
		    << USART2_##name##_OFFSET))		\
	 | USART2_BF(name,value))

/* Register access macros */
#define usart2_readl(reg)				\
	readl((void *)USART2_BASE + USART2_##reg)
#define usart2_writel(reg,value)			\
	writel((value), (void *)USART2_BASE + USART2_##reg)

#endif /* __DRIVERS_ATMEL_USART2_H__ */
