//DISPLAY
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <String.h>
#define PIN_CLK  2
#define PIN_CE 5
#define PIN_DIN 3
#define PIN_DC   4
#define PIN_RST  6

// Adafruit_PCD8544(CLK,DIN,D/C,CE,RST);
Adafruit_PCD8544 display = Adafruit_PCD8544(PIN_CLK, PIN_DIN, PIN_DC, PIN_CE, PIN_RST);

//Shift register 1 pins
int LATCH_PIN = 5;      // Latch pin of 74HC595 is connected to Digital pin 5
int CLOCK_PIN = 6;      // Clock pin of 74HC595 is connected to Digital pin 6
int DATA_PIN = 4;       // Data pin of 74HC595 is connected to Digital pin 4

//hvilke lys som er på og hvilke av
byte register1 = 00000000; // Variable to hold the pattern of which LEDs are currently turned on or off
byte registre[] = {register1, };
int registerIndeks = 0;
int antRegistre = 3;

int sensorInn = 11; // Pin connected to the motion sensor
int sensorUt = 10;
int led1 = 12;   // Pin connected to the LED
int led2 = 13;
int myLeds[] = {led1, led2};
int len = sizeof(myLeds) / sizeof(myLeds[0]);
int ledIndeks = 0;
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
  //Oppsett av LED-lys
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Serial.begin(9600);
  Serial.print("Program start");

	display.begin();
	display.setContrast(57); // 57 setter sterkest farge
}

void loop() {
  
  sjekkSensorer(); // Sjekker om baade sensor 1 eller sensor 2 har blitt aktivert
  settAntallLys(); // Hvis begge sensorene har blitt aktivert oeker eller reduseres antall lys i kongehuset
  
  Serial.println(ledIndeks);
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

  if (sensorInnTrigget && sensor2Trigget){


    if(innMillis < utMillis && (registerIndeks < antRegistre || ledIndeks < 7)) {
      bitSet(registre[registerIndeks], ledIndeks);
      updateShiftRegister();
      ledIndeks++;
      totaltAntBesokende ++;
      //Invariant: helt til slutt når siste lys er skrudd på, er ledIndeks 0 og registerIndeks 4;
      if (ledIndeks > 7) {
        ledIndeks = 0;
        registerIndeks++;
        if (registerIndeks > antRegistre) {
            registerIndeks--;
        }
      } 
      
    }

    // Gaar bare inn i kodeblokk om man ikke er paa register 0 og indeks 0 samtidig
    else if (innMillis > utMillis && (registerIndeks > 0 || ledIndeks > 0)){
      if(ledIndeks == 0 && registerIndeks > 0) {
        ledIndeks = 7;
        bitClear(registre[--registerIndeks], ledIndeks);
        updateShiftRegister();
        
      }
      else{
          bitClear(registre[registerIndeks], ledIndeks);
          updateShiftRegister();
          ledIndeks--;
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

void visStatus() {
  delay(200);
  display.clearDisplay();
  display.setTextSize(4);
	display.setTextColor(BLACK);
	display.setCursor(0,10);
  display.print(String(ledIndeks) + "/" + String(totaltAntBesokende));
  display.display();
}

  /*
 * updateShiftRegister() - This function sets the latchPin 
 to low, then calls the Arduino function 'shiftOut' to shift out contents of variable 'leds' 
 in the shift register before putting the 'latchPin' high again.
 */
  
  void updateShiftRegister()
{
   digitalWrite(LATCH_PIN, LOW);
   shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, registre[registerIndeks]);
   digitalWrite(LATCH_PIN, HIGH);
}