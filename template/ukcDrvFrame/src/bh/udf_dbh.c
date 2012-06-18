#include <asm/uaccess.h>

#include "pdm_drv_ext.h"

#include "udf_dbh.h"
#include "udf_dbh_ds.h"

#include "ukc_mem.h"
#include "ukc_misc.h"
#include "udf_misc.h"

#include "udf_internal_slot_info.h"
#include "fpga_loader.h"


struct UDF_KTYPE(udf_drv_ds) * UDF_KFUNC(udf_bh_open)(int slot)
{
	int ret = 0;
	UDF_KTYPE(SUDF_KDRV_DATA) * pData = UKC_MALLOC(UDF_KTYPE(SUDF_KDRV_DATA));
	if (pData == NULL)
	{
		ukc_log_err("malloc bh failed ????\n");
		return NULL;
	}

	// initialize
	pData->slot = slot;
	pData->hw_addr = UDF_FUNC(udf_get_slot_hw_base_addr)(slot);

	// user dtor
	ret = UDF_KFUNC(udf_ctor_drv_ds)(&pData->ds_base, slot, pData->hw_addr);
	if (ret < 0)
	{
		ukc_log_err("call bh ctor fail: %d", ret);
		goto err_exit;
	}

	return &pData->ds_base;
err_exit:
	UKC_FREE(pData);
	return NULL;
}

int UDF_KFUNC(udf_bh_release)(struct UDF_KTYPE(udf_drv_ds) * pData)
{
	UDF_KFUNC(udf_dtor_drv_ds)(pData);

	UKC_FREE(GET_ORI_KDRV_POINTER(pData));

	return 0;
}

ssize_t UDF_KFUNC(udf_bh_read)(struct UDF_KTYPE(udf_drv_ds) * pData, void * buf, size_t count)
{
	// NOTE: there is no need to do anything currently.
	return 0;
}

void * UDF_KFUNC(udf_bh_capture_hw_addr)(struct UDF_KTYPE(udf_drv_ds) * pData,unsigned int * p_mem_size)
{
	return GET_ORI_KDRV_POINTER(pData)->hw_addr;
}

int UDF_KFUNC(udf_bh_release_hw_addr)(void * addr,unsigned int mem_size)
{
	// don't need
	return 0;
}

#ifdef DRV_USR_MODE
int UDF_KFUNC(udf_bh_isr)(struct UDF_KTYPE(udf_drv_ds) * pData)
{
	// NOTE: there is no need to do anything in it, but the return value
	//       must be *<=0*.
	return 0;
}
#endif


/*
 * the ioctl interfaces provided by udf
 */
#define MT_READ_WRITE_REG_FUNC(bit_num) \
int UDF_KFUNC(udf_read##bit_num)(struct UDF_KTYPE(udf_drv_ds) * pInst, UDF_KTYPE(ioc_arg_rw##bit_num##_t) * arg) \
{ \
	arg->val = *(MT_GEN_REGP_TYPE(bit_num))( \
		(unsigned long)(GET_ORI_KDRV_POINTER(pInst)->hw_addr) \
		+ (unsigned long)(arg->addr)); \
 \
	return 0; \
} \
 \
int UDF_KFUNC(udf_write##bit_num)(struct UDF_KTYPE(udf_drv_ds) * pInst, UDF_KTYPE(ioc_arg_rw##bit_num##_t) * arg) \
{ \
	*(MT_GEN_REGP_TYPE(bit_num))( \
		(unsigned long)(GET_ORI_KDRV_POINTER(pInst)->hw_addr) \
		+ (unsigned long)(arg->addr)) = arg->val; \
 \
	return 0; \
}

MT_READ_WRITE_REG_FUNC(64)
MT_READ_WRITE_REG_FUNC(32)
MT_READ_WRITE_REG_FUNC(16)
MT_READ_WRITE_REG_FUNC(8)

#undef MT_READ_WRITE_REG_FUNC

int UDF_KFUNC(udf_enable_slot_int)(
	struct UDF_KTYPE(udf_drv_ds) * pData, const int * no_use)
{
	return pdm_enable_int(GET_ORI_KDRV_POINTER(pData)->slot);
}

int UDF_KFUNC(udf_disable_slot_int)(
	struct UDF_KTYPE(udf_drv_ds) * pData, const int * no_use)
{
	pdm_disable_int(GET_ORI_KDRV_POINTER(pData)->slot);
	return 0;
}

int UDF_KFUNC(udf_query_slot_online)(
	struct UDF_KTYPE(udf_drv_ds) * pData, const int * no_use)
{
	return pdm_query_online(GET_ORI_KDRV_POINTER(pData)->slot);
}

int UDF_KFUNC(udf_get_mem_size)(
	struct UDF_KTYPE(udf_drv_ds) * pData, unsigned int * p_mem_size)
{
	*p_mem_size = pdm_get_addr_space_size(GET_ORI_KDRV_POINTER(pData)->slot);
	return 0;
}
/*
int UDF_KFUNC(udf_download_fpga)(
	struct UDF_KTYPE(udf_drv_ds) * pData,
	const ioc_arg_buf_t * arg)
{
	return UDF_KFUNC(udf_download_fpga_imp)(
		GET_ORI_KDRV_POINTER(pData)->hw_addr,
		arg->buf,
		arg->nBytes);
}*/


