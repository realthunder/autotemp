#include <Arduino.h>
#include <IRremote.h>
#include <Servo.h>
#include <Wire.h>
#include <bitlash.h>

#define INIT_TIMEOUT unsigned long _t_start = millis(), _t_tick;

// cheap timeout detection, when wrap around, the actual timeout may double
#define IS_TIMEOUT(_timeout) \
    (_t_tick=millis(),\
     _t_tick>=_t_start&&(_t_tick-_t_start)>=_timeout || \
        _t_tick<_t_start&&_t_tick>=_timeout)

// accurate timeout detection
#define IS_TIMEOUT2(_timeout) \
    (_t_tick=millis(),\
     _t_tick>=_t_start&&(_t_tick-_t_start)>=_timeout || \
        _t_tick<_t_start&&((0xffffffff-_t_start)+_t_tick)>=_timeout)


// #define DELAY_START 10000

// #define ENABLE_SERVO

//WARNING motor prefers high frequency pwm but conflict with irsend pin 5
// #define ENABLE_MOTOR 

#define ENABLE_IIC
#define ENABLE_IR
#define ENABLE_SHELL
#define ENABLE_SERVO

#define noop do{}while(0)

#define IR_DELAY 50

#define BUTTON_PIN 1
#define RECV_PIN 4
#define STATUS_PIN 13
#define SERVO_PIN 3
#define MOTOR_PIN 21

int buttonState;
int lastButtonState;

#ifdef ENABLE_IR
#   include "ir.h"
#else
#define setupIR() noop
#define loopIR() noop
#endif

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

#ifdef ENABLE_SHELL

numvar promptCmd() {
    sp(">\r\n");
    return 0;
}

void setupShell() {
    initBitlash(38400);
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
    addBitlashFunction("prompt",(bitlash_function) promptCmd);
}

void loopShell() {
    runBitlash();
}
#else
#   define setupShell() Serial.begin(38400)
#   define loopShell() noop
#endif

void setup() {
    pinMode(BUTTON_PIN,INPUT_PULLUP);
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

