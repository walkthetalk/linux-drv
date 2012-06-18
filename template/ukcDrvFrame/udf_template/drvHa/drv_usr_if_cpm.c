
/*
 * only for prototype-checking about user-defined interfaces, so you
 * can implement your own interfaces below it.
 */
#include "udf_dth.h"

/*
 * you need it when you implement the interfaces used by *UDF*
 * TODO: change the file name to your own.
 */
#include "drv_ds_cpm.h"

/*
 * TODO: add other header files
 */

/*
 * include the kernel part header file to use the interfaces implemented
 * in kernel part.
 * NOTE: the name of every interface has a subfix named *_usr*.
 */
#include "drv_usr_if_cpmk.h"
/*********************************************************************
 ************************* implement your own if *********************
 ********************************************************************/
// NOTE: please see your own if header file, e.g. drv_if_cpm.def

int udf_drv_test_if_cpm_fn(cpm_drv_data_t * pData, cpm_ds_test_if_t * arg)
{
	int ret = 0;
#if 0
	cpmk_ds_test_if_t karg =
		{
			.testk_d = arg->test_d,
		};

	ukc_log_notice("cpm drv top half: test if udf_drv_test_if_cpm_fn");

	ret = udf_drv_test_if_cpmk_fn_kernel(pData->bh_rep, &karg);
#endif
	{
		u_int32_t offset = 0x100000 + 0x06 * 2;
		volatile u_int16_t * addr = (u_int16_t *)(((unsigned long)(pData->card_base_addr)) + offset);
		const u_int16_t val1 = *addr;
		const u_int16_t val2 = ~val1;
		ukc_log_notice("the original val is %#x", val1);

		if (arg->test_d)
		{
			*addr = val2;
			ukc_log_notice("direct write register");
		}
		else
		{
			(pData->bh_op_set.udf_write16)(pData->bh_rep,
					offset, val2);
		}

		ukc_log_notice("I want write %#x, real is %#x",
				val2,
				*addr);
	}

//	ukc_log_notice("cpm drv top half: read %#x is %#x", 1000, read_result);


	return ret;
}

// TODO: implement your own interfaces above this line, if you added it.


/*********************************************************************
 ************************* implement if used by *UDF* ****************
 ********************************************************************/
// TODO: change function name and the name of first argument.
int udf_ctor_drv_ds_cpm_fn(
	cpm_drv_data_t * pData,	// the data you should initialize
	udf_kdrv_rep_t bh_rep,	// you need store it to op bh part
	const udf_kdrv_op_set_t * bh_op_set,	// you need store it to op bh part
	int slot,
	void * card_base_addr)
{
	pData->bh_rep = bh_rep;
	pData->bh_op_set = *bh_op_set;
	pData->slot = slot;
	pData->card_base_addr = card_base_addr;

	ukc_log_notice("cpm drv top half: addr is %#lx", (unsigned long)pData->card_base_addr);

	ukc_log_notice("cpm drv top half: ctor");

	return 0;
}

// TODO: change the name of function and argument.
void udf_dtor_drv_ds_cpm_fn(cpm_drv_data_t * pData)
{
	ukc_log_notice("cpm drv top half: dtor");
	return;
}

// TODO: change the name of function and argument.
int udf_int_handler_cpm_fn(cpm_drv_data_t * pData)
{
	ukc_log_notice("cpm drv top half: int handler");
	return 0;
}

// TODO: change the name of function and first argument.
ssize_t udf_read_int_data_cpm_fn(cpm_drv_data_t * pData, void * buf, size_t count)
{
	if (count >= sizeof(pData->a))
	{
		*(int *)buf = pData->a;
		pData->a = -1;
	}

	ukc_log_notice("cpmm drv top half: read result of int handler");

	return -1;
}


