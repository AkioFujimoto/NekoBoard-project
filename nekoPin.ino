/* The revised edition of NekoBoard2 machine with pin monitor.
 // contains source of NekoBoard2.
 // (c) 2014 A.Fujimoto
 // twitter: @shiva5300
 */

// define abbrev.
#define kprint Serial.print
#define kprintln Serial.println

// declare variables.
int i;
int val;
int str; // via serial debugging.
int debugTime = 500;  // for debugging use, wait time.

/* Inportant! UNSTABLE inprementation.*/
// added on 20140928
const int aPin = 0; // analogRead pin definition.
const int motorAdir = 12; // motor A direction.
const int motorApwm = 3; // motor A speed.
const int brakeA = 9; // brake A.
const int motorBdir = 13; // motor B direction.
const int motorBpwm = 11; // motor B speed.
const int brakeB = 8; // brake B.
const int csA = A0; // current sense for motor channel A.
const int csB = A1; // current sense for motor channel B.
const int water_sense = 2;  // raise to HIGH when water is empty.
// from motorABtest-kai.

int outPin[] = {
  0, 1, 2, 3, 8, 9, 11, 12, 13};  // Pin#2 needs to be considered.
/*
  3:PWM(A), 9:Brake(A), 12:Dir(A).
 12:PWM(B), 8:Brake(B), 13:Dir(B).
 Available pins: (2), 4, 5, 6, 7, 10.
 */
int outLen = sizeof(outPin) / sizeof(outPin[0]);
int counter = 0;
int pinState[14];
int boardState;
int initTime = 400;   // margin time for first-run initialization.
unsigned long int cTime;  // temporary variable for time storing.
boolean firstRun = true;
boolean isDebug = true;
int monitorPin = 7;
char output[3];


void setup() {
  // put your setup code here, to run once:
  for(int i=0;i<=13;i++){
    pinState[i] = 0;
  }
  /*
  system("touch ~/status.txt");  // status file.
   system("echo '000' > status.txt"); // zero fill status file.
   */
  pinState[10] = 0;  // omajinai.
  for(int j = 0;j<=13;j++){
    for(int k = 0; k<=outLen; k++){
      if(j != outPin[k]){  // if the pin is not in outPin list...
        pinMode(j, INPUT);  // set pin as INPUT.
        //input pullup
        digitalWrite(j, HIGH);
      }
      else{  // otherwise, set pin as OUTPUT.
        pinMode(j, OUTPUT);
        pinState[j] = 1;
      }
    }
  }  // input Pin initialize end.

  /*  Added on 20140928  */
  // OUTPUT pin declaration begins.
  pinMode(motorAdir, OUTPUT); //Initiates Motor Channel A pin
  pinMode(brakeA, OUTPUT); //Initiates Brake Channel A pin
  pinMode(motorBdir, OUTPUT); // initialize motor channel B.
  pinMode(brakeB, OUTPUT); // initialize motor channel B.
  pinMode(water_sense, INPUT);
  //  digitalWrite(water_sense, HIGH);  // activate input pullup resistor.
  // OUTPUT pin declaration ends.


  Serial.begin(9600);
  //  Serial1.begin(115200);
}  // end setup.

void loop() {
  /*
  FILE *fp;
   fp = fopen("status.txt", "r");
   fgets(output, 2, fp);
   fclose(fp);
   kprintln();
   kprintln(output);
   delay(200);
   */

  //  Serial1.println("loop");
  cTime = millis();  // get current time.
  // put your main code here, to run repeatedly: 
  if(firstRun == true){  // This will runs in first time only.
    boardState = 1;  // initialization of all GPIO pins.
    while(boardState == 1){
      boardState = initialize(initTime);
      if (boardState == 0)break;
    }
  }
  kprintln("----------Init OK.----------");
  firstRun = false;

  // Smell checker.
  if(readOdor() == 1){  // readOdor() returns 1 when smelled.
    talk();  // voidMethod, drive speaker board via sending signal.
  }
  kprint(readOdor());  // debug print.
  kprintln("Smell Checked.");

  // Water monitor.
  if(readWater() == 1){  // readWater() returns 1 when there is no water.
    motor();  // voidMethod, supply water with motor shield.
  }
  kprint(readWater());  // debug print.
  kprintln("                Water Checked.");
  kprint("took ");
  kprint(millis() - cTime);
  kprintln(" ms for 1 iteration.");
  kprintln(digitalRead(7));  // debug print.
  if(Serial.available()!=0){
    readSerial();
  }
  delay(debugTime);

}  // end loop.

