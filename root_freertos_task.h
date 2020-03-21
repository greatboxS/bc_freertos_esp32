#ifndef root_freertos_task_h
#define root_freertos_task_h

/******************System task and timer declare here***********
 * 
 * 
 *----------------< Task flow algorithm >----------------------*

        -> MainTask     <Synchronize task>
        -> Task 1       <Check last cut task> Need to be delete after get ok
        -> Task 2       <Check ethernet data task> 
        -> Task 3       <Nextion checking task>
        -> Task 4       <Rfid checking task>

 *-----------------------< Timer  >----------------------------*
        -> Timer1   <1s Timer>      Runtime timer
        -> Timer2   <50ms Timer>    Check external sensor
        -> Timer3   <5s Timer>      Check request waiting data timeout
*/

#include "global_scope.h"
#include "ethernet_request.h"
#include "root_nextion_callback.h"
///////////////////////////////////////////////////////////////////////////////////
void freertos_task_init();
void freertos_queue_init();
void freertos_timer_init();

void debug_serial_listen();

int get_queue_size();
void Task_Main(void *par);
void Task_get_last_cut(void *par);
void Task_nextion_screen(void *par);
void Task_ethernet_checking_data(void *par);
void Task_card_detect(void *par);
void Task_ethernet_send_request(void *par);

void HandleTickTimer(TimerHandle_t xtimer);
void SensorCheckingTimer(TimerHandle_t pxTimer);
void RequestTimeout_Timer(TimerHandle_t pxTimer);

int DetectTick = 0;
bool Detected = false;
int previousDetect = 0;
int oldTick = 0;
void freertos_timer_init()
{
#pragma region Initializes Timer
    Timer1 = xTimerCreate("Timer1", (1000), pdTRUE, NULL, HandleTickTimer);

    if (Timer1 != NULL)
    {
        if (xTimerStart(Timer1, 0) != pdPASS)
        {
            printf("Can not start Timer1\r\n");
        }
        else
            printf("start Timer1 successfully\r\n");
    }
    else
        printf("Timer1 = NULL\r\n");

    Timer2 = xTimerCreate("Timer2", (50), pdTRUE, NULL, SensorCheckingTimer);

    if (Timer2 != NULL)
    {
        if (xTimerStart(Timer2, 0) != pdPASS)
        {
            printf("Can not start Timer2\r\n");
        }
        else
            printf("Start Timer2 successfully\r\n");
    }
    else
        printf("Timer2 = NULL\r\n");

    Timer3 = xTimerCreate("TimeOutTimer", (5000), pdTRUE, NULL, RequestTimeout_Timer);

    if (Timer3 != NULL)
    {
        if (xTimerStart(Timer3, 0) != pdPASS)
        {
            printf("Can not start Timer3\r\n");
        }
        else
            printf("Start Timer3 successfully\r\n");
    }
    else
        printf("Timer3 = NULL\r\n");
#pragma endregion
}

void freertos_queue_init()
{
#pragma region Initializes Queue
    QueueHandle = xQueueCreate(MAX_QUEUE_LENGHT, sizeof(Ethernet_Request_t));

    if (QueueHandle != NULL)
        printf("Create QueueHandle success\r\n");

    SemaphoreHandle = xSemaphoreCreateCounting(4, 4);

    if (SemaphoreHandle != NULL)
        printf("Create SemaphoreHandle success\r\n");

    EventGroupHandle = xEventGroupCreate();

    if (EventGroupHandle != NULL)
        printf("Create EventGroupHandle success\r\n");

#pragma endregion
}

void freertos_task_init()
{
    freertos_queue_init();

    freertos_timer_init();
#pragma region Initializes Task

    esp_task_wdt_init(8, pdFALSE);

    if (xTaskCreatePinnedToCore(Task_Main, "MainTask", 1024 * 50, NULL, 1, &MainTask, 0) == pdPASS)
        printf("Task_Main created successully\r\n");
    else
        printf("Task_Main created error\r\n");

    if (xTaskCreatePinnedToCore(Task_get_last_cut, "Task1", 1024 * 10, NULL, 1, &Task1, 1) == pdPASS)
        printf("Task_get_last_cut created successully\r\n");
    else
        printf("Task_get_last_cut created error\r\n");

    if (xTaskCreatePinnedToCore(Task_ethernet_send_request, "Task2", 1024 * 10, NULL, 1, &Task2, 0) == pdPASS)
        printf("Task_ethernet_send_request created successully\r\n");
    else
        printf("Task_ethernet_send_request created error\r\n");

    if (xTaskCreatePinnedToCore(Task_ethernet_checking_data, "Task3", 1024 * 10, NULL, 1, &Task3, 1) == pdPASS)
        printf("Task_ethernet_checking_data created successully\r\n");
    else
        printf("Task_ethernet_checking_data created error\r\n");

    if (xTaskCreatePinnedToCore(Task_nextion_screen, "Task4", 1024 * 50, NULL, 1, &Task4, 1) == pdPASS)
        printf("Task_nextion_screen created successully\r\n");
    else
        printf("Task_nextion_screen created error\r\n");

    if (xTaskCreatePinnedToCore(Task_card_detect, "Task5", 1024 * 10, NULL, 1, &Task5, 1) == pdPASS)
        printf("Task_card_detect created successully\r\n");
    else
        printf("Task_card_detect created error\r\n");

#pragma endregion
}

