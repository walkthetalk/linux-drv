#include "udf_dth.h"

#include "ukc_misc.h"
#include "fpga_loader.h"

/*
 * for *open*
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * for *close*
 */
#include <unistd.h>

/*
 * for *ioctl*
 */
#include <sys/ioctl.h>

/*
 * for *mmap*
 */
#include <sys/mman.h>

// only KM/US need
#ifdef DRV_USR_MODE
	#error "maybe error directory!!!"
#else
	#ifndef DRV_KERNEL_SIDE
	#else
		#error "maybe error directory!!!"
	#endif
#endif

typedef struct {
	void *pcpld;
	unsigned int mem_size;
}s_cpld_info;

s_cpld_info g_store_cpld_info;

int UDF_FUNC(udf_th_open)(int slot)
{
	int fd = open(UDF_FUNC(getDeviceFilePath)(slot), (O_RDWR | O_CLOEXEC));
	if (fd < 0)
	{
		ukc_log_err("open %s error %d", UDF_FUNC(getDeviceFilePath)(slot), fd);
	}

	g_store_cpld_info.pcpld = UDF_FUNC(udf_th_capture_hw_addr)(fd,&g_store_cpld_info.mem_size);

	return fd;
}

int UDF_FUNC(udf_th_release)(int fd)
{
	UDF_FUNC(udf_th_release_hw_addr)(g_store_cpld_info.pcpld,g_store_cpld_info.mem_size);
	return close(fd);
}

ssize_t UDF_FUNC(udf_th_read)(int fd, void * buf, size_t count)
{
	return read(fd, buf, count);
}

#if 0
int UDF_FUNC(udf_th_ioctl)(int fd, unsigned int cmd, unsigned long arg)
{
	return ioctl(fd, cmd, arg);
}
#endif


void * UDF_FUNC(udf_th_capture_hw_addr)(int fd,unsigned int * p_mem_size)
{
	void * ret = MAP_FAILED;

	UDF_FUNC(udf_get_mem_size)(fd, p_mem_size);

	ret = mmap(NULL, *p_mem_size, (PROT_WRITE | PROT_READ), MAP_SHARED, fd, 0);
	if (ret == MAP_FAILED)
	{
		ret = NULL;
		perror("map failed");
	}

	return ret;
}

int UDF_FUNC(udf_th_release_hw_addr)(void * addr,unsigned int mem_size)
{
	int ret = munmap(addr, mem_size);
	if (ret < 0)
	{
		perror("unmap failed");
	}

	return ret;
}

int UDF_FUNC(udf_download_fpga)(const ioc_arg_buf_t *arg)
{
    return UDF_KFUNC(udf_download_fpga_imp)(
		g_store_cpld_info.pcpld,
		arg->buf,
		arg->nBytes);
}

/*
 * auto generate the implements of interfaces defined in header file.
 * NOTE: used in library
 */
#define UDF_IOCTL_MACRO UDF_IOCTL_MACRO_USR_LIB
#include "udf_dth_ioctl_if.def"


