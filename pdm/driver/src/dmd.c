#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/device.h>

MODULE_LICENSE("Dual BSD/GPL");

#include "pdm_drv_enum.h"
#include "pdm_drv_int.h"
#include "pdm_drv_ext.h"
#include "pdm_drv_dev.h"
#include "pdm_irq_hdl.h"

dev_t g_pdmDevID = -1;

static const char g_pdmClassName[] = "TIN_PDM_DRV";
struct class *g_pPdmDrvClass = NULL;

static void dmd_exit(void);

irqreturn_t pdm_drv_isr(int irq, void * pData)
{
	int ret = pdm_handle_slot_int();

	pdm_pr_alert("irq %d occured", irq);

	if (ret > 0)
	{
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static struct drv_vsn_info_t s_pdm_drv_info = {
	.src_vsn = __PDM_DRV_VSN,
	.udf_vsn = NULL,
};

static int dmd_init(void)
{

	int ret = 0;
	pdm_pr_alert("Hello, world\n");

	// x. initialize hardware
	ret = pdm_ctor_hw();
	if (ret < 0)
	{
		pdm_pr_alert("pdm ctor hw error: %d", ret);
		return ret;
	}

	// 1. initialize manage info
	ret = pdm_ctor_mng_info();
	if (ret < 0)
	{
		pdm_pr_alert("pdm ctor mng info error");
		//goto exit;
		return ret;
	}

	// 2. allocate dev id
	ret = alloc_chrdev_region(&g_pdmDevID, 0, EPDM_DRV_MINOR_ID_NUM, g_pdmClassName);
	if (ret < 0)
	{
		pdm_pr_alert("alloc chrdev fail");
		g_pdmDevID = -1;
		goto exit;
	}

	// 3. create device class
	g_pPdmDrvClass = class_create(THIS_MODULE, g_pdmClassName);
	if (IS_OS_PTR_INVALID(g_pPdmDrvClass))
	{
		pdm_pr_alert("class_create error\n");
		ret = -EFAULT;
		g_pPdmDrvClass = NULL;
		goto exit;
	}

	// 4. register PDM online/off-line interface
	ret = pdm_reg_online_offline_cb(ETINDEV_PDM,
			pdm_drv_main_init,
			pdm_drv_main_exit,
			&s_pdm_drv_info);
	if (ret < 0)
	{
		pdm_pr_alert("reg on/off-line fail\n");
		goto exit;
	}

	// 5. pdm card online
	ret = pdm_card_online(EPDM_DRV_MINOR_ID_PDM_SELF, ETINDEV_PDM);
	if (ret < 0)
	{
		pdm_pr_alert("pdm card online fail\n");
		goto exit;
	}

	// 6. sys func init
	ret = sys_func_drv_main_init(EPDM_DRV_MINOR_ID_SYS_FUNC);
	if (ret < 0)
	{
		pdm_pr_alert("sys func init error");
		goto exit;
	}

	// 7. regs info init
	ret = regs_info_drv_main_init(EPDM_DRV_MINOR_ID_REGS_INFO);
	if (ret < 0)
	{
		pdm_pr_alert("regs info init error");
		goto exit;
	}

	// 8. sifp
	ret = sifp_init_all(g_pPdmDrvClass);
	if (ret < 0)
	{
		pdm_pr_alert("sifp ctor error");
		goto exit;
	}

	// 9. request irq
	ret = pdm_irq_init(pdm_drv_isr, "TIN_PDM_IRQ", NULL);
	if (ret < 0)
	{
		pdm_pr_alert("request irq fail\n");
		goto exit;
	}

	pdm_pr_debug("INIT success\n");

exit:
	if (ret < 0)
	{
		dmd_exit();
	}

	return ret;
}


static void dmd_exit(void)
{
	pdm_pr_debug("Goodbye, cruel world\n");

	// free irq
	pdm_irq_exit();

	// sifp
	sifp_exit_all();

	// regs info exit
	regs_info_drv_main_exit(EPDM_DRV_MINOR_ID_REGS_INFO);

	// sys func exit
	sys_func_drv_main_exit(EPDM_DRV_MINOR_ID_SYS_FUNC);

	// offline
	pdm_card_offline(EPDM_DRV_MINOR_ID_PDM_SELF);

	// unreg
	pdm_unreg_online_offline_cb(ETINDEV_PDM);

	// destroy class
	if (g_pPdmDrvClass)
	{
		class_destroy(g_pPdmDrvClass);
		g_pPdmDrvClass = NULL;
	}

	// free device id
	if (g_pdmDevID != -1)
	{
		unregister_chrdev_region(g_pdmDevID, EPDM_DRV_MINOR_ID_NUM);
		g_pdmDevID = -1;
	}

	pdm_dtor_mng_info();

	pdm_dtor_hw();
}

module_init(dmd_init)
module_exit(dmd_exit)



