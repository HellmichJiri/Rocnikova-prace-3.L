// Import externích knihoven
#include <Wire.h>
#include <RCSwitch.h>

// Definice portů
#define LED_FL 5
#define LED_FR 6
#define LED_RL 7
#define LED_RR 8

// definice konstant
#define SERIAL_SPEED 115200
#define DEBUG 1
#define RESTRICTED_ANGLE 20
#define MESSAGE_LENGTH 32
#define MULTIPLY 2


// inicializace proměnné pro určení adresy senzoru
const int MPU_addr = 0x68;
// inicializace proměnných, do kterých se uloží data
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
int16_t AcX_calc, AcY_calc;
byte x, y = 0;

unsigned long message = 0L;


// Vytvoření instance přijímače
RCSwitch transmiter = RCSwitch();

void setup() {
  // komunikace přes I2C sběrnici
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  
  Serial.begin(SERIAL_SPEED);
  pinMode(LED_FL, OUTPUT);
  pinMode(LED_FR, OUTPUT);
  pinMode(LED_RL, OUTPUT);
  pinMode(LED_RR, OUTPUT);
  transmiter.enableTransmit(10);
}

void loop() {
  digitalWrite(LED_FL, LOW);
  digitalWrite(LED_FR, LOW);
  digitalWrite(LED_RL, LOW);
  digitalWrite(LED_RR, LOW);
  
  // zapnutí přenosu
  Wire.beginTransmission(MPU_addr);
  // zápis do registru 
  Wire.write(0x3B);
  Wire.endTransmission(false);
  // vyzvednutí dat z registrů
  Wire.requestFrom(MPU_addr, 14, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  // Převod na stupně
  AcX_calc =  convertRadToDegree(AcX);
  AcY_calc =  convertRadToDegree(AcY);
  
  if (DEBUG){
    Serial.print("AcX_calc: ");
    Serial.println(AcX_calc);
    Serial.print("AcY_calc: ");
    Serial.println(AcY_calc);
  }

  // Výpočet, které LED diody májí svít a jejich rozsvícení
  if (!(abs(AcY_calc) < RESTRICTED_ANGLE && abs(AcX_calc) < RESTRICTED_ANGLE)) {
    if (AcY_calc > -RESTRICTED_ANGLE && AcX_calc < RESTRICTED_ANGLE) {
      digitalWrite(LED_FL, HIGH);
    }
    if (AcY_calc > -RESTRICTED_ANGLE && AcX_calc > -RESTRICTED_ANGLE) {
      digitalWrite(LED_FR, HIGH);
    }
    if (AcY_calc < RESTRICTED_ANGLE && AcX_calc < RESTRICTED_ANGLE) {
      digitalWrite(LED_RL, HIGH);
    }
    if (AcY_calc < RESTRICTED_ANGLE && AcX_calc > -RESTRICTED_ANGLE) {
      digitalWrite(LED_RR, HIGH);
    }
  }

  // vypočítání a přizpůsobení úhlu
  x = angleAdjustment(AcX_calc);
  y = angleAdjustment(AcY_calc);


  if (DEBUG){
    Serial.print("x: ");
    Serial.println(x);
    Serial.print("y: ");
    Serial.println(y);
  }

  message = x;
  message = message << 16;
  message = message + y;

  if (DEBUG){
    Serial.print("Zpráva k odeslání: ");
    Serial.println(message);
  }

  // odeslání zprávy
  transmiter.send(message, MESSAGE_LENGTH);
  delay(5);
}



int convertRadToDegree(int radAngle){
  //  Převod úhlu z radiánů na stupně

  return radAngle * -0.0001 * 180 / 3.1416;
}


byte angleAdjustment(int angle){  
  int returnValue = 0;
  
  // ignorování malého náklonu
  if (abs(angle) >= RESTRICTED_ANGLE){
    returnValue = angle;
  }

  // Zvýšení úhlu na dvojnásobek -> auto dosáhne maxima v 45°
  returnValue = returnValue * MULTIPLY;

  if (returnValue > 90) { 
    returnValue = 90;
  }
  if (returnValue < -90) { 
    returnValue = -90;
  }

  Serial.print("returnValue: ");
  Serial.println(returnValue);
  // převod na byte a posunutí rozsahu z <-90;90> na <0;+180>
  return (byte) abs(returnValue + 90);
}