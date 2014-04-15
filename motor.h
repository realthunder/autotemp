void setupMotor() {
    pinMode(MOTOR_PIN,OUTPUT);
    analogWriteFrequency(MOTOR_PIN,187500);
}

#ifndef ENABLE_MOTOR_VARIABLE
int motor_speed;
unsigned motor_on_t;
unsigned motor_off_t;
unsigned motor_t;
int motor_on;

void loopMotor() {
    if(!motor_off_t) return;
    unsigned t = millis();
    unsigned mt;
    int speed;
    if(motor_on) {
        speed = 0;
        mt = motor_on_t;
    }else{
        speed = motor_speed;
        mt = motor_off_t;
    }
    if(t>=mt) {
        if(t - motor_t < mt) return;
    }else if(t < mt) //cheap hanlding of wrap around
        return;
    motor_t = t; //may have drift this way, but it's ok
    motor_on = !motor_on;
    analogWrite(MOTOR_PIN,speed);
}

#   ifdef ENABLE_SHELL
numvar mtCmd() {
    int n = getarg(0);
    motor_t = millis();
    motor_speed = 0;
    if(n==0)
        motor_off_t = 0;
    else {
        motor_speed = getarg(1);
        if(n<2)
            motor_off_t = 0;
        else {
            motor_on_t = getarg(2)*1000;
            motor_off_t = n>2?(getarg(3)*1000):(motor_on_t/2);
        }
    }
    analogWrite(MOTOR_PIN,motor_speed);
}
#   endif //ENABLE_SHELL
#else //ENABLE_MOTOR_VARIABLE
int motor_speed_min;
int motor_speed_max;
int motor_step;
int motor_speed;
unsigned motor_t;

void loopMotor() {
    if(!motor_step) return;
    unsigned t = millis();
    if(t>=motor_t) {
        if(t - motor_t < motor_step) return;
    }else if(t < motor_step) //cheap hanlding of wrap around
        return;
    motor_t = t; //may have drift this way, but it's ok
    int speed;
    if(motor_speed >= 0) {
        if(++motor_speed >= motor_speed_max) {
            speed = motor_speed_max;
            motor_speed = -motor_speed_max;
            if(motor_speed_min == motor_speed_max)
                motor_step = 0;
        }else
            speed = motor_speed;
    }else if(++motor_speed >= -motor_speed_min) {
        speed = motor_speed_min;
        motor_speed = motor_speed_min;
    }else
        speed = -motor_speed;
    analogWrite(MOTOR_PIN,speed);
}

#   ifdef ENABLE_SHELL
numvar mtCmd() {
    int n = getarg(0);
    motor_t = millis();
    motor_speed = 0;
    if(n==0)
        motor_step = 0;
    else if(n<2) {
        //even for constant speed, we slow start the motor
        //from zero speed to void current spike
        motor_speed_min = motor_speed_max = getarg(1);
        motor_step = 20;
    }else{
        //for oscillating speed, we still slow start from
        //zero initially to avoid current spike
        int interval = (n>2?getarg(3):5) *1000;
        motor_speed_min = getarg(1);
        motor_speed_max = getarg(2);
        if(motor_speed_max>motor_speed_min) {
            motor_step = interval/(motor_speed_max-motor_speed_min);
            if(motor_step < 20) motor_step = 20;
        }else
            motor_step = 0;
    }
    analogWrite(MOTOR_PIN,0);
}

#   endif //ENABLE_SHELL
#endif //ENABLE_MOTOR_VARIABLE
