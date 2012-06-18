
#include <assert.h>
#include <stddef.h>

#include "slot_converter.h"

static int g_slot_vir_2_phy[ETIN_SLOT_MAX] =
{
	[ETIN_SLOT_PDM] = 0,

	[ETIN_SLOT_SELF] = 0,
};

void gen_slot_converter(int self_phy_slot)
{
	assert(self_phy_slot == 1);

	g_slot_vir_2_phy[ETIN_SLOT_SELF] = self_phy_slot;
}

int conv_slot_vir_2_phy(int vir_slot)
{
	assert(!IS_SLOT_ERR(vir_slot));
	assert(g_slot_vir_2_phy[ETIN_SLOT_SELF] != 0);

	return g_slot_vir_2_phy[vir_slot];
}

int get_selfSlotID(void)
{
	return g_slot_vir_2_phy[ETIN_SLOT_SELF];
}
/*
 * convert *slotID* to string
 */
static const char * const s_dig2Str[ETIN_SLOT_MAX] =
{
	"0", "1",
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

const char * convDig2Str(int i)
{
	if (i < 0 || (int)ARRAY_SIZE(s_dig2Str) <= i)
	{
		assert(0);
		return NULL;
	}

	return s_dig2Str[i];
}


/*
 * convert the card tye *ETinDev_t* to string.
 * NOTE: please ensure the order is same as ETinDev_t
 */
static const char * const s_cardName[ETINDEV_MAX] =
{
	[ETINDEV_PDM] = "PDM",

	[ETINDEV_MIO] = "MIO",
};

const char * getCardTypeStr(ETinDev_t dev)
{
	if (IS_CARD_TYPE_ERR(dev))
	{
		return NULL;
	}

	return s_cardName[dev];
}
