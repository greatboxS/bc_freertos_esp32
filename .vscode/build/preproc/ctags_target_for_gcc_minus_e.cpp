# 1 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
// creator: ryan-lenguyen - 18/3/2020

#define configMAX_SYSCALL_INTERRUPT_PRIORITY 5

# 6 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino" 2
# 7 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino" 2

void setup()
{
    root_freertos_init();

}

void loop()
{
    // every thing is done in task
    vTaskSuspend(
# 17 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino" 3 4
                __null
# 17 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
                    );
}
