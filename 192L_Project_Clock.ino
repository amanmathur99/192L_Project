#include <Arduino.h>
#include <LiquidCrystal.h>

//Variables
void iterateTime();
void setUserAlarm();
void displayTime(int hour, int minute, int sec);
void changeRoom();
void buttonPressed();

volatile int state = LOW;
volatile int roomState = LOW;
volatile int completedCycles = LOW;


int buttonValue = analogRead(2);

int secs = 0;
int mins = 0;
int hours = 0;

int alarmMatrix[4][3];


int lcdButton = 0;
int roomNumber = 1;

bool setAlarm = 0;

//Setup
void setup() {
    Serial.begin(9600);
    
    cli(); // disable interrupts
    TCCR1A = 0; // reset Timer1 Control registers
    TCCR1B = 0; //Set WGM_2:0 = 000
    TCNT1 = 0;
    TIMSK1 = 0x6; //Enable OCR Interrupt bits
    OCR1A = 15625; // excutes interrupt when events is reached. Frequency of the arduino, adjusted by the prescalar (16 000 000 / 1024)

    // set prescaler for Timer 1 to 1024
    TCCR1B |= _BV(CS02);
    TCCR1B &= ~_BV(CS01);
    TCCR1B |= _BV(CS00);

    sei(); //enable interrupts
    
    attachInterrupt(0, buttonPressedInterruptHandler, CHANGE); // trigger interrupt when button is pressed (set alarm, hour, min, cancel)
    attachInterrupt(1, changeRoomInterruptHandler, CHANGE); // trigger interrupt when button is pressed (change room)
}

//Main
void loop() {

  iterateTime();

  //On Button Click
  if (state){
    //Serial.println("Interrupt Triggered");
    buttonPressed();
  }
  if (roomState) {
    Serial.println("Room State Triggered");
    changeRoom();
  }
  
}

void iterateTime(){
  if (completedCycles) {
    secs++;
    completedCycles = LOW;
    
  if (secs == 60) {
    mins++;
  }
  secs = secs % 60;

  if (mins == 60){
    mins = 0;
    hours++;  
  }
    displayTime(hours, mins, secs);
  }
  
}

void displayTime(int hours, int mins, int secs){
  if (hours < 10) {
    Serial.print("0");
  }
  Serial.print(hours);
  Serial.print(":");
  if (mins < 10) {
    Serial.print("0");
  }
  Serial.print(mins);
  Serial.print(":");
  if (secs < 10) {
    Serial.print("0");
  }
  Serial.println(secs);

//  for (int i = 0; i < 20; i++) {
//    Serial.println("");
//  }
}

void setUserAlarm() {
  delay(1000);
  displayTime(12, 0, 0);
}

void buttonPressedInterruptHandler(){
  state = !state;
}

ISR (TIMER1_COMPA_vect) {
  completedCycles = !completedCycles;
  TCNT1 = 0;
}

void buttonPressed(){
  
    buttonValue = analogRead(2); //Read button value (0 - 1023) 
    Serial.println(buttonValue);
    delay(250);
    
    if (buttonValue <= 50){
      Serial.println("Set Alarm");
      setAlarm = 1;
      //Set Alarm
      
      while (setAlarm) {
        buttonValue = analogRead(2);
        
        if (buttonValue > 50 && buttonValue <=150) {
          alarmHour++;
          if (alarmHour > 23) {
            alarmHour = 0;    
          }
          Serial.print(alarmHour);
          Serial.print(":");
          Serial.println(alarmMin);
          delay(500);
        }
        
        if (buttonValue > 150 && buttonValue <= 350) {
          alarmMin++;
          if (alarmMin > 59) {
            alarmMin = 0;
          }
          Serial.print(alarmHour);
          Serial.print(":");
          Serial.println(alarmMin);
          delay(500);
        }
        
        if (buttonValue <= 50) {
          Serial.println("Alarm has been set");
          setAlarm = 0;
        }

      }
    } 
    else if (buttonValue > 350 && buttonValue <= 450){
      Serial.println("Turn Off Alarm");
      //Turn off alarm
    } else if (buttonValue > 1000){
      Serial.println("False Alarm");
    }
    
    state = LOW;

}

void changeRoom() {
  //change the room
  if (roomNumber == 4){
    roomNumber = 1;
  } else {
     roomNumber++;
  }

  Serial.print("Change to room number: ");
  Serial.println(roomNumber);

  roomState = LOW;
}

void changeRoomInterruptHandler(){
  roomState = !roomState;
}

 

