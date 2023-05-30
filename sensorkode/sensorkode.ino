//DISPLAY
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <String.h>
#define PIN_CLK  7
#define PIN_CE 10
#define PIN_DIN 8
#define PIN_DC   9
#define PIN_RST  11

// Adafruit_PCD8544(CLK,DIN,D/C,CE,RST);
Adafruit_PCD8544 display = Adafruit_PCD8544(PIN_CLK, PIN_DIN, PIN_DC, PIN_CE, PIN_RST);

//Shift register 1 pins
int LATCH_PIN = 5;      // Latch pin of 74HC595 er koblet til digital pinne 5
int CLOCK_PIN = 6;      // Clock pin of 74HC595 er koblet til digital pinne 6
int DATA_PIN = 4;      // Data pin of 74HC595 er koblet til digital pinne 4

//hvilke lys som er på og hvilke av, hver bit representerer en output-pin i shiftregisteret
byte register1 = 00000000;
//array over alle shiftregisterene i bruk
byte registre[] = {register1};
int registerIndeks = 0;
int antRegistre = 1;

int sensorInn = 13; // Pir bevegelsessensor koblet til pinne 13 
int sensorUt = 12;
int ledIndeks = 0;
int antallNaa = 0;
int innMillis = 0;
int utMillis = 0;
boolean sensorInnTrigget = false;
boolean sensor2Trigget = false;

int totaltAntBesokende = 0;

void setup() {

//shift reg nr 1
pinMode(LATCH_PIN, OUTPUT);
pinMode(DATA_PIN, OUTPUT);  
pinMode(CLOCK_PIN, OUTPUT);

  //Oppsett av sensorene
  pinMode(sensorInn, INPUT);
  pinMode(sensorUt, INPUT);
  Serial.begin(9600);
  Serial.print("Program start");

	display.begin();
	display.setContrast(57); // 57 setter sterkest farge

  updateShiftRegister();

  delay(10000);
  Serial.println("FERDIG MED KALIBRERING");
}

void loop() {
  
  delay(200);
  sjekkSensorer(); // Sjekker om baade sensor 1 eller sensor 2 har blitt aktivert
  settAntallLys(); // Hvis begge sensorene har blitt aktivert oeker eller reduseres antall lys i kongehuset
  
  Serial.println(antallNaa);
  Serial.println("totalt: ");
  Serial.println(totaltAntBesokende);
  Serial.println("Naa: ");

  visStatus(); // Skriver ut [naavaerende antall] / [totalt antall] besokende paa skjermen
}

void sjekkSensorer(){
  // Setter tilsvarende booleans lik true dersom sensoren er trigget
  // innMillis registrerer naar den ble trigget for aa vite hvilken som ble trigget forst

  if (digitalRead(sensorInn) == HIGH && !sensorInnTrigget) { 
       innMillis = millis();
       sensorInnTrigget = true;    
      Serial.println("Motion Detected led 1");
  } 
  if (digitalRead(sensorInn) == LOW) {
    sensorInnTrigget = false;
    innMillis = 0;
  }


  if(digitalRead(sensorUt) == HIGH && !sensor2Trigget) {   
       utMillis = millis();
       sensor2Trigget = true; 
    Serial.println("Motion Detected led 2");
  } 
  if (digitalRead(sensorUt) == LOW) {
    sensor2Trigget = false;
    utMillis = 0;
  }  
}

void settAntallLys(){
  // Dersom begge senorene har blitt trigget sjekkes det hvilken som ble trigget forst
  // dette viser om personen kom eller dro

  if (sensorInnTrigget && sensor2Trigget) {
    //dersom innMillis < utMillis har noen kommet inn. Gaar bare inn i kodeblokk om ikke alle lysene allerede er paa
    if(innMillis < utMillis && (registerIndeks < antRegistre && ledIndeks <= 7)) {
      Serial.println("SKAL LEGGE TIL LYS");
      bitSet(registre[registerIndeks], ledIndeks);
      updateShiftRegister();
      ledIndeks++;
      antallNaa++;
      totaltAntBesokende++;
      //når man når det åttende lyset på et shiftregister går man videre til neste shiftregister sitt forste lys
      if (ledIndeks > 7) {
        if (registerIndeks < antRegistre - 1) {
          ledIndeks = 0;
          registerIndeks++;
        }
      } 
      
      
    }

    // Dersom innMillis > utMillis har noen gått ut. Gaar bare inn i kodeblokk om man ikke er paa register 0 og indeks 0 samtidig
    else if (innMillis > utMillis && (registerIndeks > 0 || ledIndeks > 0)){
      if(ledIndeks == 0 && registerIndeks > 0) {
        ledIndeks = 7;
        //biten (i byten til det gjeldende shiftregistret) som styrer det sist tente lyset settes til 0
        bitClear(registre[--registerIndeks], ledIndeks);
        updateShiftRegister();
        
      }
      else{
          bitClear(registre[registerIndeks], --ledIndeks);
          updateShiftRegister();
          antallNaa--;
      }

      
    }

    ventSkjerm(); // Loading-screen for skjermen mens vi venter paa sensorenes cooldown
    
  }
}





void ventSkjerm() {
  //skriver 'wait...' på skjermen med en dynamisk skrift, så lenge sensorene er i cooldown-modus
  String wait[] = {"Wait", "Wait.", "Wait..", "Wait..."};
  int waitledIndeks = 0;
  while(digitalRead(sensorUt) == HIGH || digitalRead(sensorInn) == HIGH){
    
      delay(200);
      display.clearDisplay();
      display.setRotation(2);
      display.setTextSize(2);
      display.setTextColor(BLACK);
      display.setCursor(0,20);
      display.println(wait[waitledIndeks]);
      waitledIndeks++;
      if(waitledIndeks > 3){
        waitledIndeks = 0;
      }
      display.display();
      Serial.println("Venter paa at begge sensorene blir inaktive ...");

      //nullstiller alle gjeldende variabler og fortsetter videre når begge sensorene er ferdige med cooldown
      if(digitalRead(sensorUt) == LOW && digitalRead(sensorInn) == LOW){
        sensorInnTrigget = false;
        innMillis = 0;
        sensor2Trigget = false;
        utMillis = 0;
        break;
      }
    }
}

//faar de gjeldende tallene for navarende besøkende og totalt antall til å vises på skjermen
void visStatus() {
  delay(200);
  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(4);
	display.setTextColor(BLACK);
	display.setCursor(0,10);
  display.print(String(antallNaa) + "/" + String(totaltAntBesokende));
  display.display();
}


  /*
 * updateShiftRegister() - Denne funksjonen setter latchPin til lav, 
   deretter kaller den Arduino-funksjonen 'shiftOut' for å skifte ut innholdet i variabelen 'leds' i skiftregisteret 
   før den setter 'latchPin' til høy igjen.

   Akkurat denne metoden er tatt fra arduino.cc, tilpasset våre behov blant annet ved å endre argumentene fra LSBFIRST til MSBFIRST
 */
  
  void updateShiftRegister()
{
   digitalWrite(LATCH_PIN, LOW);
   shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, registre[registerIndeks]);
   digitalWrite(LATCH_PIN, HIGH);
}