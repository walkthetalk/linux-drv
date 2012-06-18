#pragma once

/*
 * endian mode
 */
#ifdef DRV_KERNEL_SIDE
#include <asm/byteorder.h>

#if defined(__LITTLE_ENDIAN)
#if defined(__BIG_ENDIAN)
#error "both big and little endian are defined."
#endif
#endif

#if defined(__LITTLE_ENDIAN)

#define __BYTE_ORDER __LITTLE_ENDIAN
#define __BIG_ENDIAN 4321

#elif defined(__BIG_ENDIAN)

#define __BYTE_ORDER __BIG_ENDIAN
#define __LITTLE_ENDIAN 1234

#else
#error " I don't know big or little"
#endif

#else	// user side
#include <endian.h>
#endif

#include "ukc_types.h"

/*
 * get the number of args
 * NOTE: it can't be zero.
 */
#define PP_NARG(...) \
	PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
	PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
	_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
	_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
	_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
	_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
	_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
	_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
	_61,_62,_63,N,...) N
#define PP_RSEQ_N() \
	63,62,61,60,                   \
	59,58,57,56,55,54,53,52,51,50, \
	49,48,47,46,45,44,43,42,41,40, \
	39,38,37,36,35,34,33,32,31,30, \
	29,28,27,26,25,24,23,22,21,20, \
	19,18,17,16,15,14,13,12,11,10, \
	9,8,7,6,5,4,3,2,1,0

/*
 * concatenate two arguments
 * NOTE: it will unfold the arguments first
 */
#define MT_CONCATENATE(arg1, arg2) MT_CONCATENATE1(arg1, arg2)
#define MT_CONCATENATE1(arg1, arg2) MT_CONCATENATE2(arg1, arg2)
#define MT_CONCATENATE2(arg1, arg2) arg1##arg2
/*
 * traverse the arguments of a macro
 * forward or backward
 * @del:	delimiter
 */
#define MT_VA_FW(del, x, y, ...) \
	x del y, ##__VA_ARGS__
#define MT_VA_BW(del, x, y, ...) \
	y, ##__VA_ARGS__ del x

/*
 * @fbw:	*FW* or *BW* to represent forward or backward
 * @what:	how to modify *x*
 * @del:	delimiter
 */

#define MT_VA_FOR_EACH_16(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_15(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_15(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_14(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_14(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_13(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_13(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_12(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_12(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_11(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_11(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_10(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_10(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_9(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_9(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_8(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_8(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_7(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_7(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_6(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_6(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_5(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_5(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_4(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_4(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_3(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_3(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_2(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_2(fbw, what, prefix, del, x, ...)\
	MT_CONCATENATE(MT_VA_, fbw)( \
		del, \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, x), \
		MT_VA_FOR_EACH_1(fbw, what, prefix, del, __VA_ARGS__))
#define MT_VA_FOR_EACH_1(fbw, what, prefix, del, x, ...) \
	prefix what(x)


/*
 * forward traversing arguments
 * NOTE: the *...* must have one argument at lease.
 */
#define MT_VA_FW_FOR_EACH(what, prefix, subfix, ...) \
	MT_CONCATENATE(MT_VA_FOR_EACH_, PP_NARG(__VA_ARGS__)) \
		(FW, what, prefix, subfix, __VA_ARGS__)
// backward traversing arguments
#define MT_VA_BW_FOR_EACH(what, prefix, subfix, ...) \
	MT_CONCATENATE(MT_VA_FOR_EACH_, PP_NARG(__VA_ARGS__)) \
		(BW, what, prefix, subfix, __VA_ARGS__)

/*
 * macro for register definition
 */
#define MT_REG(x) x
// the property of register
#define RO const
#define RW

#if __BYTE_ORDER == __LITTLE_ENDIAN

