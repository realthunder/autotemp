#define CODE_5104(u,s) ((3<<10)|(u<<7)|s)
#define T_5104 1688 /* T=1.6879ms */
#define T2_5104 (T_5104/4)

#define IR_5104 -2

IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results;

void setupIR() {
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
    irsend.enableIROut(38);
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
    }else if(codeType == IR_5104) {
        sp("Sent 5104 ");
        printInteger(codeLen,0,0);
        sp(",");
        printInteger(codeValue,0,0);
        send5104(codeLen,codeValue);
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
    for(i=2;i<=n;++i) 
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

