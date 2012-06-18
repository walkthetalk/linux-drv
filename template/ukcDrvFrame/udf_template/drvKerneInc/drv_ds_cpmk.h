#pragma once

/*
 * NOTE:
 * TODO:
 * please check this file's name, and *UDF_KDRV_DS_HDR_FILE_NAME* in
 * your cfg file, they must be same.
 */
/*
 * this file is used by *UDF* for prototype-checking
 * and the ds and interfaces will be used/issued by *UDF* code.
 */

#ifdef __cplusplus
extern "C" {
#endif

// TODO: include your own header files

/*********************************************************************
 ******************the data structure*********************************
 ********************************************************************/
/*
 * the driver's data structure, it's name must be *udf_drv_ds_***_t*,
 * *** is *UDF_TOKEN_SUBFIX*
 * if you are not satisfied with the name, please typedef it.
 */
typedef struct udf_drv_ds_cpmk_t
{
	// TODO: your own implementation
	int a;
	char b;
	char c[100];

	// NOTE: don't delete member *slot*, of course, if you are sure
	// you will not use it, you can do that.
	int slot;
} cpmk_drv_data_t;



/*********************************************************************
 ******************the data structure*********************************
 ********************************************************************/
/*
 * constructure for cpmk drv ds
 */
/*
 * TODO: change function name and the type of the first argument.
 * NOTE1:
 *     the function name must be *udf_ctor_drv_ds_xxxk_fn*, in which the
 * *xxx* represents your card type, see macro *UDF_TOKEN_SUBFIX* in
 * your cfg file. The type of the first argument must be same as above.
 * NOTE2:
 *     this is just a ctor, not new, don't forget it.
 */
int udf_ctor_drv_ds_cpmk_fn(cpmk_drv_data_t *, int slot, void * hw_addr);

/*
 * destructure for cpmk drv ds
 */
/*
 * TODO:
 *     same as above.
 * NOTE1:
 *     same as above.
 * NOTE2:
 *     this is just a dtor, not delete, don't forget it.
 */
void udf_dtor_drv_ds_cpmk_fn(cpmk_drv_data_t *);


#ifdef __cplusplus
}
#endif

