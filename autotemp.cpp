#include <Arduino.h>
#include <IRremote.h>
#include <Servo.h>
#include <Wire.h>
#include <bitlash.h>

#define DELAY_START 10000
// #define ENABLE_SERVO
// #define ENABLE_MOTOR
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
#   define CODE_5104(u,s) ((3<<10)|(u<<7)|s)
#   define T_5104 1688 /* T=1.6879ms */
#   define T2_5104 (T_5104/4)

IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results;

void setupIR() {
    irsend.enableIROut(38);
    irrecv.enableIRIn();
    pinMode(STATUS_PIN, OUTPUT);
}

void send5104(unsigned user,unsigned code /* 1~8 for K1~K8 */) {
    int i;
    static const unsigned code_5104[] = {1,2,4,8,0x10,0x20,0x43,0x46};
    static const int tick_5104[] = {T2_5104,3*T2_5104};
    code = CODE_5104(user,code_5104[code-1]);
    for(i=11;i>=0;--i) {
        unsigned c = (code>>i)&1;
        irsend.mark(tick_5104[c]);
        irsend.space(tick_5104[c^1]);
    }
    irsend.space(4*T_5104);

}

int toggle = 0; // The RC5/6 toggle state
unsigned int rawBuf[RAWBUF];
int rawInit;
int codeType=-1;
unsigned long codeValue;
int codeLen;

// Stores the code for later playback
// Most of this code is just logging
void irDecode(decode_results *results,int width) {
    int type = results->decode_type;
    int count = results->rawlen;
    int len;
    if (type == UNKNOWN) {
        unsigned int *buf = rawBuf;
        len = results->rawlen - 1;
        // To store raw codes:
        // Drop first value (gap)
        // Convert from ticks to microseconds
        // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
        sp("ir(2");
        for (int i = 1; i <= len; i++) {
            unsigned int code = results->rawbuf[i];
            if(i>1 && width && (i-1)%width == 0) {
                sp(")");
                speol();
                sp("ir(3");
            }
            sp(",");
            printInteger(code,0,0);
            *buf++ = code;
        }
        sp(")\r\nir(1,");
        printInteger(type,0,0);
        sp(")\r\n");
        codeType = type;
        codeLen = len;
        rawInit = 0;
        return;
    } else if (type == NEC && results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        sp("REPEAT\r\n");
        return;
    }
    sp("ir(1,");
    printInteger(type,0,0);
    sp(",");
    printInteger(results->bits,0,0);
    sp(",0x");
    printHex(results->value);
    sp(")\r\n");
    codeLen = results->bits;
    codeValue = results->value;
}

int irSend(int repeat) {
    int i;
    int ret = -1;
    if(codeType == UNKNOWN /* i.e. raw */) {
        sp("Sent RAW");
        // Assume 38 KHz
        if(!rawInit) {
            for(i=0;i<codeLen;++i) {
                unsigned int code = rawBuf[i]*USECPERTICK;
                if(i&1) 
                    code += MARK_EXCESS; //space;
                else
                    code -= MARK_EXCESS; //mark
                rawBuf[i] = code;
            }
            rawInit=1;
        }
#ifdef TRACE_IRSEND
        for(i=0;i<codeLen;++i) {
            if((i&15)== 0)
                speol();
            printInteger(rawBuf[i],0,0);
            sp(",");
        }
#endif
        irsend.sendRaw(rawBuf, codeLen, 38);
    }else if (codeType == NEC) {
        if (repeat) {
            irsend.sendNEC(REPEAT, codeLen);
            sp("Sent NEC REPEAT");
        } 
        else {
            irsend.sendNEC(codeValue, codeLen);
            sp("Sent NEC ");
            printInteger(codeLen,0,0);
            sp(",");
            printHex(codeValue);
        }
    } else if (codeType == SONY) {
            irsend.sendSony(codeValue, codeLen);
            sp("Sent SONY ");
            printInteger(codeLen,0,0);
            sp(",");
            printHex(codeValue);
    } else if (codeType == RC5 || codeType == RC6) {
        if (!repeat) {
            // Flip the toggle bit for a new button press
            toggle = 1 - toggle;
        }
        // Put the toggle bit into the code to send
        codeValue &= ~(1 << (codeLen - 1));
        codeValue |= (toggle << (codeLen - 1));
        if (codeType == RC5) {
            sp("Sent RC5 ");
            printInteger(codeLen,0,0);
            sp(",");
            printHex(codeValue);
            irsend.sendRC5(codeValue, codeLen);
        } 
        else {
            irsend.sendRC6(codeValue, codeLen);
            sp("Sent RC6 ");
            printInteger(codeLen,0,0);
            sp(",");
            printHex(codeValue);
        }
    }else{
        sp("Invalid type");
        ret = -1;
    }
    speol();
    return ret;
}

void loopIR() {
#ifndef ENABLE_SHELL
    // If button pressed, send the code.
    if (lastButtonState == LOW && buttonState == HIGH) {
        Serial.println("Released");
        irrecv.enableIRIn(); // Re-enable receiver
    }

    if (!buttonState) {
        Serial.println("Pressed, sending");
        digitalWrite(STATUS_PIN, HIGH);
        irSend(lastButtonState == buttonState);
        digitalWrite(STATUS_PIN, LOW);
        delay(50); // Wait a bit between retransmissions
    } 
    else if (irrecv.decode(&results)) {
        digitalWrite(STATUS_PIN, HIGH);
        irrecv.resume(); // resume receiver
        digitalWrite(STATUS_PIN, LOW);
    }
#endif
}

