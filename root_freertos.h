#ifndef root_freertos_h
#define root_freertos_h
///////////////////////////////////////////////////////////
#include "global_scope.h"
#include "root_nextion_callback.h"
#include "root_it_handle.h"
#include "root_freertos_task.h"
///////////////////////////////////////////////////////////
void root_nextion_init();
void root_setting_init();
void root_ethernet_init();
void root_sys_init();
///////////////////////////////////////////////////////////

void root_ethernet_init()
{
    ethernet_init(ETHERNET_SS_PIN, ETHERNET_RST_PIN, ethernet_handle);

    int ethernet_init_retry = 5;
    while (ethernet_begin() == 0)
    {
        ethernet_init_retry--;
        if (ethernet_init_retry < 0)
            break;
    }
}
///////////////////////////////////////////////////////////
void root_setting_init()
{
    if (digitalRead(SETTING_PIN) == LOW)
    {
        RootNextion.showMessage("Setting your device");
        printf("Entering setting mode\r\n");
        ButSettingMachineClickCallback();

        while (true)
        {
            RootNextion.Listening();
            vTaskDelay(10);
        }
    }

    ethernet_reset();
}
///////////////////////////////////////////////////////////
void root_nextion_init()
{
    RootNextion.init();
    RootNextion.PAGE_LOADING_EVENT->root_attachCallback(PAGE_LOADING_EVENT_CALLBACK);
}
///////////////////////////////////////////////////////////
void root_sys_init()
{
    io_init();

    mfrc522_init();

    printf("Read all of device parameter\r\n");

    eeprom_read_binterface_id(BKanban.BInterface.BinterfaceId);
    eeprom_read_mac(DeviceMac, sizeof(DeviceMac), EEPROM_MAC_ADDR);
    eeprom_read_server_par(ServerIp, sizeof(ServerIp), ServerPort, EEPROM_SERVER_PAR_ADDR);
    eeprom_read_user_info(BKanban.Cutting.Worker, EPPROM_USER_INFO_ADDR);
    eeprom_read_machine_info(BKanban.Machine, EEPROM_DEVICE_INFO_ADDR);
    ethernet_apply_par_changed();

    root_nextion_init();

    RootNextion.Waiting(30000, "Initializing...");

    root_ethernet_init();

    // Initialize task
    freertos_task_init();
}

#endif