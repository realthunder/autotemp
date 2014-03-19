#include <IRremote.h>
#include <Servo.h>
#include <Wire.h>

#define DELAY_START 10000
// #define ENABLE_SERVO
// #define ENABLE_MOTOR
#define ENABLE_IIC

#define noop do{}while(0)

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
    irrecv.enableIRIn();
    pinMode(STATUS_PIN, OUTPUT);
}

void send5104(unsigned user,unsigned code /* 1~8 for K1~K8 */) {
    int i;
    static const unsigned code_5104[] = {1,2,4,8,0x10,0x20,0x43,0x46};
    static const int tick_5104[] = {T2_5104,3*T2_5104};
    code = CODE_5104(user,code_5104[code-1]);
    irsend.enableIROut(38);
    for(i=11;i>=0;--i) {
        unsigned c = (code>>i)&1;
        irsend.mark(tick_5104[c]);
        irsend.space(tick_5104[c^1]);
    }
    irsend.space(4*T_5104);

}

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results) {
    codeType = results->decode_type;
    int count = results->rawlen;
    if (codeType == UNKNOWN) {
        Serial.print("Received unknown code, saving as raw, length:");
        codeLen = results->rawlen - 1;
        Serial.println(codeLen);
        // To store raw codes:
        // Drop first value (gap)
        // Convert from ticks to microseconds
        // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
        for (int i = 1; i <= codeLen; i++) {
            if(i%8 == 1)
                Serial.println("");
            if (i % 2) {
                // Mark
                rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
                Serial.print(" m");
            } 
            else {
                // Space
                rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
                Serial.print(" s");
            }
            Serial.print(rawCodes[i - 1], DEC);
        }
        Serial.println("");
    }
    else {
        if (codeType == NEC) {
            Serial.print("Received NEC: ");
            if (results->value == REPEAT) {
                // Don't record a NEC repeat value as that's useless.
                Serial.println("repeat; ignoring.");
                return;
            }
        } 
        else if (codeType == SONY) {
            Serial.print("Received SONY: ");
        } 
        else if (codeType == RC5) {
            Serial.print("Received RC5: ");
        } 
        else if (codeType == RC6) {
            Serial.print("Received RC6: ");
        } 
        else {
            Serial.print("Unexpected codeType ");
            Serial.print(codeType, DEC);
            Serial.println("");
        }
        Serial.println(results->value, HEX);
        codeValue = results->value;
        codeLen = results->bits;
    }
}

void sendCode(int repeat) {
    if (codeType == NEC) {
        if (repeat) {
            irsend.sendNEC(REPEAT, codeLen);
            Serial.println("Sent NEC repeat");
        } 
        else {
            irsend.sendNEC(codeValue, codeLen);
            Serial.print("Sent NEC ");
            Serial.println(codeValue, HEX);
        }
    } 
    else if (codeType == SONY) {
        irsend.sendSony(codeValue, codeLen);
        Serial.print("Sent Sony ");
        Serial.println(codeValue, HEX);
    } 
    else if (codeType == RC5 || codeType == RC6) {
        if (!repeat) {
            // Flip the toggle bit for a new button press
            toggle = 1 - toggle;
        }
        // Put the toggle bit into the code to send
        codeValue = codeValue & ~(1 << (codeLen - 1));
        codeValue = codeValue | (toggle << (codeLen - 1));
        if (codeType == RC5) {
            Serial.print("Sent RC5 ");
            Serial.println(codeValue, HEX);
            irsend.sendRC5(codeValue, codeLen);
        } 
        else {
            irsend.sendRC6(codeValue, codeLen);
            Serial.print("Sent RC6 ");
            Serial.println(codeValue, HEX);
        }
    } 
    else if (codeType == UNKNOWN /* i.e. raw */) {
        // Assume 38 KHz
        irsend.sendRaw(rawCodes, codeLen, 38);
        Serial.println("Sent raw");
    }
}

void loopIR() {
    // if(!digitalRead(INPUT_PIN))
    // send5104(3,2);

    // If button pressed, send the code.
    if (lastButtonState == LOW && buttonState == HIGH) {
        Serial.println("Released");
        irrecv.enableIRIn(); // Re-enable receiver
    }

    if (!buttonState) {
        Serial.println("Pressed, sending");
        digitalWrite(STATUS_PIN, HIGH);
        sendCode(lastButtonState == buttonState);
        digitalWrite(STATUS_PIN, LOW);
        delay(50); // Wait a bit between retransmissions
    } 
    else if (irrecv.decode(&results)) {
        digitalWrite(STATUS_PIN, HIGH);
        storeCode(&results);
        irrecv.resume(); // resume receiver
        digitalWrite(STATUS_PIN, LOW);
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

#define IIC_INTERVAL 1000

void setupIIC() {
    Wire.begin();
}

void loopIIC() {
    static int ready=1;
    static unsigned timer;
    char data[4];
    unsigned t = millis();
    if(ready) {
        if(t<timer || (t-timer)>IIC_INTERVAL<<10) //cheap detection of wrap around
            return;

        timer += IIC_INTERVAL;
        if(timer < t) timer = t;
        
        Wire.beginTransmission(0x27);
        Wire.endTransmission();
        ready = 0;
    }else{
        int i;
        Wire.requestFrom(0x27, 4);
        for(i=0;i<4&&Wire.available();++i)
            data[i] = Wire.read();
        Serial.println("");
        if(i!=4||(data[0]&0xc0)) {
            Serial.print("not ready: ");
            Serial.print(i);
            Serial.print(", 0x");
            Serial.println((data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3],HEX);
        }else {
            unsigned status = (data[0]&0xc0)>>6;
            unsigned hum = (((data[0]&0x3f)<<8)+data[1])*100/0x3ffe;
            unsigned temp = ((((data[2]<<8)+data[3])>>2)*165/0x3ffe)-40;
            Serial.print("H: ");
            Serial.print(hum);
            Serial.print(", T: ");
            Serial.println(temp);
            ready = 1;
        }
    }
}
#else
#   define setupIIC() noop
#   define loopIIC() noop
#endif



void setup() {
    Serial.begin(38400);
    pinMode(BUTTON_PIN,INPUT_PULLUP);
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
    loopServo();
    loopIR();
    loopMotor();
    loopIIC();
    delay(100);
    lastButtonState = buttonState;
}

