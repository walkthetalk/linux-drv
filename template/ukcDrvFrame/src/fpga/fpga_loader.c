/*
 * fpga loader
 * @author:	Ni Qingliang
 * @NOTE:	based on the code written by zhengchangwen
 */


#include "ukc_misc.h"

#include "cpld_cmm_reg_def.h"
#include "fpga_cmm_reg_def.h"

#include "fpga_loader.h"

#if 0
/*
static void dump_fpga(struct fpga_cmm_t * pFpga)
{
	int i = 0;
	u_int16_t * pData = (u_int16_t *)pFpga;
	for (i = 0; i < 4; ++i)
	{
		ukc_pr_notice("    %#06x %#06x %#06x %#06x", pData[0], pData[1], pData[2], pData[3]);
		pData = &(pData[4]);
	}
#define MT_PRINT_FPGA_REG(bf_name) \
	ukc_pr_notice("fpga reg: %s %#x", #bf_name, pFpga->bf_name)

	MT_PRINT_FPGA_REG(SOFT_VER);
	MT_PRINT_FPGA_REG(CHIP_CODE_H);
	MT_PRINT_FPGA_REG(CHIP_CODE_L);
	MT_PRINT_FPGA_REG(RELEASE_VER);
	MT_PRINT_FPGA_REG(BLANK_REG);
	MT_PRINT_FPGA_REG(DEBUG_VER);
	MT_PRINT_FPGA_REG(HOT_RESET);
	MT_PRINT_FPGA_REG(INT_MASK);
	MT_PRINT_FPGA_REG(INT);

#undef MT_PRINT_FPGA_REG

	return;
}

*/
static void dump_cpld(struct cpld_cmm_t * pCpld)
{
#define MT_PRINT_CPLD_BF(mem_name, bf_name) \
	ukc_pr_notice("    cpld %s, %#x", #bf_name, pCpld->mem_name._bf.bf_name);

	ukc_pr_notice("dump cpld");
	MT_PRINT_CPLD_BF(fpga_prog_h, FPGA_PROG_H);
	MT_PRINT_CPLD_BF(fpga_prog_l, FPGA_PROG_L);
	MT_PRINT_CPLD_BF(fpga_done, FPGA_DONE);
	//MT_PRINT_CPLD_BF(fpga_loader, FPGA_CCLK);
	//MT_PRINT_CPLD_BF(fpga_loader, FPGA_DIN);
	MT_PRINT_CPLD_BF(clk_en, CCLK_EN);
#undef MT_PRINT_CPLD_BF

	return;
}
#endif

int UDF_KFUNC(udf_download_fpga_imp)(
	struct cpld_cmm_t * pCpld,
	const u_int8_t * pBuf,
	u_int32_t nBytes)
{
	size_t i = 0;
	const u_int8_t * pBuf_end;

	ukc_log_notice("clear fpga");
	// 1. *clear* fpga
	MT_CHIP_ASS_BF(pCpld, fpga_prog_h, FPGA_PROG_H, 0);
	MT_CHIP_ASS_BF(pCpld, fpga_prog_l, FPGA_PROG_L, 1);

	//dump_cpld(pCpld);

	// 2. delay
	ukc_ussleep(20000);

	ukc_log_notice("cancel clear fpga");
	// 3. cancel *clear*
	MT_CHIP_ASS_BF(pCpld, fpga_prog_h, FPGA_PROG_H, 1);
	MT_CHIP_ASS_BF(pCpld, fpga_prog_l, FPGA_PROG_L, 0);

	//dump_cpld(pCpld);

	// 4. delay
	ukc_ussleep(10000);

	ukc_log_notice("if done?");
	// 5. if done? do we need it? maybe need ask zhengchangwen
	if (MT_CHIP_GET_BIT(pCpld, fpga_done, FPGA_DONE))
	{
		return -1;
	}

	//ukc_ussleep(10000);

	ukc_log_notice("enable clock");
	// 6. enable clock
	MT_CHIP_ASS_BF(pCpld, clk_en, CCLK_EN, 1);

	// 7. send data
	--pBuf;
	pBuf_end = pBuf + nBytes;
	while (pBuf < pBuf_end)
	{
		typeof(pCpld->fpga_loader) data_2_write = { MT_INIT_BF(FPGA_LOADER, *(++pBuf)), };
		pCpld->fpga_loader.v = data_2_write.v;
	}

	// 8. disable clock
	MT_CHIP_CLR_BIT(pCpld, clk_en, CCLK_EN);

	ukc_log_notice("wait done");
	// 9. wait done
	for (i = 0; i < 500; ++i)
	{
		if (MT_CHIP_GET_BIT(pCpld, fpga_done, FPGA_DONE))
		{
			ukc_log_notice("fpga done");
			break;
		}
		ukc_ussleep(10000);
	}

	if (i >= 500)
	{
		ukc_log_err("fpga done? no!!!!");
		return -1;
	}

	return 0;
}
