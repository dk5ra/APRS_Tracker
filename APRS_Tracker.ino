#include <LibAPRS.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

// gps serial TX to pin 12
SoftwareSerial gpsSerial(12, 11);

Adafruit_GPS GPS(&gpsSerial);
#define OPEN_SQUELCH false
#define ADC_REFERENCE REF_3V3
randomSeed(analogRead(A0)); //init random numbersssssssssss
int sendsecond = random(1,59);

  void cw_led_blink(int i, int j){   // cw_led_blink((short or long = 0 or 1) , (how often blink))
      if ( i == 0){ // short
        for (int k = 0; k < 1; k++){
        digitalWrite(15, HIGH);
        delay(200);
        digitalWrite(15, LOW);
      }
      }
      else {
        for (int k = 0; k < 1; k++){
        digitalWrite(15, HIGH);
        delay(350);
        digitalWrite(15, LOW);
        }
      }
      
  }
  
  void error(){
    for (int i = 0;i < 2;i++){
      cw_led_blink(0,3);
      cw_led_blink(1,3);
      cw_led_blink(0,3);
    }
  }
  
char lastsec = 0;

void setup() {

  
  
  pinMode( 2, INPUT_PULLUP );
  pinMode( 3, INPUT_PULLUP );
  pinMode( 4, INPUT_PULLUP );
  pinMode( 5, INPUT_PULLUP );
  
  pinMode( 13, OUTPUT );
  
  APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
  // define callsign
  char Callsign[] = "DK5RA";

  APRS_setPreamble(550);
  
  // "intelligent" mode selector use only 3 bit and pins(2,3,4)
  int aprsmode = 0;
  if (digitalRead(2) == LOW && digitalRead(3) == LOW && digitalRead(4) == LOW && digitalRead(5) == LOW){
    aprsmode = 0;
    }
    else if (digitalRead(2) == HIGH && digitalRead(3) == LOW && digitalRead(4) == LOW && digitalRead(5) == LOW) {
      aprsmode = 1;
    }
     else if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == LOW && digitalRead(5) == LOW) {
      aprsmode = 2;
    }
    else if (digitalRead(2) == LOW && digitalRead(3) == LOW && digitalRead(4) == HIGH && digitalRead(5) == LOW) {
      aprsmode = 3;
    }
    else if (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(4) == LOW && digitalRead(5) == LOW) {
      aprsmode = 4;
    }
    else if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == HIGH && digitalRead(5) == LOW) {
      aprsmode = 5;
    }
    else if (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(4) == HIGH && digitalRead(5) == LOW) {
      aprsmode = 6;
    }
    
  switch(aprsmode) {
      case 0: APRS_setCallsign(Callsign, 9); APRS_setSymbol('>'); break; // car portable
      case 1: APRS_setCallsign(Callsign, 7); APRS_setSymbol('['); break; // walk
      case 2: APRS_setCallsign(Callsign, 8); APRS_setSymbol('b'); break; // bycicle portable
      case 3: APRS_setCallsign(Callsign, 6); APRS_setSymbol(';'); break; // fieldday cmaping
      case 4: APRS_setCallsign(Callsign, 8); APRS_setSymbol('='); break; // train
      case 5: APRS_setCallsign(Callsign, 9); APRS_setSymbol('>'); error(); break; // not defined
      case 6: APRS_setCallsign(Callsign, 5); APRS_setSymbol('O'); break; // balloon
      default: APRS_setCallsign("ERROR", 0); APRS_setSymbol(' '); error(); break;
      
  }

  
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);

  // enable tim0 interrupt at value 0x7F
  OCR0A = 0x7F;
  TIMSK0 |= _BV(OCIE0A);
  
  delay(1000);
  gpsSerial.println(PMTK_Q_RELEASE);
}

// tim0 ISR for gps parser
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))
      return;
  }
}

void locationUpdate() {
  char lats[10],latvs[10];
  char lons[10],lonvs[10];
  
  // the only way i found to set the variables the correct way for libAPRS
  // since sprintf doesn't like float on AVRs
  dtostrf(GPS.latitude, 6, 2, latvs);
  dtostrf(GPS.longitude, 6, 2, lonvs);
  
  sprintf(lats, "%s%c", latvs, GPS.lat);
  if( GPS.longitude >= 10000 ){
    sprintf(lons, "%s%c", lonvs, GPS.lon);
  } else {
    sprintf(lons, "0%s%c", lonvs, GPS.lon);
  }
  APRS_setLat(lats);
  APRS_setLon(lons);

  char *comment = "AVR APRS Tracker";
      
 // turn off soft serial to stop interrupting tx
  gpsSerial.end();

  //set random value for sending period
  
  // TX
  APRS_sendLoc(comment, strlen(comment));
  
  // read blue TX LED pin and wait till TX has finished (PB1, Pin 1)
  while(bitRead(PORTB,1));
  
  //start soft serial again
  GPS.begin(9600);
}

void loop() {
// Debug Test 
/*
if( GPS.fix ){
 delay(10000);
 locationUpdate();
}*/
  
  if( GPS.fix ){
      if ( (( GPS.seconds == sendsecond  && ( lastsec != GPS.seconds )) )) {
      locationUpdate();
    }
  } else {
    delay(350);
  }
  lastsec = GPS.seconds;

}

// need to keep that unless i edit libAPRS (fucnt. only needed for rx, but compiler wants it)
void aprs_msg_callback(struct AX25Msg *msg) {
}
