#include "platform_setup.h"
#include "hardware.h"

#include "irq.h"

void irq_enable(IRQn_Type IRQn, s32 irq_prio)
{
    uint32_t prioritygroup = NVIC_GetPriorityGrouping();

    NVIC_SetPriority(IRQn, NVIC_EncodePriority(prioritygroup, irq_prio, 0));
    NVIC_EnableIRQ(IRQn);
}
