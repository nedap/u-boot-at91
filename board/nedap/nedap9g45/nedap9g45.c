// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <config.h>
#include <debug_uart.h>
#include <init.h>
#include <net.h>
#include <spartan3.h>
#include <vsprintf.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/at91sam9g45_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <linux/mtd/rawnand.h>
#include <atmel_lcdc.h>
#include <atmel_usart2.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_CMD_NAND
void nedap9g45_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Enable CS3 */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(3) |
	       AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(2),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(7) | AT91_SMC_CYCLE_NRD(4),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
	       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
	       AT91_SMC_MODE_DBW_8 |
#endif
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	at91_periph_clk_enable(ATMEL_ID_PIOC);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CFG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CFG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>
#include <nand.h>

void at91_spl_board_init(void)
{
	/*
	 * On the original AT91SAM9M10G45-EK board, the chip wm9711 stays in the
	 * test mode, so it needs do some action to exit test mode.
	 */
	at91_periph_clk_enable(ATMEL_ID_PIODE);
	at91_set_gpio_output(AT91_PIN_PD7, 0);
	at91_set_gpio_output(AT91_PIN_PD8, 0);
	at91_set_pio_pullup(AT91_PIO_PORTD, 7, 1);
	at91_set_pio_pullup(AT91_PIO_PORTD, 8, 1);

#ifdef CONFIG_SD_BOOT
	at91_mci_hw_init();
#elif CONFIG_NAND_BOOT
	nedap9g45_nand_hw_init();
#endif
}

#include <asm/arch/atmel_mpddrc.h>
static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_16_BITS | ATMEL_MPDDRC_MD_DDR2_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_14 |
		    ATMEL_MPDDRC_CR_DQMS_SHARED |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3);

	ddr2->rtr = 0x24b;

	ddr2->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |/* 6*7.5 = 45 ns */
		      2 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |/* 2*7.5 = 15 ns */
		      2 << ATMEL_MPDDRC_TPR0_TWR_OFFSET | /* 2*7.5 = 15 ns */
		      8 << ATMEL_MPDDRC_TPR0_TRC_OFFSET | /* 8*7.5 = 60 ns */
		      2 << ATMEL_MPDDRC_TPR0_TRP_OFFSET | /* 2*7.5 = 15 ns */
		      1 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET | /* 1*7.5= 7.5 ns*/
		      1 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET | /* 1 clk cycle */
		      2 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET); /* 2 clk cycles */

	ddr2->tpr1 = (2 << ATMEL_MPDDRC_TPR1_TXP_OFFSET | /* 2*7.5 = 15 ns */
		      200 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      16 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      14 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET);

	ddr2->tpr2 = (1 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      0 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET);
}

void mem_init(void)
{
	struct atmel_mpddrc_config ddr2;

	ddr2_conf(&ddr2);

	at91_system_clk_enable(AT91_PMC_DDR);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_DDRSDRC0, ATMEL_BASE_CS6, &ddr2);
}
#endif

#ifdef CONFIG_CMD_USB
static void nedap9g45_usb_hw_init(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIODE);

	at91_set_gpio_output(AT91_PIN_PD1, 0);
	at91_set_gpio_output(AT91_PIN_PD3, 0);
}
#endif

#ifdef CONFIG_CMD_NEDAP_FPGA
static int fpga_pre_config_fn(int cookie)
{
	at91_set_pio_output(CFG_SYS_FPGA_DOUT_PIN, 0);
	at91_set_pio_output(CFG_SYS_FPGA_CLK_PIN, 0);
	at91_set_pio_output(CFG_SYS_FPGA_PROGB_PIN, 1);
	at91_set_pio_input(CFG_SYS_FPGA_INITB_PIN, 0);
	at91_set_pio_input(CFG_SYS_FPGA_DONE_PIN, 0);

	return FPGA_SUCCESS;
}

