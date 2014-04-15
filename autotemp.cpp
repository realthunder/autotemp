#include <Arduino.h>
#include <IRremote.h>
#include <Servo.h>
#include <Wire.h>
#include <bitlash.h>

#define DELAY_START 10000
// #define ENABLE_SERVO
#define ENABLE_MOTOR
#define ENABLE_IIC
#define ENABLE_IR
#define ENABLE_SHELL
#define TRACE_IRSEND

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
    static int init;
    if(DELAY_START && !init) {
        delay(DELAY_START);
        Serial.println("start");
        init = 1;
    }
    buttonState = digitalRead(BUTTON_PIN);
    loopShell();
    loopServo();
    loopIR();
    loopMotor();
    loopIIC();
    lastButtonState = buttonState;
}

