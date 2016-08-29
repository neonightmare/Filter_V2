
// Filterautomationscode
// Olivier Gottsponer, Arduino Uno, 16.8.16
// V3

// Timing angepasst / 1x pro Woche & Ethernetanbindung...
/*
 Created by Rui Santos
 Visit: http://randomnerdtutorials.com for more arduino projects
        http://randomnerdtutorials.com/arduino-webserver-with-an-arduino-ethernet-shield/

//Einschalten mittels HTTP-Befehlen:
//http://192.168.1.179/?button1on

 Arduino with Ethernet Shield
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

//Ethernetserver Definitionen

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //physical mac address
byte ip[] = { 192, 168, 1, 179 };                      // ip in lan (that's what you need to use in your browser. ("192.168.1.178")
byte gateway[] = { 192, 168, 1, 1 };                   // internet access via router
byte subnet[] = { 255, 255, 255, 0 };                  //subnet mask
EthernetServer server(80);                             //server port
String readString;

// Pindefinitionen
int PosFeedback   = A3;  // pink-Leitung des Aktors, FeedbacPosition -> tuerkis Kabel -> Stecker PIN:7
int SetLinAktor   = 11; // weiss Digital Ausgang Aktror -> weiss Kabel -> Stecker PIN:5
                        // schwarz Aktor GND -> blau Kabel -> Stecker PIN:6
                        // rot Aktor +12V -> violett Kabel -> Stecker PIN:8
int DruckSensor   = 13; // Drucksensor, pullup, neg. Logik-> Stecker PIN:1 /2
int Ventil_L       = 6;  // Ansteuerung Ventil Phase-> Stecker PIN:3
int Ventil_N       = 7;  // Ansteuerung Ventil Phase-> Stecker PIN:4
int on_off        = 10; // Eingang für on_off-Schalter
int man_spuelen   = 12; // Eingang für manuellen Spülvorgang
String  spuelungAktiv;

//Variablendeklarationen

int ms_max            = 2000;   // max. Aktorposition 2000, für def. Version 1700
int ms_min            = 1000;   // min. Aktorposition 1000, für def. Version 1300
int ms_middle         = 1540;   // Mittenposition Aktor für def. Version 1500

int verz_vor          = 3000;   // verzögerung vor Servobefehl 20
int verz_nach         = 2000;   // Verzögerugn nach Servobefehl 5000
int verz_stability    = 5;     // Verzögerung loop(); Stabilität

int ifstatus          = 0;
int druckabfall_read  = LOW;
int on_off_read       = HIGH;
int man_spuelen_read  = HIGH;
int linposition       = 0;
bool  linposStateMin     = false;
bool  linposStateMax     = false;

unsigned long      lastTime     = 0;
unsigned long      elapsedTime  = 0;
unsigned long      waitTime     = 604800000; //1000*60*60*24*7  -> 1x pro Woche

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

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  //start serial connection
  Serial.begin(9600);
  Serial.println("Beginn Serial-Session: Filter_V3"); // so I can keep track of what is loaded
}


void loop()
{

  //Berechnug der proz. Position, nicht als Deklaration möglich...
  int   prozmax = (ms_max/10)-100-4;  // 96% maximum //für def. Version 66%
  int   prozmin = (ms_min/10)-100+4;  // 4% minimum  //für def. Version 34%

  //Linearposition abgfragen
  int(linposition)    = analogRead(PosFeedback) * (100 / 673.46);

  //Taster abfragen
  on_off_read         = digitalRead(on_off);
  man_spuelen_read    = digitalRead(man_spuelen);
  druckabfall_read    = digitalRead(DruckSensor);

  //Wartezeit bis zur nächsten Spülung
  if (elapsedTime >= waitTime)
  { ifstatus = 1;}

  if ((druckabfall_read == HIGH) && (on_off_read == LOW))  // Sensor, on_off pruefen, pull-up inverte logik
    { ifstatus = 1;};

  if (man_spuelen_read == LOW)  // manuell, pull-up inverte logik
    { ifstatus = 1;};

  if (linposition <=  prozmin)
    { linposStateMin = true;}; // minimum Position

  if (linposition >=  prozmax)
    { linposStateMax = true;}; //maximum Position

  switch (ifstatus) {
        case 1 :  {
                  spuelungAktiv = 'Spülvorgang läuft';
                  digitalWrite(Ventil_L, LOW); // Ventil öffnen
                  digitalWrite(Ventil_N, LOW); // Ventil öffnen
                  myservo.writeMicroseconds(ms_max);
                  delay(verz_nach);
                  if (linposStateMax) {ifstatus = 2;};
                };break;
        case 2 : {
                  spuelungAktiv =' Spülung Minimum 1';
                  myservo.writeMicroseconds(ms_min);
                  delay(verz_nach);
                  if (linposStateMin) {ifstatus = 3;};
                };break;
        case 3 : {
                  spuelungAktiv = 'Spülung Maximum 2';
                  myservo.writeMicroseconds(ms_max);
                  delay(verz_nach);
                  if (linposStateMax) {ifstatus = 4;};
                };break;
        case 4 : {
                  spuelungAktiv = 'Spülung Minimum 2';
                  myservo.writeMicroseconds(ms_min);
                  delay(verz_nach);
                  if (linposStateMin) {ifstatus = 5;};
                };break;
        case 5 : {
                  spuelungAktiv = 'Spülung Maximum 3';
                  myservo.writeMicroseconds(ms_max);
                  delay(verz_nach);
                  if (linposStateMax) {ifstatus = 6;};
                };break;
        case 6 : {
                  spuelungAktiv = 'Spülung Minimum 3';
                  myservo.writeMicroseconds(ms_min);
                  delay(verz_nach);
                  if (linposStateMin) {ifstatus = 7;};
                };break;

        case 7 : {
                  spuelungAktiv = 'Spülung StandBy';
                  myservo.writeMicroseconds(ms_middle); //set initial servo position if desired
                  digitalWrite(Ventil_L, HIGH); // Ausgang Ventil schliessen
                  digitalWrite(Ventil_N, HIGH); // Ausgang Ventil schliessen
                  lastTime = millis();
                  ifstatus = 0;
                };break;
        default:break;
      }

      elapsedTime = (millis() - lastTime);
      delay(verz_stability);

//////// Server - Funktionalität /////////////

// Create a client connection
  EthernetClient client = server.available();
    if (client) {
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();

          //read char by char HTTP request
          if (readString.length() < 100) {
            //store characters to string
            readString += c;
            //Serial.print(c);
           }

           //if HTTP request has ended
           if (c == '\n') {
             Serial.println(readString); //print to serial monitor for debuging

             client.println("HTTP/1.1 200 OK"); //send new page
             client.println("Content-Type: text/html");
             client.println();
             client.println("<HTML>");
             client.println("<HEAD>");
             client.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
             client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
             client.println("<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />");
             client.println("<TITLE>Arduino Fernsteuerung</TITLE>");
             client.println("</HEAD>");
             client.println("<BODY>");
             client.println("<H1>Relay Switch  Webserver - Filterspuelung</H1>");
             client.println("<hr />");
             client.println("<br />");
             client.println("<H2>Arduino with Ethernet Shield</H2>");
             client.println("<br />");
             client.println("<a href=\"/?button1on\"\">Turn On Filter </a>");
             client.println("<br />");
             client.println("<br />");
             // print the current readings, in HTML format:
             client.print("Status Spülung: ");
             client.print(spuelungAktiv);
             client.println("<br />");
             client.println("<br />");
             client.print("Position Spülung: ");
             client.print(linposition);
             client.print("%");
             client.println("<br />");
             client.println("<br />");

             client.println("<p>modified by neonightmre</p>");
             client.println("<br />");
             client.println("</BODY>");
             client.println("</HTML>");

             delay(1);
             //stopping client
             client.stop();
             //controls the Arduino if you press the buttons
             if (readString.indexOf("?button1on") >0){
               ifstatus = 0;
             }
              readString="";

           }
         }
      }
  }

  ////////// Ausgabe div. Daten auf Konsole ////////////////
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
