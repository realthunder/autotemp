#include <Servo.h>
int servo_pos;
int servo_step;
Servo myservo;   

INIT_TIMEOUT;

void setupServo() {
}

void loopServo() {
  if(!servo_step || !IS_TIMEOUT2(servo_step)) return;
  if(servo_pos > 0) {
      myservo.write(servo_pos);
      if(++servo_pos == 180) servo_pos = -180;
  }else{
      myservo.write(-servo_pos);
      ++servo_pos;
  }
  RESET_TIMEOUT;
}

numvar svCmd() {
    unsigned n = getarg(0);
    servo_step = 0;
    if(!n) {
        myservo.detach();
        return 0;
    }
    switch(getarg(1)) {
    case 1:
        myservo.attach(SERVO_PIN);
        servo_step = n<2?15:getarg(2);
        RESET_TIMEOUT;
        break;
    case 2:
        servo_pos = n<2?90:getarg(2);
        myservo.attach(SERVO_PIN);
        myservo.write(servo_pos);
        delay(100);
        myservo.detach();
        break;
    default:
        return -1;
    }
    sp("servo step: ");
    printInteger(servo_step,0,0);
    sp(", pos: ");
    printInteger(servo_pos,0,0);
    speol();
    return 0;
}
