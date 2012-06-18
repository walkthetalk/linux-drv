
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#include <linux/string.h>

#include "pdm_drv_int.h"

#include "pdm_drv_regrw.h"

#include "pdm_drv_ext.h"


/*
 * address for all cards
 */
#define PDM_CARD_ADDR_SPACE_SIZE 0x2000000
static struct
{
	const phys_addr_t const phy_addr;
	unsigned long const addr_space;
	void __iomem * vir_addr;
} s_card_addr[ETIN_SLOT_MAX] =
{

	[ETIN_SLOT_PDM] = { 0, 0, NULL, },

	[ETIN_SLOT_SELF] = { 0xa0000000, PDM_CARD_ADDR_SPACE_SIZE, NULL, },
};

/*
 * hardware simulation
 */

EXPORT_SYMBOL(pdm_get_pfn);
unsigned long pdm_get_pfn(int slot)
{
	if (IS_SLOT_ERR(slot))
	{
		return 0;
	}
#ifdef __TIN_SIM_HW__
	return vmalloc_to_pfn(s_card_addr[slot].vir_addr);
#else
	return (s_card_addr[slot].phy_addr >> PAGE_SHIFT);
#endif
}

EXPORT_SYMBOL(pdm_get_addr_space_size);
unsigned long pdm_get_addr_space_size(int slot)
{
	if (IS_SLOT_ERR(slot))
	{
		return 0;
	}

	return s_card_addr[slot].addr_space;
}

struct cpld_b611_t * /*const*/ g_pdm_chip = NULL;
struct cpld_cmm_t * /*const*/ g_cpld_chip[ETIN_SLOT_MAX] = { NULL };


int pdm_ctor_hw(void)
{
	int i = 0;
	// 1. map
	// NOTE: because the CPM_CPLD is used by both pdm and cpm_drv, and it is
	// too difficult to split it, so the ioremap is issued by pdm module unified.
	pdm_pr_notice("start remap");

	for (i = 0; i < ARRAY_SIZE(s_card_addr); ++i)
	{
		if (s_card_addr[i].phy_addr == 0)
		{
			continue;
		}

#ifdef __TIN_SIM_HW__
		s_card_addr[i].vir_addr = vmalloc(s_card_addr[i].addr_space);
		if (s_card_addr[i].vir_addr)
		{
			memset(s_card_addr[i].vir_addr, 0x0, s_card_addr[i].addr_space);
		}
#else
		// remap
		s_card_addr[i].vir_addr
			= ioremap(s_card_addr[i].phy_addr, s_card_addr[i].addr_space);
#endif
		if (s_card_addr[i].vir_addr == NULL)
		{
			pdm_dtor_hw();
			return -EIO;
		}

		g_cpld_chip[i] = s_card_addr[i].vir_addr;
	}

	g_pdm_chip = s_card_addr[ETIN_SLOT_SELF].vir_addr;
#ifdef __TIN_SIM_HW__ // simulate slot id when simulating
	*(u_int16_t *)((unsigned long)g_pdm_chip + 0x04 * 2) = 0x11;
	*(u_int16_t *)((unsigned long)g_pdm_chip + 0x08 * 2) = 0x06;

//	g_pdm_chip->ms_state.v
//		= ((typeof(g_pdm_chip->ms_state)){ MT_INIT_BF(CPM2_STATS, 1), MT_INIT_BF(CPM1_STATS, 1), }).v;
#endif

//	MT_CHIP_ASS_BF(g_pdm_chip, misc, En_244_2, 0);

	return 0;
}

void pdm_dtor_hw(void)
{
	int i = 0;

	g_pdm_chip = NULL;

	for (i = ARRAY_SIZE(s_card_addr) - 1; i >= 0; --i)
	{
		if (s_card_addr[i].vir_addr == NULL)
		{
			continue;
		}

		g_cpld_chip[i] = NULL;
#ifdef __TIN_SIM_HW__
		vfree(s_card_addr[i].vir_addr);
#else
		// unmap
		iounmap(s_card_addr[i].vir_addr);
		s_card_addr[i].vir_addr = NULL;
#endif
	}

	return;
}

void pdm_raw_reg_read(char * pBuf, size_t nBytes, loff_t * pOff)
{
	int i = 0;

	// clear
	memset(pBuf, 0xCCCC, nBytes);

	// fill in
	for (i = 0; i < ARRAY_SIZE(s_card_addr); ++i)
	{
		const typeof(s_card_addr[i]) * tmp = &(s_card_addr[i]);
		loff_t tmpStart = max(*pOff, (loff_t)(tmp->phy_addr));
		loff_t tmpEnd = min(((loff_t)nBytes - 1 + *pOff), ((loff_t)tmp->addr_space - 1 + (loff_t)tmp->phy_addr));

		if (tmpEnd >= tmpStart)
		{
			size_t tmpLen = tmpEnd - tmpStart + 1;
			void * pDstStart = &(pBuf[tmpStart - *pOff]);
			const void * pSrcStart = (char *)(tmpStart - tmp->phy_addr + tmp->vir_addr);
			int j = 0;

			// all registers are 2bytes-width.
			for (j = 0; j < tmpLen; j += 2)
			{
				*(u_int16_t *)pDstStart = *(u_int16_t *)pSrcStart;
				pDstStart += 2;
				pSrcStart += 2;
			}
		}
	}

	return;
}

void pdm_raw_reg_write(const char * pBuf, size_t nBytes, loff_t * pOff)
{
	int i = 0;

	// fill in
	for (i = 0; i < ARRAY_SIZE(s_card_addr); ++i)
	{
		const typeof(s_card_addr[i]) * tmp = &(s_card_addr[i]);
		loff_t tmpStart = max(*pOff, (loff_t)(tmp->phy_addr));
		loff_t tmpEnd = min(((loff_t)nBytes - 1 + *pOff), ((loff_t)tmp->addr_space - 1 + (loff_t)tmp->phy_addr));

		if (tmpEnd >= tmpStart)
		{
			size_t tmpLen = tmpEnd - tmpStart + 1;
			const void * pSrcStart = &(pBuf[tmpStart - *pOff]);
			void * pDstStart = (char *)(tmpStart - tmp->phy_addr + tmp->vir_addr);
			int j = 0;

			// all registers are 2bytes-width.
			for (j = 0; j < tmpLen; j += 2)
			{
				*(u_int16_t *)pDstStart = *(u_int16_t *)pSrcStart;
				pDstStart += 2;
				pSrcStart += 2;
			}
		}
	}

	return;
}
