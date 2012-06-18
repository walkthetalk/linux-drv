#include "udf_dbh.h"
#include "ukc_misc.h"
#include "fpga_loader.h"

/*
 * for *open*
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

/*
 * for *close*
 */
#include <unistd.h>

/*
 * for *ioctl*
 */
#include <sys/ioctl.h>

/*
 * for perror
 */
#include <stdio.h>

// must UM/US
#ifdef DRV_USR_MODE
	#ifndef DRV_KERNEL_SIDE	
	// must UM/US
	#else
		#error "must be user side"
	#endif
#else
	#error "must be user mode"
#endif

int UDF_KFUNC(udf_bh_open)(int slot)
{
	int fd = open(UDF_FUNC(getDeviceFilePath)(slot), (O_RDWR | O_CLOEXEC));
	if (fd < 0)
	{
		// TODO: assert(0);
		// TODO: LOG
		ukc_log_err("bh open %s err %d\n", UDF_FUNC(getDeviceFilePath)(slot), fd);
	}

	return fd;
}

int UDF_KFUNC(udf_bh_release)(int fd)
{
	return close(fd);
}

ssize_t UDF_KFUNC(udf_bh_read)(int fd, void * buf, size_t count)
{
	return read(fd, buf, count);
}

void * UDF_KFUNC(udf_bh_capture_hw_addr)(int fd,unsigned int * p_mem_size)
{
	void * ret = MAP_FAILED;

	UDF_KFUNC(udf_get_mem_size)(fd, p_mem_size);

	ret = mmap(NULL, *p_mem_size, (PROT_WRITE | PROT_READ), MAP_SHARED, fd, 0);
	if (ret == MAP_FAILED)
	{
		ret = NULL;
		perror("map failed");
	}

	return ret;
}

int UDF_KFUNC(udf_bh_release_hw_addr)(void * addr,unsigned int mem_size)
{
	int ret = munmap(addr, mem_size);
	if (ret < 0)
	{
		perror("unmap failed");
	}

	return ret;
}

#if 0
int UDF_KFUNC(udf_bh_ioctl)(int fd, unsigned int cmd, unsigned long arg)
{
	return ioctl(fd, cmd, arg);
}
#endif

int UDF_KFUNC(udf_download_fpga)(void *pcpld,const ioc_arg_buf_t *arg)
{
 return	UDF_KFUNC(udf_download_fpga_imp)(
		pcpld,
		arg->buf,
		arg->nBytes);
}

/*
 * auto generate the implements of interfaces defined in header file.
 * NOTE: used in library
 */
#define UDF_IOCTL_MACRO UDF_KIOCTL_MACRO_USR_LIB
#include "udf_dbh_ioctl_if.def"


