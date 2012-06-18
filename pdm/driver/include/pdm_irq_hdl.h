#pragma once

#include <linux/interrupt.h>


#ifdef __cpluscplus
#error "are you ensure using this header file in C++???"
extern "C" {
#endif

extern int pdm_irq_init(
		irq_handler_t handler,
		const char *name, void *dev);
extern void pdm_irq_exit(void);


#ifdef __cpluscplus
}
#endif