// ir(0, <column width>, <wait period in ms>)
numvar irCmdRecv(unsigned n) {
    unsigned i;
    int width = n>1?getarg(2):16;
    unsigned t = millis()+(n>2?getarg(3)*1000:5000);
    irrecv.enableIRIn();
    for(i=0;i<IR_DELAY;++i) {
        delay(100);
        if(irrecv.decode(&results)) {
            digitalWrite(STATUS_PIN, HIGH);
            irDecode(&results,width);
            digitalWrite(STATUS_PIN, LOW);
            break;
        }
        if(t<=millis()) {
            sp("Timeout\r\n");
            return -1;
        }
    }
    return 0;
}

// ir(1,[type],<len>,<value>)
numvar irCmdSend(unsigned n) {
    unsigned i;
    codeType = getarg(2);
    if(codeType != UNKNOWN) {
        codeLen = getarg(3);
        codeValue = getarg(4);
    }
    irSend(0);
}

// ir(2,[values...])  reset codeLen=0
// ir(3,[values...])
numvar irCmdSetRaw(unsigned n) {
    unsigned i;
    if(codeLen+n-1>=RAWBUF) {
        sp("out of boundary");
        speol();
        return -1;
    }
    for(i=1;i<n;++i) 
        rawBuf[codeLen++] = getarg(i);
    rawInit = 0;
    return 0;
}

numvar irCmd() {
    unsigned n = getarg(0);
    if(n==0) {
        irSend(0);
        return 0;
    }
    switch(getarg(1)) {
    case 0:
        return irCmdRecv(n);
    case 1:
        return irCmdSend(n);
    case 2:
        codeLen=0;
        //fall through
    case 3:
        return irCmdSetRaw(n);
    default:
        return -1;
    }
}

#else
#define setupIR() noop
#define loopIR() noop
#endif

#ifdef ENABLE_SERVO
Servo myservo;   
void setupServo() {
    myservo.attach(SERVO_PIN);
}
void loopServo() {
  static int pos;
  if(pos >= 0) {
      myservo.write(pos);
      if(++pos == 180) pos = -180;
  }else{
      myservo.write(-pos);
      ++pos;
  }
  delay(15);
}
#else
#define setupServo() noop
#define loopServo() noop
#endif

#ifdef ENABLE_MOTOR

void setupMotor() {
    pinMode(MOTOR_PIN,OUTPUT);
}

void loopMotor() {
    static int motor_speed;
    static int lastButton;
    if (lastButtonState == HIGH && buttonState == LOW) {
        motor_speed += 50;
        if(motor_speed >= 255) motor_speed = 0;
        Serial.print("motor speed ");
        Serial.println(motor_speed);
        analogWrite(MOTOR_PIN,motor_speed);
    }
}
#else
#   define setupMotor() noop
#   define loopMotor() noop
#endif

#ifdef ENABLE_IIC

int thMonitorInterval=1000;
byte temperature;
byte humidity;
byte thIndex;

void setupIIC() {
    Wire.begin();
}

#ifdef ENABLE_SHELL
numvar thConfig(void) {
    thMonitorInterval = getarg(1);
    return 0;
}
numvar thRead(void) {
    printInteger(thIndex,0,0);
    sp(" H: ");
    printInteger(humidity,0,0);
    sp(" T: ");
    printInteger(temperature,0,0);
    speol();
    return 0;
}
numvar thCmd() {
    if(getarg(0)>0)
        thMonitorInterval = getarg(1);
    else
        thRead();
    return 0;
}
#endif

void loopIIC() {
    static int ready=1;
    static unsigned timer;
    char data[4];
    unsigned t;
    if(!thMonitorInterval) return;

    t = millis();
    if(ready) {
        if(t<timer || (t-timer)>(thMonitorInterval<<10)) //cheap detection of wrap around
            return;

        timer += thMonitorInterval;
        if(timer < t) timer = t;
        
        Wire.beginTransmission(0x27);
        Wire.endTransmission();
        ready = 0;
    }else{
        int i;
        Wire.requestFrom(0x27, 4);
        for(i=0;i<4&&Wire.available();++i)
            data[i] = Wire.read();
        if(i!=4||(data[0]&0xc0)) {
#ifndef ENABLE_SHELL
            Serial.print("not ready: ");
            Serial.print(i);
            Serial.print(", 0x");
            Serial.println((data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3],HEX);
#endif
        }else {
            unsigned status = (data[0]&0xc0)>>6;
            unsigned hum = (((data[0]&0x3f)<<8)+data[1])*100/0x3ffe;
            unsigned temp = ((((data[2]<<8)+data[3])>>2)*165/0x3ffe)-40;
            ++thIndex;
            temperature = temp;
            humidity = hum;
#ifndef ENABLE_SHELL
            thRead();
#endif
            ready = 1;
        }
    }
}
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

