#pragma once

#include "reg_def_toolkit.h"

/*
 * cpld definition
 */
typedef volatile u_int16_t cpld_reg_t;
/*
 * @rw: *RO* or *RW*
 */
#define CPLD_REG_ADDR_GAP(prev, next) \
	cpld_reg_t gap_##prev##next[next - prev - 1]

#define CPLD_REG_UNION(mem_name, x, ...) \
	MT_REG_BF_DEF_UNION(cpld_reg_t, mem_name, \
			: 8, \
			x, ##__VA_ARGS__)

#define CPLD_REG_RW_UNION(mem_name, rw, x, ...) \
	MT_REG_BF_DEF_RW_UNION(cpld_reg_t, rw, mem_name, \
			: 8, \
			x, ##__VA_ARGS__)


struct cpld_cmm_t
{
	// 0x00
	CPLD_REG_RW_UNION(code_system, RO,
			CPLD_CODE_SYSTEM	: 8);
	// 0x01
	CPLD_REG_RW_UNION(code_board, RO,
			CPLD_CODE_BOARD	: 8);
	// 0x02
	CPLD_REG_RW_UNION(version, RO,
			CPLD_VERSION	: 8);
	// 0x03
	CPLD_REG_RW_UNION(test_version, RO,
			TEST_VERSION	: 8);
	// 0x04
	CPLD_REG_RW_UNION(board_type, RO,
			BOARD_TYPE	: 8);
	// 0x05
	CPLD_REG_RW_UNION(ext_bom_type, RO,
			BOARD_EXT_TYPE	: 4,
			BOM_TYPE	: 4);
	// 0x06
	CPLD_REG_RW_UNION(pcb_version, RO,
					: 4,
			PCB_VERSION	: 4);
	// 0x07
	CPLD_REG_RW_UNION(blank_reg, RW,
			BLANK_REG	: 8);
	// 0x08
	CPLD_REG_RW_UNION(id, RO,
			SYSTEM_ID	: 2,
					: 1,
			SLOT_ID		: 5);
	// 0x09 ~ 0x0E
	CPLD_REG_ADDR_GAP(0x08, 0x0F);
	// 0x0F
	CPLD_REG_RW_UNION(clk_en, RW,
					: 7,
			CCLK_EN		: 1);
	// 0x10
	CPLD_REG_RW_UNION(fpga_done, RO,
					: 7,
			FPGA_DONE	: 1);
	// 0x11
	CPLD_REG_RW_UNION(fpga_prog_h, RW,
					: 7,
			FPGA_PROG_H	: 1);
	// 0x12
	CPLD_REG_RW_UNION(fpga_prog_l, RW,
					: 7,
			FPGA_PROG_L	: 1);
	// 0x13
	CPLD_REG_RW_UNION(fpga_loader, RW,
			FPGA_LOADER	: 8);
};

typedef char char_cpld_cmm_aux[sizeof(struct cpld_cmm_t) == 0x14 * 2 ? 1 : -1];
