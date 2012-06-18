#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "ukc_misc.h"

/*
 * I know, I should use *discriptor*, but not *represent*, however,
 * pls. ignore that.
 */
/*
 * kernel part
 * using a different type against *void ** is only for type-checking.
 */
typedef union
{
	int fd;
	void * dsp;
} udf_kdrv_rep_t;

/*
 * the interface *read32* and *write32* should be not card-specific,
 * so we can do further optimization, execute them through a global
 * device file, if so, there is no need for the first argument.
 */
typedef struct udf_kdrv_op_set
{
	u_int64_t (*udf_read64)(udf_kdrv_rep_t, u_int64_t addr);
	void (*udf_write64)(udf_kdrv_rep_t, u_int64_t addr, volatile u_int64_t val);

	u_int32_t (*udf_read32)(udf_kdrv_rep_t, u_int64_t addr);
	void (*udf_write32)(udf_kdrv_rep_t, u_int64_t addr, volatile u_int32_t val);

	u_int16_t (*udf_read16)(udf_kdrv_rep_t, u_int64_t addr);
	void (*udf_write16)(udf_kdrv_rep_t, u_int64_t addr, volatile u_int16_t val);

	u_int8_t (*udf_read8)(udf_kdrv_rep_t, u_int64_t addr);
	void (*udf_write8)(udf_kdrv_rep_t, u_int64_t addr, volatile u_int8_t val);

	int (*udf_query_slot_online)(udf_kdrv_rep_t);

	void (*udf_enable_slot_int)(udf_kdrv_rep_t);
	void (*udf_disable_slot_int)(udf_kdrv_rep_t);
} udf_kdrv_op_set_t;



#ifdef __cplusplus
}
#endif


