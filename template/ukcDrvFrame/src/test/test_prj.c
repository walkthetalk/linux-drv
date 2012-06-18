#include "udf_dth.h"

#include <stdio.h>



int main(int argc, char * const argv[])
{
	udf_th_rep_t hw = udf_th_open(0);
	if (IS_OPEN_ERR(hw))
	{
		printf("open err %ld\n", (long int)hw);
		return -1;
	}

	int test_read;
	udf_reset(hw, &test_read);

	printf("test_read result is %d\n", test_read);

	udf_th_release(hw);

	return 0;
}
