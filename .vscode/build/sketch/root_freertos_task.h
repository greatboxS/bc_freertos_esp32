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
int get_queue_size();
void Task_Main(void *par);
void Task_GetLastCut(void *par);
void Task_NextionHandle(void *par);
void Task_EthernetHandle(void *par);
void Task_RFIDHandle(void *par);

void HandleTickTimer(TimerHandle_t xtimer);
void SensorCheckingTimer(TimerHandle_t pxTimer);
void RequestTimeout_Timer(TimerHandle_t pxTimer);
int DetectTick = 0;
bool Detected = false;
int previousDetect = 0;
int oldTick = 0;

static void freertos_task_init()
{
#pragma region Initializes Queue
    QueueHandle = xQueueCreate(MAX_QUEUE_LENGHT, sizeof(Ethernet_Request_t *));

    if (QueueHandle != NULL)
        printf("Create QueueHandle success\r\n");

    SemaphoreHandle = xSemaphoreCreateCounting(4, 4);

    if (SemaphoreHandle != NULL)
        printf("Create SemaphoreHandle success\r\n");

    EventGroupHandle = xEventGroupCreate();

    if (EventGroupHandle != NULL)
        printf("Create EventGroupHandle success\r\n");

#pragma endregion

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
        if (xTimerStart(Timer2, 0) != pdPASS)
        {
            printf("Can not start Timer3\r\n");
        }
        else
            printf("Start Timer3 successfully\r\n");
    }
    else
        printf("Timer3 = NULL\r\n");
#pragma endregion

#pragma region Initializes Task

    if (xTaskCreate(Task_Main, "MainTask", 1024 * 30, NULL, 1, &MainTask) == pdPASS)
        printf("Task_Main created successully\r\n");
    else
        printf("Task_Main created error\r\n");

    if (xTaskCreate(Task_GetLastCut, "Task1", 1024 * 30, NULL, 1, &Task1) == pdPASS)
        printf("Task_GetLastCut created successully\r\n");
    else
        printf("Task_GetLastCut created error\r\n");

    if (xTaskCreate(Task_EthernetHandle, "Task2", 1024 * 100, NULL, 1, &Task2) == pdPASS)
        printf("Task_EthernetHandle created successully\r\n");
    else
        printf("Task_EthernetHandle created error\r\n");

    if (xTaskCreate(Task_NextionHandle, "Task3", 1024 * 50, NULL, 1, &Task3) == pdPASS)
        printf("Task_NextionHandle created successully\r\n");
    else
        printf("Task_NextionHandle created error\r\n");

    if (xTaskCreate(Task_RFIDHandle, "Task4", 1024 * 30, NULL, 1, &Task4) == pdPASS)
        printf("Task_RFIDHandle created successully\r\n");
    else
        printf("Task_RFIDHandle created error\r\n");

    vTaskSuspendAll();
    vTaskResume(MainTask);

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

    Flag.LastCutTick_Handle();

    Flag.RequestTick_Handle();

    BKanban.Time.Tick_Handle();

    if (BKanban.Cutting.IsCutting)
    {
        BKanban.Cutting.sec++;
        if (BKanban.Cutting.sec >= 60)
        {
            BKanban.Cutting.sec = 0;
            BKanban.Cutting.RunTime++;
        }
    }

    EventBits_t request_event = xEventGroupGetBits(EventGroupHandle) & EVENT_REQUEST_OK;
    EventBits_t last_cut_event = xEventGroupGetBits(EventGroupHandle) & EVENT_GET_LAST_CUT_OK;

    if (BKanban.Time.Year() == 0 && request_event == 0 && last_cut_event != 0)
    {
        Ethernet_GetTime();
    }
}
//------------------------------> Timer3 <----------------------------------------//
void RequestTimeout_Timer(TimerHandle_t pxTimer)
{
    xEventGroupClearBits(EventGroupHandle, EVENT_REQUEST_OK);
    xTimerStop(pxTimer, (TickType_t)0);
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
    printf("Start Task1\r\n");
    vTaskResume(Task1);
    printf("Begin Main Task\r\n");
    for (;;)
    {
        int queueSize = get_queue_size();

        EventBits_t event_bit = xEventGroupWaitBits(EventGroupHandle,
                                                    EVENT_TASK1_OK | EVENT_TASK1_OK | EVENT_TASK3_OK | EVENT_TASK4_OK | EVENT_GET_LAST_CUT_OK,
                                                    pdTRUE, pdFALSE, (TickType_t)10);

        if ((event_bit & EVENT_TASK1_OK) != 0)
        {
            printf("Task1 OK ->Resume Task2");
            // Task 1 get last cut ok

            if ((event_bit & EVENT_GET_LAST_CUT_OK) != 0)
            {
                printf("Task1 done, delete this task\r\n");
                vTaskDelete(Task1);
            }

            // Check if the cutting not yet finish
            if (BKanban.Cutting.Continue && !BKanban.Cutting.IsCutting && queueSize <= 0)
            {
                bool send_queue = false;
                if (CurrentRequest != NULL)
                {
                    if (CurrentRequest->Cancel | CurrentRequest->request_ok)
                        send_queue = true;
                }
                else
                    send_queue = true;

                if (send_queue)
                {
                    printf("Resume last cutting\r\n");
                    ButContinueClickCallback();
                }
            }

            vTaskResume(Task2);
        }

        if ((event_bit & EVENT_TASK2_OK) != 0)
        {
            // Task 2 get last cut ok
            printf("Task2 OK -> Resume Task3\r\n");
            vTaskSuspend(Task2);
            vTaskResume(Task3);
        }

        if ((event_bit & EVENT_TASK3_OK) != 0)
        {
            // Task 3 get last cut ok
            printf("Task3 OK -> Resume Task4\r\n");
            vTaskSuspend(Task3);
            vTaskResume(Task4);
        }

        if ((event_bit & EVENT_TASK4_OK) != 0)
        {
            // Task 4 get last cut ok
            vTaskSuspend(Task4);

            if ((event_bit & EVENT_GET_LAST_CUT_OK) != 0)
            {
                printf("Task4 OK -> Resume Task2 \r\n");
                vTaskResume(Task2);
            }
            else
            {
                printf("Task4 OK -> Resume Task1 \r\n");
                vTaskResume(Task1);
            }
        }
    }
}

