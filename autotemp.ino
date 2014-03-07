#include <IRremote.h>

IRsend irsend;
#define CODE_5104(u,s) ((3<<10)|(u<<7)|s)
#define T_5104 1688 /* T=1.6879ms */
#define T2_5104 (T_5104/4)
#define INPUT_PIN 1

void setup() {
  Serial.begin(38400);
  pinMode(INPUT_PIN,INPUT_PULLUP);
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

void loop() {
    if(!digitalRead(INPUT_PIN)) {
        send5104(3,2);
        send5104(3,2);
    }
}
