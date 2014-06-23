#include <Wire.h>
#ifndef ENABLE_SHELL
unsigned thMonitorInterval=1000;
#else
unsigned thMonitorInterval=0;
#endif
int temperature;
int humidity;
int thReady=1;

void setupIIC() {
    Wire.begin();
}

int thUpdate(unsigned timeout) {
    char data[4];
    int i;
    INIT_TIMEOUT;
    if(thReady) {
        Wire.beginTransmission(0x27);
        Wire.endTransmission();
        thReady = 0;
    }
    while(1) {
        Wire.requestFrom(0x27, 4);
        for(i=0;i<4&&Wire.available();++i)
            data[i] = Wire.read();
        if(i!=4||(data[0]&0xc0)) {
            if(!timeout) return -1;
            delay(50);
            if(IS_TIMEOUT(timeout)) {
                thReady = 1;
                return -1;
            }
        }else {
            unsigned hum = (((data[0]&0x3f)<<8)+data[1])*1000/0x3ffe;
            unsigned temp = ((((data[2]<<8)+data[3])>>2)*1650/0x3ffe)-400;
            temperature = temp;
            humidity = hum;
            thReady = 1;
            return 0;
        }
    }
}

#ifdef ENABLE_SHELL
numvar thCmd() {
    if(getarg(0))
        thMonitorInterval = getarg(1);
    else if(!thUpdate(1000)) {
        sp("H: ");
        printInteger(humidity,0,0);
        sp(" T: ");
        printInteger(temperature,0,0);
        speol();
    }else
        sp("Timeout\r\n");
    return 0;
}
#endif

void loopIIC() {
    static unsigned long timer;
    unsigned long t;
    if(!thMonitorInterval) return;

    t = millis();
    if(thReady) {
        if(t<timer || (t-timer)>(thMonitorInterval<<10)) //cheap detection of wrap around
            return;
        timer += thMonitorInterval;
        if(timer < t) timer = t;
    }
    if(!thUpdate(0)) {
        Serial.print("H: ");
        Serial.print(humidity);
        Serial.print(" T:");
        Serial.println(temperature);
    }
}
