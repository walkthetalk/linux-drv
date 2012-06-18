/*
 * when simulating, drop the interrupt
 */
#if 1	// disble interrupt

#include "cp_hdr.h"
#include "pdm_irq_hdl.h"

int pdm_irq_init(
		irq_handler_t handler,
		const char *name, void *dev)
{
	return 0;
}
void pdm_irq_exit(void)
{
	return;
}
#else	// p2020 version
#include <linux/of_platform.h>
#include "cp_hdr.h"
#include "pdm_irq_hdl.h"


static int s_irqnum = -1;

static int __devinit p2020_int_probe(
	struct of_device * ofdev,
	const struct of_device_id * match_tbl)
{
	struct resource irq;

	/*
	 * IMPORTANT!!!!
	 * the return value is the irq's vector
	 */
	s_irqnum = of_irq_to_resource(ofdev->node, 0, &irq);
	if (s_irqnum == NO_IRQ) {
		pdm_pr_alert("irq to resource's ret is %d", s_irqnum);
		return -EIO;
	}

	pdm_pr_info("pdm irq num is %d", s_irqnum);

	return 0;

}

static int __devexit p2020_int_remove(struct of_device *ofdev)
{
	return 0;
}

/*
 * this table must be same as it in dts, or the probe will fail.
 */
static const struct of_device_id p2020_int_match_tbl[] = {
	{.compatible = "fsl,cpld0",},
	{},
};

/* Structure for a device driver */
static struct of_platform_driver mpc_cpld_driver = {
	.match_table	= p2020_int_match_tbl,
	.probe		= p2020_int_probe,
	.remove		= __devexit_p(p2020_int_remove),
	.driver	=
		{
			.owner = THIS_MODULE,
			.name = "tin_cpm_cpld",
		},
};

int pdm_irq_init(
		irq_handler_t handler,
		const char *name, void *dev)
{
	int ret;
	ret = of_register_platform_driver(&mpc_cpld_driver);
	if (ret < 0)
	{
		pdm_pr_alert("of_register_platform_driver failed (%d)", ret);
		return ret;
	}

	return request_irq(s_irqnum, handler, IRQF_TRIGGER_LOW, name, dev);
}
void pdm_irq_exit(void)
{
	free_irq(s_irqnum, NULL);
	of_unregister_platform_driver(&mpc_cpld_driver);
}

#endif

