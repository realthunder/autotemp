#include <Servo.h>
int servo_pos;
int servo_step;
int servo_max = 180;
Servo myservo;   

INIT_TIMEOUT;

void setupServo() {
}

void loopServo() {
  if(!servo_step || !IS_TIMEOUT2(servo_step)) return;
  if(servo_pos > 0) {
      myservo.write(servo_pos);
      if(++servo_pos >= servo_max) servo_pos = -servo_max;
  }else{
      myservo.write(-servo_pos);
      ++servo_pos;
  }
  RESET_TIMEOUT;
}

int sv(unsigned n, unsigned arg1, unsigned arg2) {
    servo_step = 0;
    if(!n) {
        myservo.detach();
        return 0;
    }
    switch(arg1) {
    case 1:
        servo_step = n<2?15:arg2;
        if(!servo_step)
            myservo.detach();
        else {
            myservo.attach(SERVO_PIN);
            RESET_TIMEOUT;
        }
        break;
    case 2:
        servo_pos = n<2?90:arg2;
        myservo.attach(SERVO_PIN);
        myservo.write(servo_pos);
        delay(100);
        myservo.detach();
        break;
    case 3:
        servo_max = n<2?180:arg2;
        break;
    default:
        return -1;
    }
#ifdef ENABLE_SHELL
    sp("servo step: ");
    printInteger(servo_step,0,0);
    sp(", pos: ");
    printInteger(servo_pos,0,0);
    speol();
#endif
    return 0;
}

#ifdef ENABLE_SHELL
numvar svCmd() {
    return sv(getarg(0),getarg(1),getarg(2));
}
#endif
