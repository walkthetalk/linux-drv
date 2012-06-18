#include "kdrv_op_set.h"
#include "udf_dbh.h"

/*
 * It looks like: the *inline* is no use when compiling kernel module,
 * it will result multiple definition, so I added the keyword *static*.
 */
#ifdef DRV_USR_MODE
#ifndef DRV_KERNEL_SIDE
	static inline int udf_kdrv_rep_2_real(udf_kdrv_rep_t rep)
	{
		return rep.fd;
	}
	static inline udf_kdrv_rep_t udf_kdrv_real_2_rep(int fd)
	{
		return ((udf_kdrv_rep_t){ .fd = fd, });
	}
#else
	static inline UDF_KTYPE(udf_bh_rep_t) udf_kdrv_rep_2_real(udf_kdrv_rep_t rep)
	{
		return (UDF_KTYPE(udf_bh_rep_t))(rep.dsp);
	}
	static inline udf_kdrv_rep_t udf_kdrv_real_2_rep(UDF_KTYPE(udf_bh_rep_t) real)
	{
		return ((udf_kdrv_rep_t){ .dsp = real, });
	}
#endif
#else	// DRV_KERNEL_MODE
#ifndef DRV_KERNEL_SIDE
#error "you must define DRV_KERNEL_SIDE for bh when DRV_KERNEL_MODE"
#endif
	static inline UDF_KTYPE(udf_bh_rep_t) udf_kdrv_rep_2_real(udf_kdrv_rep_t rep)
	{
		return (UDF_KTYPE(udf_bh_rep_t))(rep.dsp);
	}
	static inline udf_kdrv_rep_t udf_kdrv_real_2_rep(UDF_KTYPE(udf_bh_rep_t) real)
	{
		return ((udf_kdrv_rep_t){ .dsp = real, });
	}
#endif



