/*
 * the implementation of user if
 */
#ifdef DRV_USR_MODE
	// no need
#else
#ifndef DRV_KERNEL_SIDE
#error
#else
// only valid for kernel mode + kernel side.


#include "drv_self_cfg.h"
#include UDF_KDRV_IF_USR_HDR_FILE_NAME

#include "udf_dbh.h"
#include "udf_rep_c_real.h"
//#include "udf_dbh_ds.h"

// ensure udf_drv_rep_t is bigger than or equal to int
typedef char UDF_KTYPE(udf_rep_constraint)[sizeof(udf_kdrv_rep_t) >= sizeof(UDF_KTYPE(udf_bh_rep_t)) ? 1 : -1];
/*
 * get op set implementation
 */

#define MT_ENCAP_READ_WRITE_REG_FUNC(bit_num) \
static MT_GEN_VAR_TYPE(bit_num) udf_read##bit_num##_encap(udf_kdrv_rep_t rep, u_int64_t addr) \
{ \
	UDF_KTYPE(ioc_arg_rw##bit_num##_t) arg = { \
		.addr = addr, \
		.val = (MT_GEN_VAR_TYPE(bit_num))(-1), \
	}; \
 \
	UDF_KFUNC(udf_read##bit_num)(udf_kdrv_rep_2_real(rep), &arg); \
 \
	return arg.val; \
} \
\
static void udf_write##bit_num##_encap(udf_kdrv_rep_t rep, \
		u_int64_t addr, MT_GEN_REG_TYPE(bit_num) val) \
{ \
	UDF_KTYPE(ioc_arg_rw##bit_num##_t) arg = { \
		.addr = addr, \
		.val = val, \
	}; \
 \
	UDF_KFUNC(udf_write##bit_num)(udf_kdrv_rep_2_real(rep), &arg); \
 \
	return; \
}

MT_ENCAP_READ_WRITE_REG_FUNC(64)
MT_ENCAP_READ_WRITE_REG_FUNC(32)
MT_ENCAP_READ_WRITE_REG_FUNC(16)
MT_ENCAP_READ_WRITE_REG_FUNC(8)

#undef MT_ENCAP_READ_WRITE_REG_FUNC

static void udf_enable_slot_int_encap(udf_kdrv_rep_t rep)
{
	UDF_KFUNC(udf_enable_slot_int)(udf_kdrv_rep_2_real(rep), NULL);

	return;
}

static void udf_disable_slot_int_encap(udf_kdrv_rep_t rep)
{
	UDF_KFUNC(udf_disable_slot_int)(udf_kdrv_rep_2_real(rep), NULL);

	return;
}

static int udf_query_slot_online_encap(udf_kdrv_rep_t rep)
{
	int ret = 0;
	ret = UDF_KFUNC(udf_query_slot_online)(udf_kdrv_rep_2_real(rep), NULL);

	return ret;
}

int UDF_KFUNC(get_drv_op_set)(udf_kdrv_op_set_t * rSet)
{
	if (rSet == NULL)
	{
		return -1;
	}

	ukc_log_debug("get op set of card %s in kbh", UDF_CARD_IDF);

	rSet->udf_read64 = udf_read64_encap;
	rSet->udf_write64 = udf_write64_encap;

	rSet->udf_read32 = udf_read32_encap;
	rSet->udf_write32 = udf_write32_encap;

	rSet->udf_read16 = udf_read16_encap;
	rSet->udf_write16 = udf_write16_encap;

	rSet->udf_read8 = udf_read8_encap;
	rSet->udf_write8 = udf_write8_encap;

	rSet->udf_enable_slot_int = udf_enable_slot_int_encap;
	rSet->udf_disable_slot_int = udf_disable_slot_int_encap;

	rSet->udf_query_slot_online = udf_query_slot_online_encap;

	return 0;
}


/*
 * user-defined interface's implementation
 */
#define UDF_IOCTL_MACRO_ENCAP(no_use, func, type) \
int func##_kernel(udf_kdrv_rep_t rep, type * arg) \
{ \
	return func(udf_kdrv_rep_2_real(rep), arg); \
}

#include UDF_KDRV_IF_DEF_FILE_NAME

#undef UDF_IOCTL_MACRO_ENCAP






#endif
#endif

