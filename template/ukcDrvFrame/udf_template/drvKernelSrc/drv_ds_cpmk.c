#include "udf_dbh.h"
#include "drv_ds_cpmk.h"

/*
 * TODO: implement your own interfaces
 */
int udf_drv_test_if_cpmk_fn(
	cpmk_drv_data_t * pData,
	cpmk_ds_test_if_t * arg)
{
	pData->a = arg->testk_d;
	arg->testk_d = 1000;

	ukc_log_notice("cpm drv bottom half: test if");

	return 0;
}

int udf_ctor_drv_ds_cpmk_fn(cpmk_drv_data_t * arg, int slot, void * hw_addr)
{
	arg->a = 10;
	arg->b = 11;

	arg->slot = slot;

	ukc_log_notice("cpm drv bottom half: ctor");

	return 0;
}

/*
 * destructure for cpmk drv ds
 */
void udf_dtor_drv_ds_cpmk_fn(cpmk_drv_data_t * arg)
{
	ukc_log_notice("cpm drv bottom half: dtor");
	return;
}



