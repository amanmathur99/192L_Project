#include <Arduino.h>
#include <LiquidCrystal.h>

//Function declarations
void iterateTime();
void setUserAlarm();
void displayTime(int hour, int minute, int sec);
void changeRoom();
void buttonPressed();

//Volatile vars for interrupts
volatile int state = LOW;
volatile int roomState = LOW;
volatile int completedCycles = LOW;


int buttonValue = analogRead(2);

//Ints used in program
int secs = 0;
int mins = 0;
int hours = 0;
int alarmMatrix[4][2];
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
    attachInterrupt(1, changeRoomInterruptHandler, RISING); // trigger interrupt when button is pressed (change room)
}

//Main
void loop() {

  iterateTime();

  //On Button Click
  if (state){
    buttonPressed();
  }

  //On change room button click
  if (roomState) {
    changeRoom();
  }

//  for ( int i = 0; i < 3; i++){
//    if (checkAlarm(i)){
//      Serial.println("Light turns on");
//    }
//  }
  
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


bool checkAlarm(int roomNumber){ 
  
  if (hours == alarmMatrix[roomNumber][0] && mins == alarmMatrix[roomNumber][1]){
    return true;
  } else{
    return false;
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

}

void setUserAlarm() {
  displayTime(12, 0, 0);
}


void buttonPressed(){
  
    buttonValue = analogRead(2); //Read button value (0 - 1023) 
    
    if (buttonValue <= 50){
      delay(500);
      Serial.println("Set Alarm");
      setAlarm = 1;
      //Set Alarm
      
      while (setAlarm) {
        buttonValue = analogRead(2);
        
        if (buttonValue > 50 && buttonValue <=150) { //Increase hour 
          delay(200);
          alarmMatrix[roomNumber][0]++;
          if (alarmMatrix[roomNumber][0] > 23) {
            alarmMatrix[roomNumber][0] = 0;    
          }
          Serial.print(alarmMatrix[roomNumber][0]);
          Serial.print(":");
          Serial.println(alarmMatrix[roomNumber][1]);
        }
        
        if (buttonValue > 150 && buttonValue <= 350) { //Increase minute
          delay(200);
          alarmMatrix[roomNumber][1]++;;
          if (alarmMatrix[roomNumber][1] > 59) {
            alarmMatrix[roomNumber][1] = 0;
          }
          Serial.print(alarmMatrix[roomNumber][0]);
          Serial.print(":");
          Serial.println(alarmMatrix[roomNumber][1]);
          delay(500);
        }
        
        if (buttonValue <= 50) {
          delay(200);
          Serial.println("Alarm has been set to: ");
          Serial.print(alarmMatrix[roomNumber][0]);
          Serial.print(" : ");
          Serial.print(alarmMatrix[roomNumber][1]);
          setAlarm = 0;
        }

      }
    } 
    else if (buttonValue > 350 && buttonValue <= 450){
      delay(200);
      Serial.println("Turn Off Alarm");
      //Turn off alarm
    } else if (buttonValue > 1000){
      Serial.println("False Alarm");
    }
    
    state = LOW;

}

void changeRoom() {
  delay(100);
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

//Interrupt Handlers
void changeRoomInterruptHandler(){
  roomState = !roomState;
}


void buttonPressedInterruptHandler(){
  state = !state;
}

ISR (TIMER1_COMPA_vect) {
  completedCycles = !completedCycles;
  TCNT1 = 0;
}

 

