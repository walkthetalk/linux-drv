#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

/*
 * I know, I should use *discriptor*, but not *represent*, however,
 * pls. ignore that.
 */
typedef union
{
	int fd;
	void * dsp;
} udf_drv_rep_t;


typedef ssize_t (*fn_udf_read)(udf_drv_rep_t rep, void * buf, size_t count);

/*
 * the definition of driver operation set
 */
typedef struct udf_drv_op_set
{
	int (*udf_rep_2_fd)(udf_drv_rep_t rep);
	int (*udf_rep_is_err)(udf_drv_rep_t rep);
	udf_drv_rep_t (*udf_open)(int slot);
	int (*udf_release)(udf_drv_rep_t rep);
//	ssize_t (*udf_read)(udf_drv_rep_t rep, void * buf, size_t count);
	fn_udf_read udf_read;

	int (*udf_enable_int)(udf_drv_rep_t rep, int * arg);
	int (*udf_query_online)(udf_drv_rep_t rep);

	int (*udf_download_fpga)(udf_drv_rep_t rep, const char * file_name);
} udf_drv_op_set_t;


#ifdef __cplusplus
}
#endif


