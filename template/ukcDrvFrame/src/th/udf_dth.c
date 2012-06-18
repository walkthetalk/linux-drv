#include "udf_cfg.h"
//#include UDF_DRV_DS_HDR_FILE_NAME	// no relation with *udf_cfg.h*

#include "udf_dth.h"

#include "udf_dbh.h"
#include "ukc_mem.h"
#include "ukc_misc.h"
#include "ukc_errno.h"
#include "udf_rep_c_real.h"
#include "udf_dth_ds.h"

#include UDF_KDRV_IF_USR_HDR_FILE_NAME

// UM/US or KM/KS
#ifdef DRV_USR_MODE
	#ifndef DRV_KERNEL_SIDE
	#else
		#error "top half must not *DRV_KERNEL_SIDE* in *DRV_USR_MODE*"
	#endif
#else
	#ifndef DRV_KERNEL_SIDE
		#error "top half must be KERNEL_SIDE in KERNEL_MODE"
	#else

	#endif
#endif

#if 0 //move to *udf_dth_ds.h*
typedef struct
{
	struct UDF_TYPE(udf_drv_ds) ds_base; // implemented by user

	int slot_id;
	int slot_base_addr;

	UDF_TYPE(udf_bh_rep_t) bh_rep;
} UDF_TYPE(SUDF_DRV_DATA);

#define GET_BH_FROM_TH(pTh) (pTh->bh_rep)
#define GET_ORI_DRV_POINTER(pData) container_of(pData,UDF_TYPE(SUDF_DRV_DATA),ds_base)
#endif


struct UDF_TYPE(udf_drv_ds) * UDF_FUNC(udf_th_open)(int slot)
{
	int ret = 0;
	udf_kdrv_op_set_t bh_op_set =
	{
		.udf_read64 = NULL,
		.udf_write64 = NULL,
		.udf_read32 = NULL,
		.udf_write32 = NULL,
		.udf_read16 = NULL,
		.udf_write16 = NULL,
		.udf_read8 = NULL,
		.udf_write8 = NULL,
		.udf_enable_slot_int = NULL,
		.udf_disable_slot_int = NULL,
	};

	// malloc data
	UDF_TYPE(SUDF_DRV_DATA) * pData = UKC_MALLOC(UDF_TYPE(SUDF_DRV_DATA));
	if (pData == NULL)
	{
		ukc_log_err("malloc th data structure fail");
		return NULL;
	}

	// open bh
	pData->bh_rep = UDF_KFUNC(udf_bh_open)(slot);
	if (IS_UDF_BH_REP_ERR(pData->bh_rep))
	{
		ukc_log_err("open bh err %ld", (long int)pData->bh_rep);
		goto err_exit;
	}

	pData->slot_id = slot;

	//ukc_log_notice("th want the hw addr of %#x", pData->bh_rep);//get mem_size
	pData->hw_addr = UDF_KFUNC(udf_bh_capture_hw_addr)(pData->bh_rep,&pData->mem_size);
	if (pData->hw_addr == NULL)
	{
		ukc_log_err("get hw addr error");
		goto err_exit2;
	}

	// get bh op set
	ret = UDF_KFUNC(get_drv_op_set)(&bh_op_set);
	if (ret < 0)
	{
		ukc_log_err("get bh op set err %d", ret);
		goto err_exit2;
	}

	// construct ds_base
	ret = UDF_FUNC(udf_ctor_drv_ds)(&pData->ds_base,
			udf_kdrv_real_2_rep(pData->bh_rep),
			&bh_op_set,
			slot,
			pData->hw_addr);
	if (ret < 0)
	{
		ukc_log_err("call th ctor err: %d", ret);
		goto err_exit2;
	}

	return &pData->ds_base;
err_exit2:
	UDF_KFUNC(udf_bh_release)(pData->bh_rep);
err_exit:
	UKC_FREE(pData);
	return NULL;
}

int UDF_FUNC(udf_th_release)(struct UDF_TYPE(udf_drv_ds) * pData)
{
	UDF_TYPE(SUDF_DRV_DATA) * pOri = GET_ORI_DRV_POINTER(pData);

	// dtor user data
	UDF_FUNC(udf_dtor_drv_ds)(pData);

	// release hw addr
	UDF_KFUNC(udf_bh_release_hw_addr)(pOri->hw_addr,pOri->mem_size);

	// release bh
	UDF_KFUNC(udf_bh_release)(GET_BH_FROM_DBASE(pData));

	// free self
	UKC_FREE(pOri);

	return 0;
}


