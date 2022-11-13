#ifndef _WATERIUS_SETUP_h
#define _WATERIUS_SETUP_h

#include <Arduino.h>

#define WATERIUS_2C 0 // attiny85 - 2 счетчика импульсов

/*
    Включение логирования
    3 pin TX -> RX (TTL-USB 3.3 или 5в), 9600 8N1
    При логировании не работает счетчик2 на 3-м пине (Вход 2).

    #define LOG_ON
*/

#ifndef LOG_ON
#define LOG_BEGIN(x)
#define LOG(x)
#else
#undef LOG_BEGIN
#undef LOG

// TinyDebugSerial на PB3 только в attiny85, 1MHz
#include "TinyDebugSerial.h"
#define LOG_BEGIN(x) mySerial.begin(x)
#define LOG(x)                \
    mySerial.print(millis()); \
    mySerial.print(F(" : ")); \
    mySerial.println(x);
#endif


/*
    Период пробуждения во время подсчёта
*/
#define WDT_PERIOD_COUNTING WDTO_30MS

/*
    Период проверки сторожевого таймера во время работы с ESP
*/
#define WDT_PERIOD_WAKEUP WDTO_250MS

/*
    Кол-во пробуждений за 1 минуту во время подсчёта
*/
#define ONE_SECOND 32

/*
    Период отправки данных на сервер, мин.
*/
#define WAKEUP_PERIOD_DEFAULT 1L * 60L * ONE_SECOND

/*
    Аварийное отключение, если ESP зависнет и не пришлет команду "сон".
*/
#define WAIT_ESP_MSEC 30000UL

/*
    Сколько милисекунд пользователь может
    настраивать ESP. Если не закончил, питание ESP выключится.
*/
#define SETUP_TIME_MSEC 600000UL

/*
    время долгого нажатия кнопки, милисекунд
*/
#ifndef MODKAM_VERSION
#define LONG_PRESS_MSEC  3000
#endif

struct Data
{
    uint32_t value0;
    uint32_t value1;
};

struct CounterState
{                   // не добавляем в Data, т.к. та в буфере кольцевом
    uint8_t state0; // состояние входа
    uint8_t state1;
};

struct ADCLevel
{
    uint16_t adc0;
    uint16_t adc1;
};

struct Header
{

    /*
    Версия прошивки
    */
    uint8_t version;

    /*
    Причина перезагрузки (регистр MCUSR datasheet 8.5.1):
         0001 - PORF: Power-on Reset Flag. Напряжение питания было низкое или 0.
         0010 - EXTRF: External Reset Flag. Пин ресет был в низком уровне.
         0100 - BORF: Brown-out Reset Flag. Напряжение питание было ниже требуемого.
         1000 - WDRF: Watchdog Reset Flag. Завершение работы таймера.

    8  - 1000 - WDRF
    9  - 1001 - WDRF + PORF
    10 - 1010 - WDRF + EXTRF
    */
    uint8_t service;

#ifdef MODKAM_VERSION
    /*
    Напряжение питания в мВ.
    */
    uint32_t voltage;
#else
    /*
    ver 24: убрал напряжение
    */
    uint16_t reserved;

    /*
    Для совместимости с 0.10.0.
    */
    uint8_t reserved2;

    /*
    Включение режима настройки
    */
    uint8_t setup_started_counter;
#endif

    /*
    Количество перезагрузок
    */
    uint8_t resets;

    /*
    Модификация
    0 - Классический. 2 счетчика
    1 - 4C2W. 4 счетчика
    */
    uint8_t model;

    CounterState states; // TODO убрать
    Data data;
    ADCLevel adc;

    // HEADER_DATA_SIZE

    uint8_t crc;
#ifdef MODKAM_VERSION
    uint8_t setup_started_counter;
#else
    uint8_t reserved3;
#endif
}; // 24 байт

#define HEADER_DATA_SIZE 22

#define TX_BUFFER_SIZE HEADER_DATA_SIZE + 2

#endif