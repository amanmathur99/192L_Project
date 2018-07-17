#include <Arduino.h>

//Function declarations
void iterateTime();
void setUserAlarm();
void displayTime(int hour, int minute, int sec);
void changeRoom();
void buttonPressed();

//Volatile vars for interrupts
volatile int state = LOW;
volatile int changeRoomState = LOW;
volatile int completedCycles = LOW;


int buttonValue = analogRead(2);

//Ints used in program
volatile int secs = 0;
int mins = 30;
int hours = 12;
int alarmMatrix[4][2];
int lcdButton = 0;
int roomNumber = 0;
bool setAlarm = 0;
int lightAmount = 0;

const int room0 = 8;
const int room1 = 9;
const int room2 = 10;
const int room3 = 11;

bool roomState[4];
//Setup
void setup() {
  Serial.begin(9600);

  cli(); // disable interrupts
  TCCR1A = 0; // reset Timer1 Control registers
  TCCR1B = 0; //Set WGM_2:0 = 000
  TCNT1 = 0;
  TIMSK1 = 0x6; //Enable OCR Interrupt bits
  OCR1A = 8000; // excutes interrupt when events is reached. Frequency of the arduino, adjusted by the prescalar (16 000 000 / 1024) 15625

  // set prescaler for Timer 1 to 1024
  TCCR1B |= _BV(CS02);
  TCCR1B &= ~_BV(CS01);
  TCCR1B |= _BV(CS00);

  sei(); //enable interrupts

  attachInterrupt(0, buttonPressedInterruptHandler, CHANGE); // trigger interrupt when button is pressed (set alarm, hour, min, cancel)
  attachInterrupt(1, changeRoomInterruptHandler, RISING); // trigger interrupt when button is pressed (change room)
  pinMode(room0, OUTPUT);
  pinMode(room1, OUTPUT);
  pinMode(room2, OUTPUT);
  pinMode(room3, OUTPUT);

}

//Main
void loop() {

  iterateTime();

  //On Button Click
  if (state) {
    buttonPressed();
  }

  //On change room button click
  if (changeRoomState) {
    changeRoom();
  }

  for ( int i = 0; i <= 3; i++) {
    if (checkAlarm(i)) {
      roomState[i] = true;
    }
  }

  lightAmount = analogRead(3);
  if (lightAmount > 500) {
    for ( int i = 0; i <= 3; i++) {
      roomState[i] = true;
    }
  } 
  
  for (int i = 0; i < 4; i++) {
    if (roomState[i]){
       digitalWrite(i + 8, HIGH);
    } else {
      digitalWrite(i+8, LOW);
    }
  }
}

bool checkAlarm(int roomNumber) {

  if (hours == alarmMatrix[roomNumber][0] && mins == alarmMatrix[roomNumber][1]) {
    return true;
  } else {
    return false;
  }

}

void iterateTime() {
  if (completedCycles) {

    if (secs >= 60) {
      mins++;
    }

    secs = secs % 60;

    if (mins == 60) {
      mins = 0;
      hours++;
    }

    displayTime(hours, mins, secs);
    completedCycles = LOW;
  }

}




void displayTime(int hours, int mins, int secs) {
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


void buttonPressed() {

  buttonValue = analogRead(2); //Read button value (0 - 1023)

  if (buttonValue <= 50) {
    delay(500);
    Serial.println("Set Alarm");
    setAlarm = 1;
    //Set Alarm

    while (setAlarm) {
      buttonValue = analogRead(2);

      if (buttonValue > 50 && buttonValue <= 150) { //Increase hour
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
  else if (buttonValue > 350 && buttonValue <= 450) {
    delay(200);
    //Turn off alarm
    Serial.println("Turn off Alarm");
    roomState[roomNumber] = false;
  } else if (buttonValue > 1000) {
    Serial.println("False Alarm");
  }

  state = LOW;

}

void changeRoom() {
  delay(100);
  //change the room
  if (roomNumber == 4) {
    roomNumber = 1;
  } else {
    roomNumber++;
  }

  Serial.print("Change to room number: ");
  Serial.println(roomNumber);
  changeRoomState = LOW;

}

//Interrupt Handlers
void changeRoomInterruptHandler() {
  changeRoomState = !changeRoomState;
}


void buttonPressedInterruptHandler() {
  state = !state;
}

ISR (TIMER1_COMPA_vect) {
  completedCycles = !completedCycles;
  secs++;
  TCNT1 = 0;
}



