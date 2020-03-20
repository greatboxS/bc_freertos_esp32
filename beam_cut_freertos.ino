// creator: ryan-lenguyen - 18/3/2020

#define configMAX_SYSCALL_INTERRUPT_PRIORITY 5

#include "global_scope.h"
#include "root_freertos.h"

void setup()
{
    root_sys_init();
}

void loop()
{
    // every thing is done in task
    vTaskSuspend(NULL);
}