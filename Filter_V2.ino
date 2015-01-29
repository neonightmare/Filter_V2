
// Filterautomationscode 
// Olivier Gottsponer, Arduino Uno, 7.8.2014
// V2

#include <Servo.h>  

Servo myservo;  // create servo object to control a servo  

// Pindefinitionen
int PosFeedback   = A3;  // pink-Leitung des Aktors, FeedbacPosition
int SetLinAktor   = 11; // weiss Digital Ausgang Aktror,
int DruckSensor   = 13; // Drucksensor, pullup, neg. Logik
int Ventil_L       = 6;  // Ansteuerung Ventil Phase
int Ventil_N       = 7;  // Ansteuerung Ventil Phase
int on_off        = 10; // Eingang für on_off-Schalter
int man_spuelen   = 12; // Eingang für manuellen Spülvorgang

//Variablendeklarationen

int ms_max            = 2000;   // max. Aktorposition 2000, für def. Version 1700
int ms_min            = 1000;   // min. Aktorposition 1000, für def. Version 1300
int ms_middle         = 1540;   // Mittenposition Aktor für def. Version 1500

int verz_vor          = 4000;   // verzögerung vor Servobefehl 20
int verz_nach         = 4000;   // Verzögerugn nach Servobefehl 5000
int verz_stability    = 20;     // Verzögerung loop(); Stabilität

int ifstatus          = 0;
int druckabfall_read  = LOW;
int on_off_read       = HIGH;
int man_spuelen_read  = HIGH;
int linposition       = 0;

unsigned long      lastTime     = 0;
unsigned long      elapsedTime  = 0;
unsigned long      waitTime     = 1800000; // (30 * 60 * 1000) = 30min

void setup() 
{  

  //attaches the servo on pin SetLinAktor to the servo object
  myservo.attach(SetLinAktor); // weisse-leitung des Aktors
  myservo.writeMicroseconds(ms_middle); //set initial servo position if desired

  //Ein - AUsgänge definieren 
  pinMode(Ventil_L, OUTPUT); 
  pinMode(Ventil_N, OUTPUT); 
  pinMode(DruckSensor, INPUT_PULLUP); 
  pinMode(on_off, INPUT_PULLUP); 
  pinMode(man_spuelen, INPUT_PULLUP);  

  //Ventile schliessen
  digitalWrite(Ventil_L, HIGH); // Ventil schliessen
  digitalWrite(Ventil_N, HIGH); // Ventil schliessen

  //start serial connection
  Serial.begin(9600);
  Serial.println("Beginn Serial-Session: Filter_V2"); // so I can keep track of what is loaded
} 


void loop() 
{     

  //Berechnug der proz. Position, nicht als Deklaration möglich...
  int   prozmax = (ms_max/10)-100-4;  // 96% maximum //für def. Version 66%
  int   prozmin = (ms_min/10)-100+4;  // 4% minimum  //für def. Version 34%

  //Linearposition abgfragen
  int(linposition)    = analogRead(PosFeedback) * (100 / 673.46);


  //Wartezeit bis zur nächsten Spülung
  if (elapsedTime >= waitTime) 
  {
    ifstatus = 0;
  }

  //Taster, Sensoren abfragen	
  if (ifstatus == 0)
  {
    druckabfall_read    = digitalRead(DruckSensor);
    delay(verz_stability);
  }
  on_off_read         = digitalRead(on_off);
  man_spuelen_read    = digitalRead(man_spuelen);




  if ((druckabfall_read == HIGH) && (on_off_read == LOW) && (ifstatus == 0))  // Sensor, on_off pruefen, pull-up inverte logik
  {
    digitalWrite(Ventil_L, LOW); // Ventil öffnen
    digitalWrite(Ventil_N, LOW); // Ventil öffnen
    myservo.writeMicroseconds(ms_max);
    ifstatus = 1;
  }

  else if (man_spuelen_read == LOW)  // manuell, pull-up inverte logik
  {
    digitalWrite(Ventil_L, LOW); // Ventil öffnen
    digitalWrite(Ventil_N, LOW); // Ventil öffnen
    myservo.writeMicroseconds(ms_max); 
    ifstatus = 1;
  }

  if ((linposition <=  prozmin) && (ifstatus == 1)) //MAX 1
  {
    delay(verz_vor);
    myservo.writeMicroseconds(ms_min);
    delay(verz_nach);
    ifstatus = 2;
  }

  else if ((linposition >=  prozmax) && (ifstatus == 2))//MIN 1
  { 
    delay(verz_vor);
    myservo.writeMicroseconds(ms_max);
    delay(verz_nach);
    ifstatus = 3;     
  }

  else if ((linposition <=  prozmin) && (ifstatus == 3)) //MAX 2
  { 
    delay(verz_vor);
    myservo.writeMicroseconds(ms_min);
    delay(verz_nach);
    ifstatus = 4;     
  }

  else if ((linposition >=  prozmax) && (ifstatus == 4))//MIN 2
  { 
    delay(verz_vor);
    myservo.writeMicroseconds(ms_max);
    delay(verz_nach);
    ifstatus = 5;     
  }  
  else if ((linposition <=  prozmin) && (ifstatus == 5)) //MAX 3
  { 
    delay(verz_vor);
    myservo.writeMicroseconds(ms_min);
    delay(verz_nach);
    ifstatus = 6;     
  }

  else if ((linposition >=  prozmax) && (ifstatus == 6 ))//MIN 3
  { 
    delay(verz_vor);
    myservo.writeMicroseconds(ms_max);
    delay(verz_nach);
    ifstatus = 7;     
  }  

  if (ifstatus == 7)
  {
    myservo.writeMicroseconds(ms_middle);
    digitalWrite(Ventil_L, HIGH); // Ausgang Ventil schliessen
    digitalWrite(Ventil_N, HIGH); // Ausgang Ventil schliessen
    //delay(verz_nach_spuelen);
    lastTime = millis();
    ifstatus = 8;
  }
  
  if (ifstatus == 8 )
  {
      elapsedTime = (millis() - lastTime);
  }
  delay(verz_stability);

  // Ausgabe div. Daten auf Konsole
  Serial.println("lastTime:"); // so I can keep track of what is loaded
  Serial.println(lastTime); 
  Serial.println("aktMillis"); // so I can keep track of what is loaded
  Serial.println(millis()); 
  Serial.println("Elapsedtime:"); // so I can keep track of what is loaded
  Serial.println(elapsedTime); 
  Serial.println("Status:"); // so I can keep track of what is loaded
  Serial.println(ifstatus);
  Serial.println("Linearposition:"); // so I can keep track of what is loaded
  Serial.println(int(linposition)); 
}	