#ifdef DRV_USR_MODE
#else
#ifndef DRV_KERNEL_SIDE
#else
int UDF_FUNC(udf_th_isr)(struct UDF_TYPE(udf_drv_ds) * pData)
{
	return UDF_FUNC(udf_int_handler)(pData);
}
#endif
#endif

ssize_t UDF_FUNC(udf_th_read)(struct UDF_TYPE(udf_drv_ds) * pData, void * buf, size_t count)
{
	ssize_t ret = 0;
	ret = UDF_KFUNC(udf_bh_read)(
			GET_BH_FROM_DBASE(pData), buf, count);
	if (ret < 0)
	{
		return ret;
	}
#ifdef DRV_USR_MODE
#ifndef DRV_KERNEL_SIDE
	ret = UDF_FUNC(udf_int_handler)(pData);
	if (ret <= 0)
	{
		return ret;
	}
#endif
#endif
	ret = UDF_FUNC(udf_read_int_data)(pData, buf, count);
#ifdef DRV_USR_MODE
#ifndef DRV_KERNEL_SIDE
	if (ret >= 0)
	{
		// if *read* handle interrupt, then need re-enable it.
		UDF_KFUNC(udf_enable_slot_int)(GET_BH_FROM_DBASE(pData),
			&(GET_ORI_DRV_POINTER(pData)->slot_id));
	}
#endif
#endif

	return ret;
}

int UDF_FUNC(udf_reset)(struct UDF_TYPE(udf_drv_ds) * pData, int * arg)
{
#if 0
	u_int32_t data;

	ukc_log_err("in func udf_reset\n");
	data = UDF_KFUNC(read32)(
			GET_BH_FROM_DBASE(pData),
			(u_int32_t *)arg);
	ukc_log_err("udf_reset: read %ld is %d", (long int)arg, (int)data);
#endif
	return 0;
}

int UDF_FUNC(udf_hello)(struct UDF_TYPE(udf_drv_ds) * pData, int * arg)
{
#if 0
	ukc_log_err("in func udf_hello\n");
	UDF_FUNC(write32)(
			GET_BH_FROM_DBASE(pData),
			(u_int32_t *)arg, 100);
	ukc_log_err("udf_hello: read %ld is %d", (long int)arg, (int)100);
#endif
	return 0;
}

int UDF_FUNC(udf_enable_int)(struct UDF_TYPE(udf_drv_ds) * pData, int * arg)
{
	ukc_log_notice("udf th enable int arg is %d", *arg);
	if (*arg)
	{
		return UDF_KFUNC(udf_enable_slot_int)(
				GET_BH_FROM_DBASE(pData),
				&GET_ORI_DRV_POINTER(pData)->slot_id);
	}
	else
	{
		UDF_KFUNC(udf_disable_slot_int)(
				GET_BH_FROM_DBASE(pData),
				&GET_ORI_DRV_POINTER(pData)->slot_id);
		return 0;
	}
}

int UDF_FUNC(udf_query_online)(struct UDF_TYPE(udf_drv_ds) * pData, int * arg)
{
	return UDF_KFUNC(udf_query_slot_online)(
			GET_BH_FROM_DBASE(pData),
			&GET_ORI_DRV_POINTER(pData)->slot_id);
}

int UDF_FUNC(udf_get_mem_size)(struct UDF_TYPE(udf_drv_ds) * pData, unsigned int * arg)
{
	return UDF_KFUNC(udf_get_mem_size)(
			GET_BH_FROM_DBASE(pData),
			arg);
}
#ifdef DRV_USR_MODE
int UDF_FUNC(udf_download_fpga)(struct UDF_TYPE(udf_drv_ds) * pData,
		const ioc_arg_buf_t * arg)
{
	UDF_TYPE(SUDF_DRV_DATA) * pOri = GET_ORI_DRV_POINTER(pData);
	return UDF_KFUNC(udf_download_fpga)(
			pOri->hw_addr, arg);
}
#endif
