/*
 * the implementation of user if
 */
#include "drv_self_cfg.h"
#include UDF_DRV_IF_USR_HDR_FILE_NAME

#include "udf_dth.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>


// ensure udf_drv_rep_t is bigger than or equal to int
typedef char UDF_TYPE(udf_rep_constraint)[sizeof(udf_drv_rep_t) >= sizeof(int) ? 1 : -1];
/*
 * get op set implementation
 */
static int udf_rep_2_fd(udf_drv_rep_t rep)
{
	return rep.fd;
}

static inline udf_drv_rep_t udf_fd_2_rep(int fd)
{
	return ((udf_drv_rep_t){ .fd = fd, });
}

static int udf_rep_is_err_encap(udf_drv_rep_t rep)
{
	if (rep.fd <= 0)
	{
		return 1;
	}

	return 0;
}

static udf_drv_rep_t udf_open_encap(int slot)
{
	int ret = UDF_FUNC(udf_th_open)(UDF_FUNC(conv_slot_phy_2_vir)(slot));

	return udf_fd_2_rep(ret);
}

static int udf_release_encap(udf_drv_rep_t rep)
{
	return UDF_FUNC(udf_th_release)(udf_rep_2_fd(rep));
}

static ssize_t udf_read_encap(udf_drv_rep_t rep, void * buf, size_t count)
{
	return UDF_FUNC(udf_th_read)(udf_rep_2_fd(rep), buf, count);
}

static int udf_enable_int_encap(udf_drv_rep_t rep, int * arg)
{
	ukc_log_notice("th lib enable int arg is %d", *arg);
	return UDF_FUNC(udf_enable_int)(udf_rep_2_fd(rep), arg);
}

static int udf_query_online_encap(udf_drv_rep_t rep)
{
	return UDF_FUNC(udf_query_online)(udf_rep_2_fd(rep), NULL);
}

static int udf_download_fpga_encap(udf_drv_rep_t rep, const char * file_name)
{
	int fpga_fd = -1;
	struct stat fpga_stat;
	int ret = -1;
	void * buf = NULL;
	ssize_t read_ret = -1;

	ioc_arg_buf_t arg;

	fpga_fd = open(file_name, O_RDONLY);
	if (fpga_fd < 0)
	{
		perror("open fpga file");
		return fpga_fd;
	}

	ret = fstat(fpga_fd, &fpga_stat);
	if (ret < 0)
	{
		close(fpga_fd);
		perror("stat fpga file");
		return ret;
	}

	ukc_log_debug("file size is %ld", fpga_stat.st_size);

	buf = malloc(fpga_stat.st_size);
	if (buf == NULL)
	{
		close(fpga_fd);
		perror("malloc for fpga file");
		return -1;
	}

	read_ret = read(fpga_fd, buf, fpga_stat.st_size);
	if (read_ret < 0)
	{
		free(buf);
		close(fpga_fd);
		perror("read fpga file");
		return read_ret;
	}

	close(fpga_fd);

	arg.buf = buf;
	arg.nBytes = fpga_stat.st_size;
	ret = UDF_FUNC(udf_download_fpga)(&arg);

	free(buf);

	return ret;
}

int UDF_FUNC(get_drv_op_set)(udf_drv_op_set_t * rSet)
{
	if (rSet == NULL)
	{
		return -1;
	}

	ukc_log_notice("get op set of card %s in lib", UDF_CARD_IDF);

	rSet->udf_rep_is_err = udf_rep_is_err_encap;
	rSet->udf_rep_2_fd = udf_rep_2_fd;
	rSet->udf_open = udf_open_encap;
	rSet->udf_release = udf_release_encap;
	rSet->udf_read = udf_read_encap;

	rSet->udf_enable_int = udf_enable_int_encap;
	rSet->udf_query_online = udf_query_online_encap;

	rSet->udf_download_fpga = udf_download_fpga_encap;

	return 0;
}


/*
 * user-defined interface's implementation
 */
#define UDF_IOCTL_MACRO_ENCAP(no_use, func, type) \
int func##_usr(udf_drv_rep_t rep, type * arg) \
{ \
	return func(udf_rep_2_fd(rep), arg); \
}

#include UDF_DRV_IF_DEF_FILE_NAME

#undef UDF_IOCTL_MACRO_ENCAP


