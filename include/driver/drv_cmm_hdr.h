#pragma once

#ifdef __cpluscplus
extern "C" {
#endif

// device type
typedef enum
{
	ETINDEV_NULL = -1,
	ETINDEV_MIN = 0,
	ETINDEV_PDM = 0,

	// service card
	ETINDEV_MIO,

	// TODO: add new type above this sentence
	ETINDEV_MAX,
} ETinDev_t;

#define IS_CARD_TYPE_INVALID(x) ((x) < ETINDEV_NULL || ETINDEV_MAX <= (x))

#define IS_CARD_TYPE_ERR(x) ((x) < ETINDEV_MIN || ETINDEV_MAX <= (x))
#define IS_CARD_N_NULL(x) (x != ETINDEV_NULL)

/*
 * NOTE: slot 0 is a pseudo slot index, used by PDM driver; so the macro
 * *IS_SLOT_ERR* is only used by pdm driver, for other driver, you should
 * implement your own logic to check if the slot index is valid.
 */
typedef enum
{
	ETIN_SLOT_MIN = 0,
	ETIN_SLOT_PDM = 0,

	ETIN_SLOT_SELF,

	ETIN_SLOT_MAX,
} ETinSlot_t;

#define IS_SLOT_ERR(x) ((x) < 0 || ETIN_SLOT_MAX <= (x))

#define ARRAY_ELE2(x, ...) x, ##__VA_ARGS__, x, ##__VA_ARGS__
#define ARRAY_ELE4(x, ...) ARRAY_ELE2(x, ##__VA_ARGS__), ARRAY_ELE2(x, ##__VA_ARGS__)
#define ARRAY_ELE8(x, ...) ARRAY_ELE4(x, ##__VA_ARGS__), ARRAY_ELE4(x, ##__VA_ARGS__)
#define ARRAY_ELE16(x, ...) ARRAY_ELE8(x, ##__VA_ARGS__), ARRAY_ELE8(x, ##__VA_ARGS__)
#define ARRAY_ELE32(x, ...) ARRAY_ELE16(x, ##__VA_ARGS__), ARRAY_ELE16(x, ##__VA_ARGS__)
#define ARRAY_ELE64(x, ...) ARRAY_ELE32(x, ##__VA_ARGS__), ARRAY_ELE32(x, ##__VA_ARGS__)

#define ARRAY_ELE_TIN_MAX_SLOT_NUM(x, ...) ARRAY_ELE32(x, ##__VA_ARGS__)



#define ARRAY_ELE2fn(func, n, x, ...) func((n), x, ##__VA_ARGS__), func(((n) + 1), x, ##__VA_ARGS__)
#define ARRAY_ELE4fn(func, n, x, ...) ARRAY_ELE2fn(func, (n), x, ##__VA_ARGS__), ARRAY_ELE2fn(func, ((n) + 2), x, ##__VA_ARGS__)
#define ARRAY_ELE8fn(func, n, x, ...) ARRAY_ELE4fn(func, (n), x, ##__VA_ARGS__), ARRAY_ELE4fn(func, ((n) + 4), x, ##__VA_ARGS__)
#define ARRAY_ELE16fn(func, n, x, ...) ARRAY_ELE8fn(func, (n), x, ##__VA_ARGS__), ARRAY_ELE8fn(func, ((n) + 8), x, ##__VA_ARGS__)
#define ARRAY_ELE32fn(func, n, x, ...) ARRAY_ELE16fn(func, (n), x, ##__VA_ARGS__), ARRAY_ELE16fn(func, ((n) + 16), x, ##__VA_ARGS__)
#define ARRAY_ELE64fn(func, n, x, ...) ARRAY_ELE32fn(func, (n), x, ##__VA_ARGS__), ARRAY_ELE32fn(func, ((n)+32), x, ##__VA_ARGS__)

#define ARRAY_ELE_TIN_MAX_SLOT_NUMfn(func, n, x, ...) ARRAY_ELE32fn(func, (n), x, ##__VA_ARGS__)


#ifdef __cpluscplus
}
#endif


