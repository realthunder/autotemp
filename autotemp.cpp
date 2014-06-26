#include <Arduino.h>

#ifndef BAUD_RATE
#define BAUD_RATE 38400
#endif

#define INIT_TIMEOUT unsigned long _t_start = millis(), _t_tick;
#define RESET_TIMEOUT _t_start=millis()

// cheap timeout detection, when wrap around, the actual timeout may double
#define IS_TIMEOUT(_timeout) \
    (_t_tick=millis(),\
     (_t_tick>=_t_start&&(_t_tick-_t_start)>=_timeout) || \
        (_t_tick<_t_start&&_t_tick>=_timeout))

// accurate timeout detection
#define IS_TIMEOUT2(_timeout) \
    (_t_tick=millis(),\
     (_t_tick>=_t_start&&(_t_tick-_t_start)>=_timeout) || \
        (_t_tick<_t_start&&((0xffffffff-_t_start)+_t_tick)>=_timeout))


// #define DELAY_START 10000

//WARNING motor prefers high frequency pwm but conflict with irsend pin 5
#define ENABLE_MOTOR 

// #define ENABLE_IIC
#define ENABLE_IR
// #define ENABLE_SHELL
#define ENABLE_SERVO

#ifdef ENABLE_SHELL
#   include <bitlash.h>
#endif

#define noop do{}while(0)

#define IR_DELAY 50

#ifdef BOARD_PRO2
#   define BUTTON_PIN 7
#   define RECV_PIN 4
#   define STATUS_PIN 13
#   define MOTOR_PIN 6
// for mega board, pins 5 and 6 are paired on timer0. 
// Pins 9 and 10 are paired on timer1. Pins 3 and 11
// are paired on timer2. Servo library uses time1, so
// pwm function on pin 9 and 10 will be affected whether
// or not there is servo connected. IRRmote uses pin 3
// for transmitting. So it is only safe to change pwm
// frequency on pin 5 and 6, i.e. timer0.
#   define SERVO_PIN 9
#else
#   define BUTTON_PIN 1
#   define RECV_PIN 4
#   define STATUS_PIN 13
#   define MOTOR_PIN 21
#   define SERVO_PIN 3
#endif

int buttonState;
int lastButtonState;

#ifdef ENABLE_SERVO
#   include "servo.h"
#else
#define setupServo() noop
#define loopServo() noop
#endif

#ifdef ENABLE_MOTOR
#   include "motor.h"
#else
#   define setupMotor() noop
#   define loopMotor() noop
#endif

#ifdef ENABLE_IIC
#   include "i2c.h"
#else
#   define setupIIC() noop
#   define loopIIC() noop
#endif

#ifdef ENABLE_IR
#   include "ir.h"
#else
#define setupIR() noop
#define loopIR() noop
#endif

#ifdef ENABLE_SHELL

void setupShell() {
    initBitlash(BAUD_RATE);
#ifdef ENABLE_IIC
	addBitlashFunction("th", (bitlash_function) thCmd);
#endif
#ifdef ENABLE_IR
    addBitlashFunction("ir",(bitlash_function) irCmd);
#endif
#ifdef ENABLE_MOTOR
    addBitlashFunction("mt",(bitlash_function) mtCmd);
#endif
#ifdef ENABLE_SERVO
    addBitlashFunction("sv",(bitlash_function) svCmd);
#endif
}

void loopShell() {
    runBitlash();
}
#else
#   define setupShell() Serial.begin(BAUD_RATE)
#   define loopShell() noop
#endif

void setup() {
    pinMode(BUTTON_PIN,INPUT_PULLUP);
    lastButtonState = buttonState = digitalRead(BUTTON_PIN);
    setupShell();
    setupIR();
    setupServo();
    setupMotor();
    setupIIC();
}

void loop() {
#ifdef DELAY_START
    static int init;
    if(!init) {
        delay(DELAY_START);
        Serial.println("start");
        init = 1;
    }
#endif
    buttonState = digitalRead(BUTTON_PIN);
    loopShell();
    loopServo();
    loopIR();
    loopMotor();
    loopIIC();
    lastButtonState = buttonState;
}