void Task_GetLastCut(void *par)
{
    for (;;)
    {
        int queueSize = get_queue_size();

        EventBits_t get_last_cut_event = xEventGroupGetBits(EventGroupHandle) & EVENT_GET_LAST_CUT_OK;

        if (get_last_cut_event == 0 && queueSize <= 0)
        {
            bool send_queue = false;
            if (CurrentRequest != NULL)
            {
                if (CurrentRequest->Cancel | CurrentRequest->request_ok)
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

        printf("Set bit to notify that Task1 have done\r\n");
        xEventGroupSetBits(EventGroupHandle, EVENT_TASK1_OK);

        vTaskDelay(2);
    }
}

void Task_EthernetHandle(void *par)
{
    for (;;)
    {
        ethernet_listen();

        int queue_space = get_queue_size();

        printf("Queue counter %d\r\n", queue_space);

        if (queue_space > 0)
        {
            bool remake = false;
            if (CurrentRequest == NULL)
                remake = true;

            else if (CurrentRequest->Cancel || CurrentRequest->request_ok)
                remake = true;

            if (remake)
            {
                printf("Queue receive\r\n");
                xQueueReceive(QueueHandle, &(CurrentRequest), (TickType_t)0);
            }
        }

        if (CurrentRequest != NULL)
        {
            printf("%s\r\n", CurrentRequest->request_url);
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
                    ethernet_send_request(CurrentRequest);
                }
            }
        }
        printf("Set bit to notify that Task2 have done\r\n");
        xEventGroupSetBits(EventGroupHandle, EVENT_TASK2_OK);

        vTaskDelay(2);
    }
}

void Task_NextionHandle(void *par)
{
    bool OldCutTime_Val = 0;
    for (;;)
    {
        if (mfrc522_read_new_tag())
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
        RootNextion.Listening();

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

        if (OldCutTime_Val != BKanban.Cutting.TotalCutTimes && !updateState)
        {
            RootNextion.SetPage_numberValue(CUTTING_PAGE, RootNextion.CuttingPageHandle.TOTAL_TIME, BKanban.Cutting.TotalCutTimes);
            if (BKanban.Cutting.IsCutting)
            {
                BKanban.Cutting.CurrentCutTimes++;
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
        }

        // 1 second trigger
        if (Flag.IsTrigger())
        {
            BKanban.EthernetState = ethernet_get_module_status();

            RootNextion.setLanStatus(BKanban.CurrentWindowId, BKanban.EthernetState == 1 ? true : false);

            if (BKanban.Cutting.IsCutting)
            {
                char temp[32]{0};
                snprintf(temp, sizeof(temp), "%d:%d", BKanban.Cutting.RunTime, BKanban.Cutting.sec);
                RootNextion.SetPage_stringValue(CUTTING_PAGE, RootNextion.CuttingPageHandle.TIME, temp);
            }

            if (BKanban.Cutting.IsCutting && !Flag.IsRequest)
            {
                if (BKanban.CurrentWindowId != CUTTING_PAGE && BKanban.CurrentWindowId != CONFIRM_SIZE_PAGE)
                    RootNextion.GotoPage(CUTTING_PAGE);
            }

            Nextion_UpdateTime();
            printf("Free heap size: %d\r\n", esp_get_free_heap_size());
        }

        printf("Set bit to notify that Task3 have done\r\n");
        xEventGroupSetBits(EventGroupHandle, EVENT_TASK3_OK);

        vTaskDelay(2);
    }
}

void Task_RFIDHandle(void *par)
{
    for (;;)
    {
        printf("Task_RFIDHandle take the semaphore\r\n");
        if (mfrc522_read_new_tag())
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

        printf("Set bit to notify that Task4 have done\r\n");
        xEventGroupSetBits(EventGroupHandle, EVENT_TASK4_OK);

        vTaskDelay(2);
    }
}

#endif