#include "pdm_drv_sim.h"
#include "pdm_drv_regrw.h"

#define MT_PRINT_REG_MEM_BF(mem_name, bf_name) \
	pdm_pr_notice("%s: %#x", #bf_name, MT_CHIP_GET_BF(g_pdm_chip, mem_name, bf_name))

int pdm_sim_slot_on_off_line(ETinSlot_t slot, ETinDev_t type)
{
	switch (slot) {
	case ETIN_SLOT_SELF:
		// nothing to do
		break;
	default:
		pdm_pr_notice("when sim, the slot error: %d", slot);
		break;
	}

	return 0;
}

const char * pdm_slot_2_str(ETinSlot_t slot)
{
	static const char * slotStr[ETIN_SLOT_MAX] = {
		[ETIN_SLOT_PDM] = "PDM",
		[ETIN_SLOT_SELF] = "SELF",
	};

	if (0 <= slot && slot < ARRAY_SIZE(slotStr))
	{
		return slotStr[slot];
	}

	return "UNKNOWN";
}
const char * pdm_dev_2_str(ETinDev_t type)
{
	static const char * devStr[ETINDEV_MAX] = {
		[ETINDEV_PDM] = "PDM",

		// service card
		[ETINDEV_MIO] = "MIO",
	};

	if (0 <= type && type < ARRAY_SIZE(devStr))
	{
		return devStr[type];
	}

	return "UNKNOWN";
}


static void pdm_dump_cpld_raw_reg(u_int16_t * pCpld, u_int8_t is_high)
{
	int i = 0;

	if (pCpld == NULL)
	{
		pdm_pr_notice("        nothing");
		return;
	}

	pdm_pr_notice("   \\\\offs ""00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	pdm_pr_notice("base\\\\    ""||-||-||-||-||-||-||-||-||-||-||-||-||-||-||-||");
	for (i = 0; i < 4; ++i)
	{
#define MT_PRT_REG_ARRAY(n, addr) (is_high ? (addr[n] >> 8) : (addr[n] & 0xFF))
		pdm_pr_notice("0x%02x:     %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
			(i * 16), ARRAY_ELE16fn(MT_PRT_REG_ARRAY, 0, pCpld));
		pCpld = &(pCpld[16]);
#undef MT_PRT_REG_ARRAY
	}

	return;
}

void pdm_dump_base(const SAllCardType_t * pCardType)
{
	int i;

	// raw register
	for (i = ARRAY_SIZE(g_cpld_chip) - 1; i >= 0; --i)
	{
		pdm_pr_notice("cpld %s", pdm_slot_2_str(i));
		pdm_dump_cpld_raw_reg((u_int16_t *)(g_cpld_chip[i]), 0);
	}

	// user-friendly infomation
#define MT_PRINT_INT_STATE(mask_mem_name, mask_bf_name, state_mem_name, state_bf_name) \
	pdm_pr_notice("%s: %#x, state: %#x", #mask_bf_name, MT_CHIP_GET_BF(g_pdm_chip, mask_mem_name, mask_bf_name), \
			MT_CHIP_GET_BF(g_pdm_chip, state_mem_name, state_bf_name))
#define MT_PRINT_ONLINE_STATE(mem_name, bf_name, slot_enum) \
	pdm_pr_notice("%s: %#x, %s", #bf_name, MT_CHIP_GET_BF(g_pdm_chip, mem_name, bf_name), pdm_dev_2_str(pCardType->card_type[slot_enum]))

	// 0x00
	MT_PRINT_REG_MEM_BF(code_system, CPLD_CODE_SYSTEM);
	// 0x01
	MT_PRINT_REG_MEM_BF(code_board, CPLD_CODE_BOARD);
	// 0x02
	MT_PRINT_REG_MEM_BF(version, CPLD_VERSION);
	// 0x03
	MT_PRINT_REG_MEM_BF(test_version, TEST_VERSION);
	// 0x04
	MT_PRINT_REG_MEM_BF(board_type, BOARD_TYPE);
	// 0x05
	MT_PRINT_REG_MEM_BF(ext_bom_type, BOARD_EXT_TYPE);
	MT_PRINT_REG_MEM_BF(ext_bom_type, BOM_TYPE);
	// 0x06
	MT_PRINT_REG_MEM_BF(pcb_version, PCB_VERSION);
	// 0x07
	MT_PRINT_REG_MEM_BF(blank_reg, BLANK_REG);
	// 0x08
	// 0x09


	MT_PRINT_REG_MEM_BF(cpu_reset, CLR_VOL_MONIR);

	MT_PRINT_REG_MEM_BF(cpu_reset, VOL_MONIR);

#undef MT_PRINT_ONLINE_STATE
#undef MT_PRINT_INT_STATE
}

void pdm_dump_sys_func(void)
{

	MT_PRINT_REG_MEM_BF(blank_reg, BLANK_REG);

	return;
}

ETinDev_t pdm_get_slot_card_type(int slot, const struct cpld_cmm_t * pCpld)
{
	static const ETinDev_t s_fixed_card_type[ETIN_SLOT_MAX] =
	{
		[ETIN_SLOT_PDM] = ETINDEV_PDM,

		[ETIN_SLOT_SELF] = ETINDEV_MIO,
	};

	// 1. fixed ?
	if (likely(s_fixed_card_type[slot] != ETINDEV_NULL))
	{
		return s_fixed_card_type[slot];
	}
	// TODO: bug

	return ETINDEV_NULL;
}

