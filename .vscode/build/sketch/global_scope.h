#ifndef global_scope_h
#define global_scope_h

#define log_ln(...) printf("DEBUG: %s, line: %d\r\n%s\r\n", __func__, __LINE__, __VA_ARGS__)
#define f_log() printf("Func: %s, line: %d\r\n", __func__, __LINE__)

#define EEPROM_SERVER_PAR_ADDR 0
#define EPPROM_USER_INFO_ADDR 100
#define EEPROM_DEVICE_INFO_ADDR 200
#define EEPROM_CUT_ADDR 300
#define EEPROM_MAC_ADDR 310

#define NEXTION_SERIAL_NUM 2
#define NEXTION_BAUD 9600
#define NEXTION_TIMEOUT 100

#define MAX_QUEUE_LENGHT 10

#define configSUPPORT_STATIC_ALLOCATION 1

#include "esp_io_config.h"
#include "root_app_data.h"
#include "root_mfrc522.h"
#include "json_get_member.h"
#include "root_nextion.h"
#include "root_ethernet.h"
#include "esp_eeprom_config.h"
#include "freertos/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static RootNextionClass RootNextion = RootNextionClass(NEXTION_SERIAL_NUM, NEXTION_BAUD, NEXTION_TIMEOUT);
static HttpHeader_t httpHeader;
static BKanban_t BKanban;
static SysFlag_t Flag;
static Ethernet_Request_t xRequest;
static Ethernet_Request_t *CurrentRequest = NULL;
static Schedule_t SelectedSchedule;

static EventGroupHandle_t EventGroupHandle;
static QueueHandle_t QueueHandle;
static SemaphoreHandle_t SemaphoreHandle;
static TimerHandle_t Timer1, Timer2, Timer3;
static TaskHandle_t Task1, Task2, Task3, Task4, MainTask;

#define EVENT_GET_LAST_CUT_OK 1 << 0
#define EVENT_CONTINUE_OK 1 << 1
#define EVENT_REQUEST_OK 1 << 2

#define EVENT_TASK1_OK 1 << 4
#define EVENT_TASK2_OK 1 << 5
#define EVENT_TASK3_OK 1 << 6
#define EVENT_TASK4_OK 1 << 7
#endif