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
