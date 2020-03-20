#ifndef esp_io_config_h
#define esp_io_config_h
#include <Arduino.h>

#define ETHERNET_SS_PIN GPIO_NUM_25
#define ETHERNET_RST_PIN GPIO_NUM_26

#define MFRC522_SS_PIN GPIO_NUM_32
#define MFRC522_RST_PIN GPIO_NUM_33

#define EXTERNAL_INTERRUPT_PIN GPIO_NUM_5
#define SETTING_PIN GPIO_NUM_15
#define ALARM_PIN GPIO_NUM_13

static void external_interrupt_init(void(callback_func)())
{
    printf("Setup external interrupt callback\r\n");
    pinMode(EXTERNAL_INTERRUPT_PIN, INPUT);
    attachInterrupt(EXTERNAL_INTERRUPT_PIN, callback_func, FALLING);
}

static void io_init()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    printf("Setup external interrupt pin\r\n");
    pinMode(SETTING_PIN, INPUT_PULLUP);

    printf("Setup external interrupt pin\r\n");
    pinMode(EXTERNAL_INTERRUPT_PIN, INPUT);

    printf("Setup alarm io\r\n");
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    pinMode(ALARM_PIN, OUTPUT);
    digitalWrite(ALARM_PIN, HIGH);

    printf("Setup ethernet io\r\n");
    pinMode(ETHERNET_SS_PIN, OUTPUT);
    pinMode(ETHERNET_RST_PIN, OUTPUT);
    digitalWrite(ETHERNET_SS_PIN, HIGH);
    digitalWrite(ETHERNET_RST_PIN, HIGH);
}

static void Output_Alarm(uint8_t count = 1)
{
    for (size_t i = 0; i < count; i++)
    {
        digitalWrite(ALARM_PIN, LOW);
        delay(50);
        digitalWrite(ALARM_PIN, HIGH);
        delay(50);
    }
}
#endif