static int fpga_progb_fn(int assert, int flush, int cookie)
{
	at91_set_pio_output(CFG_SYS_FPGA_PROGB_PIN, 1 - assert);

	return assert;
}

static int fpga_clk_fn(int assert, int flush, int cookie)
{
	at91_set_pio_output(CFG_SYS_FPGA_CLK_PIN, assert);

	return assert;
}

static int fpga_initb_fn(int cookie)
{
	return 1 - at91_get_pio_value(CFG_SYS_FPGA_INITB_PIN);
}

static int fpga_done_fn(int cookie)
{
	return at91_get_pio_value(CFG_SYS_FPGA_DONE_PIN);
}

static int fpga_dout_fn(int assert, int flush, int cookie)
{
	at91_set_pio_output(CFG_SYS_FPGA_DOUT_PIN, assert);

	return assert;
}

static int fpga_fastwr_fn(void *buf, size_t len, int flush, int cookie)
{
	const unsigned char *p = buf;
	size_t i;
	int j;

	for (i = 0; i < len; ++i) {
		unsigned char c = *p++;

		for (j = 0x80; j != 0x00; j >>= 1) {
			at91_set_pio_output(CFG_SYS_FPGA_DOUT_PIN,
					    (c & j) ? 1 : 0);
			at91_set_pio_output(CFG_SYS_FPGA_CLK_PIN, 0);
			at91_set_pio_output(CFG_SYS_FPGA_CLK_PIN, 1);
		}

		if ((i & 0x3ff) == 0)
			schedule();
	}

	return FPGA_SUCCESS;
}

static xilinx_spartan3_slave_serial_fns fpga_fns = {
	.pre = fpga_pre_config_fn,
	.pgm = fpga_progb_fn,
	.clk = fpga_clk_fn,
	.init = fpga_initb_fn,
	.done = fpga_done_fn,
	.wr = fpga_dout_fn,
	.post = NULL,
	.bwr = fpga_fastwr_fn,
	.abort = NULL,
};

static xilinx_desc spartan3 = {
	.family = xilinx_spartan3,
	.iface = slave_serial,
	.size = 588877,
	.iface_fns = &fpga_fns,
	.cookie = 0,
	.operations = &spartan3_op,
};

static void nedap9g45_fpga_hw_init(void)
{
	fpga_init();
	fpga_add(fpga_xilinx, &spartan3);
}

static int nedap9g45_serial2_init(void)
{
	unsigned long divisor;

	at91_set_a_periph(AT91_PIO_PORTB, 6, 1);
	at91_set_a_periph(AT91_PIO_PORTB, 7, 0);
	at91_periph_clk_enable(ATMEL_ID_USART2);

	usart2_writel(CR, USART2_BIT(RSTRX) | USART2_BIT(RSTTX));

	divisor = (get_usart_clk_rate(USART2_ID) / CFG_FPGA_BAUDRATE) / 16;
	usart2_writel(BRGR, USART2_BF(CD, divisor));

	usart2_writel(CR, USART2_BIT(RXEN) | USART2_BIT(TXEN));

	usart2_writel(MR, USART2_BF(USART_MODE, USART2_USART_MODE_NORMAL) |
		      USART2_BF(USCLKS, USART2_USCLKS_MCK) |
		      USART2_BF(CHRL, USART2_CHRL_8) |
		      USART2_BF(PAR, USART2_PAR_NONE) |
		      USART2_BF(NBSTOP, USART2_NBSTOP_1));

	return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	at91_seriald_hw_init();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	return 0;
}
#endif

int board_init(void)
{
	/* arch number of AT91SAM9M10G45EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9M10G45EK;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	nedap9g45_nand_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	nedap9g45_usb_hw_init();
#endif
#ifdef CONFIG_CMD_NEDAP_FPGA
	nedap9g45_serial2_init();
	nedap9g45_fpga_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *) CFG_SYS_SDRAM_BASE,
				    CFG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
}
#endif
