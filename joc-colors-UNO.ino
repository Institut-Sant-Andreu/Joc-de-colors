#include <EducationShield.h>
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>

// llibreries per a la generació de la llavor dels nombres aleatoris
#include <SPI.h>
#include <SD.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>
volatile uint32_t seed;
volatile int8_t nrot;

Button botoStart = Button(9);
Melody me = Melody(8);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // SDA A4, SCL A5

// LEDs
#define pinLedsJoc 2
#define pinLedsJugador1 3
#define pinLedsJugador2 4
#define NUMPIXELS 1 // número de leds a cada "tira"
Adafruit_NeoPixel ledsJoc = Adafruit_NeoPixel(NUMPIXELS, pinLedsJoc, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledsJugador1 = Adafruit_NeoPixel(NUMPIXELS, pinLedsJugador1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledsJugador2 = Adafruit_NeoPixel(NUMPIXELS, pinLedsJugador2, NEO_GRB + NEO_KHZ800);

byte lockedComponent, lockedValue, Rjoc, Gjoc, Bjoc, Rjugador1, Gjugador1, Bjugador1, Rjugador2, Gjugador2, Bjugador2;
int maxSegons = 10;
int numSegons, puntsJugador1, puntsJugador2;
unsigned long tempsRef;
String statusJoc = "aturat";

void setup() {
  for (int i = 2; i < 5; i++) { // Pins dels neopixels
    pinMode(i, OUTPUT);
  }
  lcd.begin(16, 2);
  Serial.begin(9600);
  botoStart.begin();
  ledsJoc.begin();
  ledsJugador1.begin();
  ledsJugador2.begin();
  Serial.println("PREMEU START");
  lcd.print(" JUGUEU !!!");
  lcd.setCursor(0, 1);
  lcd.print("PREMEU START");
  CreateTrulyRandomSeed(); // no es pot fer servir el pin A0 perquè està ocupat per un potenciòmetre
  randomSeed(seed);
}

void loop() {
  if (statusJoc == "aturat") {
    botoStart.pressed();
    generaColor();
    encenLeds(0);
    numSegons = maxSegons;
    Serial.println(numSegons);
    lcd.clear();
    lcd.print(numSegons);
    tempsRef = millis();
    statusJoc = "jugant";
  }

  if (statusJoc == "jugant") {
    calculaPunts();
    encenLeds(1);
    encenLeds(2);
    if (millis() > (tempsRef + 1000)) {
      tempsRef = millis();
      numSegons--;
      Serial.println(numSegons);
      lcd.clear();
      lcd.print(numSegons);
    }
    if (numSegons == 0 || puntsJugador1 == 0 || puntsJugador2 == 0) {
      gameOver(); // s'acaba la partida si algun jugador encerta o si s'acaba el temps
    }
  }
}

void generaColor() {
  lockedComponent = random(1, 4);
  Serial.print("Locked Component: ");
  Serial.println(lockedComponent);
  lockedValue = random(0, 256);
  Serial.print("Locked Value: ");
  Serial.println(lockedValue);
  if (lockedComponent == 1) {
    Rjoc = lockedValue;
  } else {
    Rjoc = random(0, 256);
  }
  if (lockedComponent == 2) {
    Gjoc = lockedValue;
  } else {
    Gjoc = random(0, 256);
  }
  if (lockedComponent == 3) {
    Bjoc = lockedValue;
  } else {
    Bjoc = random(0, 256);
  }
  Serial.print("Color RGB Joc: ");
  Serial.print(Rjoc);
  Serial.print(" ");
  Serial.print(Gjoc);
  Serial.print(" ");
  Serial.println(Bjoc);
}

void calculaPunts() {
  switch (lockedComponent) {
    case 1:
      Rjugador1 = Rjoc;
      Gjugador1 = map(analogRead(A0), 0, 1023, 0, 255);
      Bjugador1 = map(analogRead(A1), 0, 1023, 0, 255);
      Rjugador2 = Rjoc;
      Gjugador2 = map(analogRead(A2), 0, 1023, 0, 255);
      Bjugador2 = map(analogRead(A3), 0, 1023, 0, 255);
      break;
    case 2:
      Rjugador1 = map(analogRead(A0), 0, 1023, 0, 255);
      Gjugador1 = Gjoc;
      Bjugador1 = map(analogRead(A1), 0, 1023, 0, 255);
      Rjugador2 = map(analogRead(A2), 0, 1023, 0, 255);
      Gjugador2 = Gjoc;
      Bjugador2 = map(analogRead(A3), 0, 1023, 0, 255);
      break;
    case 3:
      Rjugador1 = map(analogRead(A0), 0, 1023, 0, 255);
      Gjugador1 = map(analogRead(A1), 0, 1023, 0, 255);
      Bjugador1 = Bjoc;
      Rjugador2 = map(analogRead(A2), 0, 1023, 0, 255);
      Gjugador2 = map(analogRead(A3), 0, 1023, 0, 255);
      Bjugador2 = Bjoc;
      break;
  }
  puntsJugador1 = abs(Rjugador1 - Rjoc) + abs(Gjugador1 - Gjoc) + abs(Bjugador1 - Bjoc);
  puntsJugador2 = abs(Rjugador2 - Rjoc) + abs(Gjugador2 - Gjoc) + abs(Bjugador2 - Bjoc);
}

void encenLeds(int numTira) {
  switch (numTira) {
    case 0:
      for (int i = 0; i < NUMPIXELS; i++) {
        ledsJoc.setPixelColor(i, ledsJoc.Color(Rjoc, Gjoc, Bjoc));
      }
      ledsJoc.show();
      break;
    case 1:
      for (int i = 0; i < NUMPIXELS; i++) {
        ledsJugador1.setPixelColor(i, ledsJugador1.Color(Rjugador1, Gjugador1, Bjugador1));
      }
      ledsJugador1.show();
      break;
    case 2:
      for (int i = 0; i < NUMPIXELS; i++) {
        ledsJugador2.setPixelColor(i, ledsJugador2.Color(Rjugador2, Gjugador2, Bjugador2));
      }
      ledsJugador2.show();
      break;
  }
}

void apagaLeds() {
  for (int i = 0; i < NUMPIXELS; i++) {
    ledsJoc.setPixelColor(i, ledsJoc.Color(0, 0, 0));
    ledsJugador1.setPixelColor(i, ledsJugador1.Color(0, 0, 0));
    ledsJugador2.setPixelColor(i, ledsJugador2.Color(0, 0, 0));
  }
  ledsJoc.show();
  ledsJugador1.show();
  ledsJugador2.show();
}

void gameOver() {
  me.effect_gameover();
  Serial.println(" GAME OVER");
  lcd.clear();
  lcd.print(" GAME OVER");
  delay(1000);
  me.effect_win();
  Serial.println(" GUANYADOR:");
  lcd.clear();
  lcd.print(" GUANYADOR:");
  lcd.setCursor(0, 1);
  if (puntsJugador1 < puntsJugador2) {
    Serial.println(" JUGADOR 1");
    lcd.print(" JUGADOR 1");
  }
  if (puntsJugador1 > puntsJugador2) {
    Serial.println(" JUGADOR 2");
    lcd.print(" JUGADOR 2");
  }
  if (puntsJugador1 == puntsJugador2) {
    Serial.println(" EMPAT");
    lcd.print(" EMPAT");
  }
  delay(2000);
  apagaLeds();
  Serial.println("PREMEU START");
  lcd.clear();
  lcd.print("JUGUEU !!!");
  lcd.setCursor(0, 1);
  lcd.print("PREMEU START");
  statusJoc = "aturat";
}

void CreateTrulyRandomSeed()
{
  seed = 0;
  nrot = 32;
  cli();
  MCUSR = 0;
  _WD_CONTROL_REG |= (1 << _WD_CHANGE_BIT) | (1 << WDE);
  _WD_CONTROL_REG = (1 << WDIE);
  sei();
  while (nrot > 0);
  cli();
  MCUSR = 0;
  _WD_CONTROL_REG |= (1 << _WD_CHANGE_BIT) | (0 << WDE);
  _WD_CONTROL_REG = (0 << WDIE);
  sei();
}