///////////////////////////////////////////////////////////////////////////////////
int get_queue_size()
{
    if (QueueHandle != NULL)
        return (MAX_QUEUE_LENGHT - uxQueueSpacesAvailable(QueueHandle));
    return -1;
}

//------------------------------> Timer3 <----------------------------------------//
void HandleTickTimer(TimerHandle_t xtimer)
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    Flag.SubmitTick_Handle();

    BKanban.Time.Tick_Handle();

    BKanban.Cutting.CuttingTime_TickHandle();
}
//------------------------------> Timer3 <----------------------------------------//
void RequestTimeout_Timer(TimerHandle_t pxTimer)
{
    if (xEventGroupClearBits(EventGroupHandle, EVENT_REQUEST_OK))
        printf("-------------------------------------Clear flag ok\r\n");
    else
        printf("-------------------------------------Clear failed\r\n");

    xTimerStop(Timer3, (TickType_t)0);
}
//------------------------------> Timer3 <----------------------------------------//
void SensorCheckingTimer(TimerHandle_t pxTimer)
{
    if (digitalRead(EXTERNAL_INTERRUPT_PIN) == LOW && !Detected)
    {
        Detected = true;
        oldTick = millis();
    }

    if (Detected && millis() - oldTick > 2000)
    {
        oldTick = millis();
        Detected = false;
    }

    if (Detected)
    {
        DetectTick++;
        if (DetectTick >= 10)
        {
            if (digitalRead(EXTERNAL_INTERRUPT_PIN) == HIGH)
            {
                if (millis() - previousDetect > 2000)
                {
                    printf("New cut detected\r\n");

                    if (BKanban.Cutting.IsCutting)
                    {
                        BKanban.Cutting.CurrentCutTimes++;
                    }

                    BKanban.Cutting.TotalCutTimes++;
                    BKanban.Cutting.SubmitCutTime++;
                    DetectTick = 0;
                    Detected = false;
                    previousDetect = millis();
                }
            }
        }
    }
}
//--------------------------------------------------------------------------------//

void Task_Main(void *par)
{

    for (;;)
    {
        esp_task_wdt_reset();

        debug_serial_listen();

        EventBits_t event_bit = xEventGroupWaitBits(EventGroupHandle,
                                                    (EVENT_TASK1_OK | EVENT_TASK2_OK | EVENT_TASK3_OK | EVENT_TASK4_OK | EVENT_TASK5_OK),
                                                    pdFALSE, // do not clear bit before returning
                                                    pdFALSE, // donot wait for all bits
                                                    (TickType_t)10);

        event_bit = xEventGroupGetBits(EventGroupHandle);

        if ((event_bit & EVENT_REQUEST_OK) == 0)
        {
            int queueSize = get_queue_size();

            if (queueSize > 0)
            {
                bool remake = false;
                if (CurrentRequest == NULL)
                    remake = true;

                else if (CurrentRequest->Cancel || CurrentRequest->request_ok)
                    remake = true;

                if (remake)
                {
                    printf("Queue receive\r\n");
                    CurrentRequest = NULL;
                    xQueueReceive(QueueHandle, &QueueReceive, (TickType_t)0);
                    CurrentRequest = &QueueReceive;
                }
            }
        }

        if ((event_bit & EVENT_TASK1_OK) == EVENT_TASK1_OK) // Task_get_last_cut done, prepare to task2
        {
            printf("Resum Task2\r\n");
            vTaskResume(Task2);
            xEventGroupClearBits(EventGroupHandle, EVENT_TASK1_OK);
        }
        if ((event_bit & EVENT_TASK2_OK) == EVENT_TASK2_OK) // Task_get_last_cut done, prepare to task2
        {
            printf("Resum Task3\r\n");
            vTaskResume(Task3);
            xEventGroupClearBits(EventGroupHandle, EVENT_TASK2_OK);
        }
        if ((event_bit & EVENT_TASK3_OK) == EVENT_TASK3_OK) // Task_get_last_cut done, prepare to task2
        {
            printf("Resum Task4\r\n");
            vTaskResume(Task4);
            xEventGroupClearBits(EventGroupHandle, EVENT_TASK3_OK);
        }
        if ((event_bit & EVENT_TASK4_OK) == EVENT_TASK4_OK) // Task_get_last_cut done, prepare to task2
        {
            printf("Resum Task5\r\n");
            vTaskResume(Task5);
            xEventGroupClearBits(EventGroupHandle, EVENT_TASK4_OK);
        }
        if ((event_bit & EVENT_TASK5_OK) == EVENT_TASK5_OK) // Task_get_last_cut done, prepare to task2
        {
            if ((event_bit & EVENT_GET_LAST_CUT_OK) == EVENT_GET_LAST_CUT_OK) // already get the last cut so dont need to resum
            {
                printf("Resume Task2\r\n");
                vTaskResume(Task2);
            }
            else
            {
                printf("Resume Task1\r\n");
                vTaskResume(Task1);
            }
            xEventGroupClearBits(EventGroupHandle, EVENT_TASK5_OK);
        }
        vTaskDelay(2);
    }
}

