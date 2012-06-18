/*
 * the definition of data structure used by your own interface
 * NOTE:
 *     this data structure will be used in both user space and kernel space.
 */
/*
 * NOTE:
 * TODO:
 * please check this file's name, and *UDF_KDRV_IF_DS_HDR_FILE_NAME* in
 * your cfg file, they must be same.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// TODO: add the definition of data structure used by your own interface
/*
 * NOTE:
 * don't use *pointer* in the structure, if so, you can't access the data
 * pointed to by the pointer, but use *copy_to_user* and/or "copy_from_user*
 * (because the interfaces using these ds may run
 * in user space, and also kernel space.)
 */
typedef struct
{
	int testk_d;
} cpmk_ds_test_if_t;


#ifdef __cplusplus
}
#endif

