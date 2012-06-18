
#include <sys/times.h>
#include <stdio.h>
#include <stdlib.h>
#include "drv_usr_if_cpm.h"

#include "omtgroup.h"

int main(int argc, char * argv[])
{
	udf_drv_op_set_t op_set;
	udf_drv_rep_t drv_rep;
	cpm_ds_test_if_t ds_test;

	if (argc >= 2)
	{
		ds_test.test_d = 1;
	}
	else
	{
		ds_test.test_d = 0;
	}
	
	printf("test_d is %d\n", ds_test.test_d);

	struct tms lt_before;
	struct tms lt_after;

	InitGroup(2);

	int ret = get_drv_op_set_cpm_fn(&op_set);
	if (ret < 0)
	{
		printf("get op set err\n");
		return -1;
	}

	int slot = 6;
	if (argc >= 3)
	{
		long int lslot = strtol(argv[1], NULL, 0);
		if (lslot < 3 || 10 < lslot)
		{
			printf("the slot argument is error: %s", argv[1]);
			printf("the range is [3, 10]");
			return -3;
		}
		slot = lslot;
		printf("slot is %s", argv[1]);
	}
	drv_rep = (op_set.udf_open)(slot);
	if ((op_set.udf_rep_is_err)(drv_rep))
	{
		printf("open err\n");
		return -2;
	}
#if 1
	times(&lt_before);// = time(NULL);

	//ret = udf_drv_test_if_cpm_fn_usr(drv_rep, &ds_test);

	ret = (op_set.udf_download_fpga)(drv_rep, argv[2]);

	times(&lt_after);// = time(NULL);

	printf("cpm drv top half: udiff: %ld, sdiff: %ld\n",
		lt_after.tms_stime - lt_before.tms_stime,
		lt_after.tms_utime - lt_before.tms_utime);
#else
	printf("file name is %s\n", argv[1]);
	ret = (op_set.udf_download_fpga)(drv_rep, argv[1]);
#endif
	ret = (op_set.udf_release)(drv_rep);

	return 0;
}
