#pragma once

#include "reg_def_toolkit.h"

/*
 * cpld definition
 */
typedef volatile u_int16_t fpga_reg_t;
/*
 * @rw: *RO* or *RW*
 */
// all bit-field are *rw*
#define FPGA_REG_RW(rw, x) \
		fpga_reg_t rw x
#define FPGA_REG_ADDR_GAP(prev, next) \
	fpga_reg_t const gap_##prev##next[next - prev - 1]

#define FPGA_REG_UNION(mem_name, x, ...) \
	MT_REG_BF_DEF_UNION(fpga_reg_t, mem_name, \
			x, ##__VA_ARGS__)

#define FPGA_REG_RW_UNION(mem_name, rw, x, ...) \
	MT_REG_BF_DEF_UNION(fpga_reg_t rw, mem_name, \
			x, ##__VA_ARGS__)


struct fpga_cmm_t
{
	// 0x00
	FPGA_REG_RW(RO,
		SOFT_VER);
	// 0x01
	FPGA_REG_ADDR_GAP(0x00, 0x02);
	// 0x02
	FPGA_REG_RW(RO,
		CHIP_CODE_H);
	// 0x03
	FPGA_REG_RW(RO,
		CHIP_CODE_L);
	// 0x04
	FPGA_REG_RW(RO,
		RELEASE_VER);
	// 0x05
	FPGA_REG_ADDR_GAP(0x04, 0x06);
	// 0x06
	FPGA_REG_RW(RO,
		BLANK_REG);
	// 0x07
	FPGA_REG_ADDR_GAP(0x06, 0x08);
	// 0x08
	FPGA_REG_RW(RO,
		DEBUG_VER);
	// 0x09 ~ 0x0F
	FPGA_REG_ADDR_GAP(0x08, 0x10);
	// 0x10
	FPGA_REG_UNION(hot_reset,
		RO		: 15,
		RW HOT_RESET	: 1);
	// 0x11 ~ 0x1F
	FPGA_REG_ADDR_GAP(0x10, 0x20);
	// 0x20
	FPGA_REG_UNION(int_mask,
		RO		: 15,
		RW INT_MASK	: 1);
	// 0x21
	FPGA_REG_UNION(int_state,
		RO		: 15,
		RO INT		: 1);
	// 0x22 ~ 0x7F
	FPGA_REG_ADDR_GAP(0x21, 0x80);
	// 0x80 ~ 0xFF
	FPGA_REG_ADDR_GAP(0x7F, 0x100);
};

typedef char char_fpga_cmm_aux[sizeof(struct fpga_cmm_t) == 0x100 * 2 ? 1 : -1];
