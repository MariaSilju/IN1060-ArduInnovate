//av og på knapp funksjon
volatile boolean skruddPaa = true;
int PAA_KNAPP = 8;

//INPUT FRA BEVEGELSESSENSORER

//for å teste: noenKom er true, fjern else i else if og fjern kommetnar

boolean noenKom = false;
boolean noenDro = false;

boolean sensorInnTrigget = false;
boolean sensorUtTrigget = false;


int LATCH_PIN = 5;      // Latch pin of 74HC595 is connected to Digital pin 5
int CLOCK_PIN = 6;      // Clock pin of 74HC595 is connected to Digital pin 6
int DATA_PIN = 4;       // Data pin of 74HC595 is connected to Digital pin 4
  
byte leds = 0; // Variable to hold the pattern of which LEDs are currently turned on or off
int ledIndeks = 0;

// Sensorenes signalnoder
int SENSOR_SIGNAL_INN = 12;
int SENSOR_SIGNAL_UT = 11;


int signalInnMillis = 0;
int signalUtMillis = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  
  // Set all the pins of 74HC595 as OUTPUT
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);  
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(PAA_KNAPP, INPUT_PULLUP);
  
  // Signal noder til sensorene
  pinMode(SENSOR_SIGNAL_INN, INPUT);
  pinMode(SENSOR_SIGNAL_UT, INPUT);
  
  
  // Legger til en interrupt som skal kjøres når knappen trykkes
  attachInterrupt(digitalPinToInterrupt(PAA_KNAPP), skruAv, FALLING); 
  
}

void loop() {
  
  if(digitalRead(SENSOR_SIGNAL_INN) == HIGH){
    signalInnMillis = millis();
  	sensorInnTrigget = true;
  }
  if(digitalRead(SENSOR_SIGNAL_UT) == HIGH){
    signalUtMillis = millis();
    sensorUtTrigget = true;
  }
  
  //forutsetter at sensorene ikke trigges samtidig, om de gjør settes det til noenDro
  if ((sensorInnTrigget && sensorUtTrigget) && (signalInnMillis - signalUtMillis <= abs(4000))) {
    if (signalInnMillis < signalUtMillis) {
    	noenKom = true;
     	signalInnMillis = 0;
    	signalUtMillis = 0;
    }
    else {
    	noenDro = true;
        signalInnMillis = 0;
        signalUtMillis = 0;
    }
    
  }

  updateShiftRegister();
  delay(500);

  //TODO:   Sørge for at dette ikke kan skje samtidig, fordi de aksesserer en felles variabel som heter ledIndeks!

  if (noenKom) {
    skruPaaEtLys();
  }
  if (noenDro) {
    skruAvEtLys();
  }
  
  
  
}



 void skruPaaEtLys() {
   //setter bit'en som representerer det neste lyset til 1
    bitSet(leds, ledIndeks);

    //oppdaterer arduino-brettet med den nye verdien til byten
    updateShiftRegister();
    delay(1000);

    //flytter indeksen til det neste lyset som skal skrus på neste gang noen kommer
    //TODO: Legge inn at dersom denne indeksen er større enn 6/7, dvs alle lysene på dette shift registeret er påskrudd, gå videre til neste shift reg.
    ledIndeks++;
    noenKom = false;
    sensorInnTrigget = false;
 }

 void skruAvEtLys() {
   //bare dersom minst ett lys allerede er tent
    if (ledIndeks > 0) {
    bitClear(leds, ledIndeks-1);

    //oppdaterer arduino-brettet med den nye verdien til byten
    updateShiftRegister();
    delay(1000);

    //setter indeksen riktig etter at et lys slukket
    ledIndeks--;

    //setter skruAvEtLys til false, slik at dette kun skjer en gang per forespørsel fra bevegelsessensorene
    noenDro = false;
    sensorUtTrigget = false;
    }
 }


  /*
 * updateShiftRegister() - This function sets the latchPin 
 to low, then calls the Arduino function 'shiftOut' to shift out contents of variable 'leds' 
 in the shift register before putting the 'latchPin' high again.
 */
  
  
  void updateShiftRegister()
{
   digitalWrite(LATCH_PIN, LOW);
   shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, leds);
   digitalWrite(LATCH_PIN, HIGH);
}



//skrur av alt
void skruAv() {
  skruddPaa = false;
  detachInterrupt(digitalPinToInterrupt(PAA_KNAPP)); // Fjerner interrupten for knappen
  // Eventuelt kan du legge til kode her for å slå av eventuelle tilkoblede enheter
}
