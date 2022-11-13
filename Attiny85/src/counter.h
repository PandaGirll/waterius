#ifndef _COUNTER_h
#define _COUNTER_h

#include <Arduino.h>
#include <avr/wdt.h>

// значения компаратора с pull-up
//    : замкнут (0 ом) - намур-замкнут (1к5) - намур-разомкнут (5к5) - обрыв линии
// 0  : ?                ?                     ?                       ?
// если на входе 3к3
// 3к3:  100-108 - 140-142  - 230 - 1000
//

// приведение к 8-ми разрядному значению АЦП
#define LIMIT_CLOSED       ((uint8_t) 29)
#define LIMIT_NAMUR_CLOSED ((uint8_t) 128)
#define LIMIT_NAMUR_OPEN   ((uint8_t) 200)


#define TRIES 3 //Сколько раз после окончания импульса
                //должно его не быть, чтобы мы зарегистририровали импульс

enum CounterState_e
{
    CLOSE,
    NAMUR_CLOSE,
    NAMUR_OPEN,
    OPEN
};

struct CounterB
{
    int8_t _checks; // -1 <= _checks <= TRIES

    uint8_t _pin;  // дискретный вход
    uint8_t _apin; // номер аналогового входа

    uint16_t adc;  // уровень замкнутого входа
    uint8_t state; // состояние входа

    explicit CounterB(uint8_t pin, uint8_t apin = 0)
        : _checks(-1), _pin(pin), _apin(apin), adc(0), state(CounterState_e::CLOSE)
    {
        DDRB &= ~_BV(pin); // INPUT
    }

    inline bool is_close(uint16_t a)
    {
        state = value2state(a);
        return state == CounterState_e::CLOSE || state == CounterState_e::NAMUR_CLOSE;
    }

    inline bool is_impuls(uint8_t a)
    {
        //Детектируем импульс когда он заканчивается!
        //По сути софтовая проверка дребега

        if (is_close(a))
        {
            _checks = TRIES;
            adc = a;
        }
        else
        {
            if (_checks >= 0)
            {
                _checks--;
            }
            if (_checks == 0)
            {
                return true;
            }
        }
        return false;
    }

    // Возвращаем текущее состояние входа
    inline enum CounterState_e value2state(uint8_t value)
    {
        if (value < LIMIT_CLOSED)
        {
            return CounterState_e::CLOSE;
        }
        else if (value < LIMIT_NAMUR_CLOSED)
        {
            return CounterState_e::NAMUR_CLOSE;
        }
        else if (value < LIMIT_NAMUR_OPEN)
        {
            return CounterState_e::NAMUR_OPEN;
        }
        else
        {
            return CounterState_e::OPEN;
        }
    }
};

struct ButtonB
{
    uint8_t _pin; // дискретный вход

    explicit ButtonB(uint8_t pin)
        : _pin(pin)
    {
        DDRB &= ~_BV(pin);   // INPUT
        PORTB &= ~_BV(_pin); // INPUT
    }

    inline bool digBit()
    {
        return bit_is_set(PINB, _pin);
    }

    // Проверка нажатия кнопки
    bool pressed()
    {

        if (digBit() == LOW)
        {                             //защита от дребезга
            delayMicroseconds(20000); //нельзя delay, т.к. power_off
            return digBit() == LOW;
        }
        return false;
    }

    // Замеряем сколько времени нажата кнопка в мс
    unsigned long wait_release()
    {

        unsigned long press_time = millis();
        while (pressed())
        {
            wdt_reset();
        }
        return millis() - press_time;
    }

    bool long_pressed()
    {
#ifdef MODKAM_VERSION
        return false;
#else
        return wait_release() > LONG_PRESS_MSEC;
#endif
    }
};

#endif