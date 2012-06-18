#include "udf_dbh_ext.h" // provide driver for online/offline
#include "udf_internal_slot_info.h"	// get hw address

#include "sifp_drv_ext.h"	// reg/unreg sifp call-back

#include "ukc_misc.h"

#ifdef UDF_KDRV_SIFP_HDR_FILE_NAME
#include UDF_KDRV_SIFP_HDR_FILE_NAME	// call user-defined interface to init sifp
#endif

int UDF_KFUNC(udf_bh_online)(int slot)
{
	int ret = 0;

#ifdef UDF_KDRV_SIFP_HDR_FILE_NAME
	{
		void * hw_addr = UDF_FUNC(udf_get_slot_hw_base_addr)(slot);
		sifp_info_t sifp_info;
		UDF_KFUNC(udf_sifp_init)(hw_addr, &sifp_info);
		ret = sifp_reg_for_online(slot, &sifp_info);
		if (ret < 0)
		{
			ukc_log_err("udf_bh_online reg sifp fail: %d", ret);
		}
	}
#endif

	return ret;
}

void UDF_KFUNC(udf_bh_offline)(int slot)
{
#ifdef UDF_KDRV_SIFP_HDR_FILE_NAME
	sifp_unreg_for_offline(slot);
#endif
}
