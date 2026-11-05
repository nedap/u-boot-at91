// SPDX-License-Identifier: GPL-2.0+
/*
 * Real-time Timer of the AT91SAM9260 family. The counter cannot be written, only reset, so the
 * time is the counter plus an offset in a backup register, the one Linux's rtc-at91sam9 uses.
 * An offset of 0 means nobody set the time yet; that is reported like any other time.
 */

#include <dm.h>
#include <rtc.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>

#define RTT_MR			0x00
#define RTT_MR_RTTRST		BIT(18)
#define RTT_AR			0x04
#define RTT_VR			0x08

#define RTT_PRESCALER_1HZ	32768
#define RTT_ALARM_OFF		0xffffffff
#define RTT_RESTART_TIMEOUT_US	1000

struct at91sam9_rtt_priv {
	void __iomem *rtt;
	void __iomem *offset;
};

static int at91sam9_rtt_restart(struct at91sam9_rtt_priv *priv, u32 offset)
{
	u32 count;
	int ret;

	writel(RTT_PRESCALER_1HZ | RTT_MR_RTTRST, priv->rtt + RTT_MR);
	writel(RTT_ALARM_OFF, priv->rtt + RTT_AR);

	ret = readl_poll_timeout(priv->rtt + RTT_VR, count, !count,
				 RTT_RESTART_TIMEOUT_US);
	if (ret)
		return ret;

	writel(offset, priv->offset);

	return 0;
}

static int at91sam9_rtt_get(struct udevice *dev, struct rtc_time *time)
{
	struct at91sam9_rtt_priv *priv = dev_get_priv(dev);
	u32 count;

	/* The counter runs on the slow clock, so one read can catch it mid-update. */
	do {
		count = readl(priv->rtt + RTT_VR);
	} while (count != readl(priv->rtt + RTT_VR));

	rtc_to_tm((u64)count + readl(priv->offset), time);

	return 0;
}

static int at91sam9_rtt_set(struct udevice *dev, const struct rtc_time *time)
{
	time64_t secs = rtc_mktime(time);

	/* The register holds 32 bits, and Linux reads an offset of 0 as a time never set. */
	if (secs <= 0 || secs > U32_MAX)
		return -EINVAL;

	return at91sam9_rtt_restart(dev_get_priv(dev), secs);
}

static int at91sam9_rtt_reset(struct udevice *dev)
{
	return at91sam9_rtt_restart(dev_get_priv(dev), 0);
}

static int at91sam9_rtt_probe(struct udevice *dev)
{
	struct at91sam9_rtt_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args reg;
	fdt_addr_t base;
	int ret;

	priv->rtt = dev_read_addr_ptr(dev);
	if (!priv->rtt)
		return -EINVAL;

	ret = dev_read_phandle_with_args(dev, "atmel,rtt-rtc-time-reg", NULL, 1,
					 0, &reg);
	if (ret)
		return ret;

	base = ofnode_get_addr(reg.node);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;
	priv->offset = (void __iomem *)base + reg.args[0];

	return 0;
}

static const struct rtc_ops at91sam9_rtt_ops = {
	.get	= at91sam9_rtt_get,
	.set	= at91sam9_rtt_set,
	.reset	= at91sam9_rtt_reset,
};

static const struct udevice_id at91sam9_rtt_ids[] = {
	{ .compatible = "atmel,at91sam9260-rtt" },
	{ }
};

U_BOOT_DRIVER(rtc_at91sam9_rtt) = {
	.name		= "rtc_at91sam9_rtt",
	.id		= UCLASS_RTC,
	.of_match	= at91sam9_rtt_ids,
	.probe		= at91sam9_rtt_probe,
	.ops		= &at91sam9_rtt_ops,
	.priv_auto	= sizeof(struct at91sam9_rtt_priv),
};