# define MT_REG_BF_DEF(type, x, ...) \
	MT_VA_BW_FOR_EACH(MT_REG, type, ;, x, ##__VA_ARGS__)

#elif __BYTE_ORDER == __BIG_ENDIAN

# define MT_REG_BF_DEF(type, x, ...) \
	MT_VA_FW_FOR_EACH(MT_REG, type, ;, x, ##__VA_ARGS__)

#else
#error "I don't know the endian mode"
#endif

/*
 * there is no *rw* property about the *v*, for simulation write
 */
#define MT_REG_BF_DEF_UNION(type, mem_name, x, ...) \
	union \
	{ \
		type v; \
		struct \
		{ \
			MT_REG_BF_DEF(type, x, ##__VA_ARGS__); \
		} _bf; \
	} mem_name
#define MT_REG_BF_DEF_RW_UNION(type, rw, mem_name, x, ...) \
	union \
	{ \
		type v; \
		struct \
		{ \
			MT_REG_BF_DEF(type rw, x, ##__VA_ARGS__); \
		} _bf; \
	} mem_name

//#define MT_CHIP_MEM_ADDR(chip_addr, mem_name) (&chip_addr->mem_name)

/*
 * non-atomic version
 */
#define MT_CHIP_SET_BIT(chip_addr, mem_name, bit_name) \
	MT_CHIP_ASS_BF(chip_addr, mem_name, bit_name, 0x1)

#define MT_CHIP_CLR_BIT(chip_addr, mem_name, bit_name) \
	MT_CHIP_ASS_BF(chip_addr, mem_name, bit_name, 0x0)

#define MT_CHIP_CHG_BIT(chip_addr, mem_name, bit_name) \
	error? yes error

#define MT_CHIP_ASS_BIT(chip_addr, mem_name, bit_name, val) \
	MT_CHIP_ASS_BF(chip_addr, mem_name, bit_name, val)


/*
 * common
 */
/*
 * define an variable and assign the register value
 */
#define MT_CHIP_DEF_ASS(chip_addr, mem_name, var) \
	typeof(chip_addr->mem_name) var = chip_addr->mem_name
/*
 * reg in chip operation
 */
#define MT_CHIP_GET_BIT(chip_addr, mem_name, bit_name) \
	MT_CHIP_GET_BF(chip_addr, mem_name, bit_name)

#define MT_CHIP_REG_GET_BF(chip_mem, bf_name) \
	(((typeof(chip_mem)){ .v = (chip_mem).v, })._bf.bf_name)
#define MT_CHIP_GET_BF(chip_addr, mem_name, bf_name) \
	MT_CHIP_REG_GET_BF(chip_addr->mem_name, bf_name)
//	(chip_addr->mem_name._bf.bf_name)

/*
 * please see the gcc option "-fstrict-volatile-bitfields".
 * Q: maybe you will ask why ASS_BF but not SET_BF, because you have GET_BF?
 * A: the reason is bit operation, set is mean make it's value to 1.
 * NOTE: lower than 4.6, there is no this option, so what we can do is:
 */
#define MT_CHIP_REG_ASS_BF(chip_mem, bf_name, val) \
	do { \
		typeof(chip_mem) tmp = { .v = (chip_mem).v, }; \
		tmp._bf.bf_name = (val); \
		(chip_mem).v = tmp.v; \
	} while (0)

#define MT_CHIP_ASS_BF(chip_addr, mem_name, bf_name, val) \
	MT_CHIP_REG_ASS_BF(chip_addr->mem_name, bf_name, val)

/*
 * only for memory
 */
#define MT_REG_GET_BF(reg_name, bf_name) \
	reg_name._bf.bf_name

#define MT_INIT_BF(bf_name, val) \
	._bf.bf_name = val


/*
 * raw register operation
 */
#define MT_CHIP_RAW_REG_AND(chip_addr, mem_name, mem_mask) \
	chip_addr->mem_name.v &= mem_mask

#define MT_CHIP_RAW_REG_OR(chip_addr, mem_name, mem_mask) \
	chip_addr->mem_name.v |= mem_mask

/*
 * directly write, for optimization, no read operation. 
 */
#define MT_OP_REG_WRITE(chip_reg, bf_name, val) \
	do { \
		typeof(chip_reg) tmp = { MT_INIT_BF(bf_name, (val)), }; \
		(chip_reg).v = tmp.v; \
	} while (0)

#define MT_REG_COPY(dst_reg, src_reg) \
	do { \
		dst_reg.v = src_reg.v; \
	} while (0)