void Task_get_last_cut(void *par)
{
    for (;;)
    {
        esp_task_wdt_reset();
        vTaskDelay(2);

        int queueSize = get_queue_size();

        EventBits_t event_grp = xEventGroupGetBits(EventGroupHandle);
        EventBits_t event_lc = event_grp & EVENT_GET_LAST_CUT_OK;
        EventBits_t event_req = event_grp & EVENT_REQUEST_OK;

        if (event_lc == 0 && queueSize <= 0 && event_req == 0)
        {
            bool send_queue = false;
            if (CurrentRequest != NULL)
            {
                if (CurrentRequest->Cancel)
                    send_queue = true;
            }
            else
                send_queue = true;

            if (send_queue)
            {
                printf("Get last cut\r\n");
                Ethernet_GetLastCut();
            }
        }

        printf("--->> Task1 have done\r\n");

        xEventGroupSetBits(EventGroupHandle, EVENT_TASK1_OK);

        // Task 1 get last cut ok
        if (event_lc == EVENT_GET_LAST_CUT_OK)
        {
            printf("Task1 done, delete this task\r\n");
            vTaskDelete(Task1);
        }

        vTaskSuspend(Task1);
    }
}

/*
Send request to server task
Must be disable after done to Synchronize with the main task
*/
void Task_ethernet_send_request(void *par)
{
    for (;;)
    {
        esp_task_wdt_reset();
        if (CurrentRequest != NULL)
        {
            printf("-------------------->> Request retry times: %d\r\n",CurrentRequest->retry);
            if (CurrentRequest->retry >= 5)
            {
                CurrentRequest->Cancel = true;
                printf("Block current request\r\n");
            }
            else
            {
                EventBits_t request_event = xEventGroupGetBits(EventGroupHandle) & EVENT_REQUEST_OK;

                if (request_event == 0 && !CurrentRequest->Cancel && !CurrentRequest->request_ok)
                {
                    printf("Send new request\r\n");
                    printf("----------------------->> %s\r\n", CurrentRequest->request_url);
                    ethernet_send_request(CurrentRequest);
                }
            }
        }
        vTaskDelay(2);
        printf("--->> Task2 have done\r\n");
        xEventGroupSetBits(EventGroupHandle, EVENT_TASK2_OK);
        vTaskSuspend(Task2);
    }
}

void Task_ethernet_checking_data(void *par)
{
    for (;;)
    {
        esp_task_wdt_reset();

        ethernet_listen();

        printf("--->> Task3 have done\r\n");

        xEventGroupSetBits(EventGroupHandle, EVENT_TASK3_OK);

        vTaskDelay(2);
        vTaskSuspend(Task3);
    }
}