// subroutines.
int initialize(int x){
  for(counter = 0; counter <= x; counter++){
    for(int i = 0; i<=13;i++){
      if(pinState[i] == 0){
        val = digitalRead(i);
        kprint(val);
      }
      else if(pinState[i] == 1){
        kprint("*");
      }
      if (i==13)break;
      kprint(",");
      if(i%7 == 0 && i != 0){
        kprint("   ");
      }
    }
    kprint(". | ");
    for(i = 0;i<=5;i++){
      kprint(analogRead(i));
      kprint(",");
      if (i == 5) break;
    }
    kprintln(".");
    kprintln(outLen);
    kprint("Please wait... ");
    kprintln(initTime - counter);
    //    delay(100);
    //    counter++;
  }
  return 0;
}

int readOdor(){
  if(~isDebug){
    // no smell code
    return 0;
  }
  else{
    // smell code
    if(isDebug)return 1;
  }  
}

int readWater(){
  if(~isDebug){
    // water is OK
    return 0;
  }
  else{
    // water is empty
    if(isDebug)return 1;
  }

}

void talk(){
  atpOn();  // invoke talking board.
}

void motor(){
  motorDebugB();
}


void atpOn(){
}

void motorDebugB(){
  // motor A is feeding.
  // motor B is watering.
  digitalWrite(brakeA, HIGH);
  digitalWrite(brakeB, HIGH);
  digitalWrite(brakeA, LOW);
  digitalWrite(brakeB, LOW);

  if(digitalRead(water_sense) == HIGH){
    Serial.println(analogShow(aPin));
    //forward @ full speed
    digitalWrite(motorBdir,HIGH);  // drive this motor channel!
    digitalWrite(brakeB, LOW);
    //    digitalWrite(motorAdir, HIGH); //Establishes forward direction of Channel A
    digitalWrite(brakeA, LOW);   //Disengage the Brake for Channel A
    //    digitalWrite(motorApwm, 255);   //Spins the motor on Channel A at full speed
    digitalWrite(motorBpwm,255);  // spins the motor on channel B at half speed.
    delay(300);  // set apropriate time for winding up an water tank.
    Serial.println(analogShow(aPin));
    //backward @ full speed
    digitalWrite(motorBdir,LOW);
    digitalWrite(brakeB, LOW);
    //    digitalWrite(motorAdir, LOW); //Establishes forward direction of Channel A
    digitalWrite(brakeA, LOW);   //Disengage the Brake for Channel A
    //    digitalWrite(motorApwm, 255);   //Spins the motor on Channel A at full speed
    digitalWrite(motorBpwm,255);  // spins the motor on channel B at half speed.
    delay(300);
    Serial.println(analogShow(aPin));

    //    digitalWrite(brakeA, HIGH); //Eengage the Brake for Channel A
    digitalWrite(brakeB, HIGH); //engage the brake for channel B
    delay(1000);
    digitalWrite(brakeA, LOW); // Disable the brake for Channel A
    digitalWrite(brakeB, LOW);
    Serial.println(analogShow(aPin));
  }
}
int analogShow(int x){
  return analogRead(x);
}

void readSerial(){
  str = Serial.read();
  kprint(str);
  delay(1000);

  if(str == 114 /* atoi("r") */ ){  // "r"
    kprintln("read");
    //    kprintln(atoi(str));
    delay(5000);
    boardState = initialize(10);
    delay(5000);

  }

  //  if(str != '\0' ){
  //    kprintln(str);
  //    switch(str){
  //    case 'r':
  //      // read ports.
  //      break;
  //
  //    default:
  //    }
  //    delay(3000);
  //  }  
  //  Serial.flush();

}





