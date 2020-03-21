# 1 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
/* 

* creator: ryan-lenguyen 

* date: 18/3/2020

* github: https://github.com/greatboxS/bc_freertos_esp32

* gmail: phuockha0708@gmail.com

*/
# 8 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
# 9 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino" 2
# 10 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino" 2

void setup()
{
    // every thing is done in task
    // easy money!
    root_sys_init();
}

void loop()
{
    esp_task_wdt_reset();
    // every thing is done in task
    vTaskSuspend(
# 22 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino" 3 4
                __null
# 22 "e:\\Visual Code\\ESP32\\BSYS_FreeRTOS\\beam_cut_freertos.ino"
                    );
}
