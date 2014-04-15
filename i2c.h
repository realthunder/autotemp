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
