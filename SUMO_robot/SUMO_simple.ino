/*
 * This code is not guaranteed to work for your robot; however, this gives one good idea of program flow.
 * This code also does not program for a second line sensor. Notice the use of counters. How will you
 * implement code for the second line sensor?
 */
// libraries
#include "Timer.h"

// function prototype declarations
void read_sensors();
void motor_forward(int motor_speed);
void motor_backward(int motor_speed);
void motor_stop();
void motor_turn_right(int motor_speed);
void motor_turn_left(int motor_speed);
void motor_back_right(int motor_speed);
void motor_back_left(int motor_speed);

// global variable declarations
int front_sensor; //see "read_sensors" function for more details
int front_sensor_old; //the old value is used to achieve more flexibility with the sensor reading
int line_sensor;
int front_sensor_counter; //only take front sensor readings if this counter is 0
int line_sensor_counter; //only take line sensor readings if this counter is 0

//speed ranges from 0 to 255 (PWM)
int slower_speed = 120;
int default_speed = 200;
int faster_speed = 230;
int fastest_speed = 255;

//reading period (ms)
int reading_period = 1;

//Timer object
Timer t;

// setup function, runs once upon system start-up
void setup() {
	//pinMode settings
	pinMode(13, OUTPUT);
	pinMode(12, OUTPUT);
	pinMode(11, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(6, OUTPUT);
	
	//wait for 3 seconds before movement
	delay(3000);
	
	//initialize sensor counters to 0 (start reading)
	front_sensor_counter = 0;
	line_sensor_counter = 0;
	
	//timer functions. Call "read_sensors" every "reading_period" ms
	//read_sensors reads the sensor values and change movement based on the values
	//this is where the main strategy goes
	t.every(reading_period, read_sensors);
  
}

// loop funtion, runs repeatedly
void loop() {
	t.update();
}

// function definitions

/* make the motors move forward
   takes an int argument for motor_speed
   returns nothing */
void motor_forward(int motor_speed) {
	analogWrite(11, motor_speed);
	analogWrite(6, motor_speed);
  
	digitalWrite(13, HIGH);
	digitalWrite(12, LOW);
	digitalWrite(8, HIGH);
	digitalWrite(7, LOW);
}

/* make the motors move backward
   takes an int argument for motor_speed
   returns nothing */
void motor_backward(int motor_speed) {
	analogWrite(11, motor_speed);
	analogWrite(6, motor_speed);
  
	digitalWrite(13, LOW);
	digitalWrite(12, HIGH);
	digitalWrite(8, LOW);
	digitalWrite(7, HIGH);
  
}

/* stop the motors from moving
   takes no argument
   returns nothing */
void motor_stop() {
	digitalWrite(11, LOW);
	digitalWrite(6, LOW);
 
	digitalWrite(13, LOW);
	digitalWrite(12, LOW);
	digitalWrite(8, LOW);
	digitalWrite(7, LOW);
}

/* make the motors turn right
   takes an int argument for motor_speed
   returns nothing */
void motor_turn_right(int motor_speed){
	analogWrite(11, motor_speed);
	analogWrite(6, motor_speed/1.5);
  
	digitalWrite(13, HIGH);
	digitalWrite(12, LOW);
	digitalWrite(8, HIGH);
	digitalWrite(7, LOW);
}

/* make the motors turn left
   takes an int argument for motor_speed
   returns nothing */
void motor_turn_left(int motor_speed){
	analogWrite(11, motor_speed/1.5);
	analogWrite(6, motor_speed);
  
	digitalWrite(13, HIGH);
	digitalWrite(12, LOW);
	digitalWrite(8, HIGH);
	digitalWrite(7, LOW);
}

/* make the motors turn back left
   takes an int argument for motor_speed
   returns nothing */
void motor_back_left(int motor_speed){
	analogWrite(11, motor_speed/1.5);
	analogWrite(6, motor_speed);
  
	digitalWrite(13, LOW);
	digitalWrite(12, HIGH);
	digitalWrite(8, LOW);
	digitalWrite(7, HIGH);

}

/* make the motors turn back right
   takes an int argument for motor_speed
   returns nothing */
void motor_back_right(int motor_speed){
	analogWrite(11, motor_speed);
	analogWrite(6, motor_speed/1.5);
  
	digitalWrite(13, LOW);
	digitalWrite(12, HIGH);
	digitalWrite(8, LOW);
	digitalWrite(7, HIGH);

}


/* read the sensor values from analog ports
   takes no argument
   returns nothing
   This is where the main strategy is*/
void read_sensors(){
  //Note: the use of the two counters (instead of the delay function) ensures when the front sensor is taking an action (counting down),
  //the program will not stop to wait for it, so the line sensor can also be functional at the same time
  //There are other implementations that can achieve the same effect, feel free to be creative
  
  //read line sensor first
  if(line_sensor_counter == 0){ //starts reading line sensor when line_sensor_counter is 0
  
    line_sensor = analogRead(A1); //read the line sensor value
    
    if(line_sensor >= 700){ //if the value is large, the white border is encountered, otherwise, the bot is still in the arena
      //set the counter so the back-off action will last approximately 1200 millisecond
      line_sensor_counter = 1200 / reading_period; 
      //set speed and direction
      motor_backward(slower_speed);
      front_sensor_counter = 0; //reset front_sensor_counter because line sensor takes higher priority
      return; //skip reading the front sensor because the white border has been detected (need to back-off immediately)
    } // if line_sensor is larger than 700, the program will simply move on to read the front sensor ()
    
  } else { //once the counter has been set to 120 (white border encountered), the else case will be executed until line_sensor_counter decrements to 0
    line_sensor_counter--;
    return; //this return ensures the front sensor is NOT read while back-off action is in progress
  } 
  
  
  // read front sensor after line sensor has been read and confirmed safe
  if(front_sensor_counter == 0){ //starts reading front sensor when front_sensor_counter is 0
    front_sensor_old = front_sensor; //store the old value first
    front_sensor = analogRead(A0); //read the front sensor value
    if(front_sensor_old <= 400){ //a value smaller than 400 means something is relatively far away (or nothing is there)
      if(front_sensor <= 400) { //old value is far, new value is also far, do nothing, keep default movement
        motor_turn_right(default_speed);
      } else { //old value is far, new value is close, something has been detected, charge at it
        front_sensor_counter = 500 / reading_period; //set the counter so this charging action will take approximately 500 millisecond
        motor_forward(faster_speed);
      }
    } else {
      //old value is close, new value is far, chasing at target for a little bit
      //Note: due to the limitation of the proximity sensor (10cm to 30cm model), sometimes an object that's extremely close (less than 10cm) 
      //will also results in a reading smaller than 400 (same as far away), so it's a good idea to keep charging for a little bit in this case
      if(front_sensor <= 400) { 
        front_sensor_counter = 300 / reading_period; //set the counter so this charging action will take approximately 300 millisecond
        motor_forward(fastest_speed);
      } else { //old value is close, new value is close, keep charging
        motor_forward(faster_speed);
      }
    }
  } else {
    front_sensor_counter--; //decrements front_sensor_counter in the case of something been detected
  }
  /* if you are using an ultrasonic sensor
   if(front_sensor_counter == 0)
   {
    front_sensor_old = front_sensor;
    read_US();
    if(front_sensor_old <= 400){ //a value smaller than 400 means something is relatively far away (or nothing is there)
      if(front_sensor <= 400) { //old value is far, new value is also far, do nothing, keep default movement
        motor_turn_right(default_speed);
      } else { //old value is far, new value is close, something has been detected, charge at it
        front_sensor_counter = 500 / reading_period; //set the counter so this charging action will take approximately 500 millisecond
        motor_forward(faster_speed);
      }
    } else {
      //old value is close, new value is far, chasing at target for a little bit
      //Note: due to the limitation of the proximity sensor (10cm to 30cm model), sometimes an object that's extremely close (less than 10cm) 
      //will also results in a reading smaller than 400 (same as far away), so it's a good idea to keep charging for a little bit in this case
      if(front_sensor <= 400) { 
        front_sensor_counter = 300 / reading_period; //set the counter so this charging action will take approximately 300 millisecond
        motor_forward(fastest_speed);
      } else { //old value is close, new value is close, keep charging
        motor_forward(faster_speed);
      }
    }
  } else {
    front_sensor_counter--; //decrements front_sensor_counter in the case of something been detected
  }
  */
}

void read_US() /* may want to use a digital pin*/
{
  int duration;
  pinMode(A0, OUTPUT);
  analogWrite(A0, LOW);
  delayMicroseconds(2);
  analogWrite(A0, HIGH);
  delayMicroseconds(10);
  analogWrite(A0, LOW);

  pinMode(A0, INPUT);
  duration = pulseIn(A0, HIGH);
  front_sensor = duration/58;  
}