void Task_nextion_screen(void *par)
{
    uint32_t OldCutTime_Val = 0;
    for (;;)
    {
        esp_task_wdt_reset();

        RootNextion.Listening();

        if (OldCutTime_Val != BKanban.Cutting.TotalCutTimes)
        {
            RootNextion.SetPage_numberValue(CUTTING_PAGE, RootNextion.CuttingPageHandle.TOTAL_TIME, BKanban.Cutting.TotalCutTimes);
            if (BKanban.Cutting.IsCutting)
            {
                RootNextion.SetPage_numberValue(CUTTING_PAGE, RootNextion.CuttingPageHandle.CUTTING_TIME, BKanban.Cutting.CurrentCutTimes);
            }
            OldCutTime_Val = BKanban.Cutting.TotalCutTimes;
        }

        //reset ethernet module every 30 min
        if (Flag.SysTick >= 1800)
        {
            mfrc.PCD_Init();
            Flag.SysTick = 0;
            int maintain_status = Ethernet.maintain();
            // #define DHCP_CHECK_NONE         (0)
            // #define DHCP_CHECK_RENEW_FAIL   (1)
            // #define DHCP_CHECK_RENEW_OK     (2)
            // #define DHCP_CHECK_REBIND_FAIL  (3)
            // #define DHCP_CHECK_REBIND_OK    (4)

            if (maintain_status != 2 && maintain_status != 4)
            {
                ethernet_reset();
                ethernet_begin();
            }

            Ethernet_GetTime();
        }

        if (Flag.IsCutSubmit())
        {
            Ethernet_SubmitCuttingTime();
        }

        if (Flag.SequenceSubmit_Tick)

            // 1 second trigger
            if (Flag.IsTrigger())
            {
                BKanban.EthernetState = ethernet_get_module_status();

                RootNextion.setLanStatus(BKanban.CurrentWindowId, BKanban.EthernetState == 1 ? true : false);

                Nextion_UpdateTime();

                RootNextion.SetPage_stringValue(HIS_PAGE, RootNextion.HisPageHandle.DEVICE_NAME, BKanban.Machine.MachineName);

                printf("Free heap size: %d\r\n", esp_get_free_heap_size());
            }

        // New page event
        if (BKanban.IsPageLoading())
        {
            switch (BKanban.CurrentPageId)
            {
            case HIS_PAGE:
                BKanban.CurrentWindowId = BKanban.CurrentPageId;
                Nextion_UpdateHisPage();
                break;
            case SEARCH_PAGE:
                Nextion_UpdateSearchPage();
                BKanban.CurrentWindowId = BKanban.CurrentPageId;
                break;

            case INFO_PAGE:
                BKanban.CurrentWindowId = BKanban.CurrentPageId;
                Nextion_UpdateInfoPage();
                break;

            case CUTTING_PAGE:
                BKanban.CurrentWindowId = BKanban.CurrentPageId;
                Nextion_UpdateCuttingPage();
                break;

            case CONFIRM_SIZE_PAGE:
                break;

            case COMPONENT_PAGE:
                Nextion_UpdateComponentPage();
                break;

            case MACHINE_PAGE:
                Nextion_UpdateMachinePage();
                break;

            case NEW_USER_PAGE:
                break;

            case LOGIN_PAGE:
                Nextion_UpdateUserPage();
                break;

            default:
                break;
            }
        }

        printf("--->> Task4 have done\r\n");

        xEventGroupSetBits(EventGroupHandle, EVENT_TASK4_OK);

        vTaskDelay(2);
        vTaskSuspend(Task4);
    }
}

void Task_card_detect(void *par)
{
    for (;;)
    {
        esp_task_wdt_feed();
        if (mfrc522_read_new_tag() && !BKanban.Cutting.IsCutting)
        {
            Output_Alarm();

            if (BKanban.CurrentPageId == NEW_USER_PAGE)
            {
                RootNextion.SetPage_stringValue(NEW_USER_PAGE, RootNextion.NewUserHandle.NEW_USER_RFID, (char *)TagNumber);
            }
            else
            {
                memccpy(BKanban.Cutting.Worker.UserRFID, TagNumber, 0, sizeof(BKanban.Cutting.Worker.UserRFID));
                Ethernet_GetWorkerInfo();
            }
        }

        printf("--->> Task5 have done\r\n");

        xEventGroupSetBits(EventGroupHandle, EVENT_TASK5_OK);

        vTaskDelay(2);
        vTaskSuspend(Task5);
    }
}

void debug_serial_listen()
{
    if (Serial.available())
    {
        String s = Serial.readString();
        if (s.indexOf("save") > -1)
        {
            char server[] = "Hello world";
            int port = 100;
            eeprom_write_server_par(server, sizeof(server), port, EEPROM_SERVER_PAR_ADDR);
        }
    }
}
#endif