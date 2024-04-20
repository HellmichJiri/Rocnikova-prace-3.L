// Import externích knihoven

#include <RCSwitch.h>  
#include <Servo.h>     

// Definice portů
#define MOTOR_LEFT_FORWARD 3
#define MOTOR_LEFT_BACKWARD 5
#define MOTOR_RIGHT_FORWARD 11
#define MOTOR_RIGHT_BACKWARD 6
#define SERVO_PIN 7
#define RF_COMMUNICATION_PIN 2

#define STEERING_OFFSET 88    // Pozice servomotoru pro jízdu rovně
#define PARSING_MASK 0x000000FF

#define SERIAL_SPEED 115200
#define DEBUG 1

// Vytvoření instance přijímače
RCSwitch receiver = RCSwitch();
// Vytvoření instance servo motoru pro řízení
Servo steering;
// Pole pro ovládání motorů
byte motorSpeed[] = { 0, 0, 0, 0 };

byte x, y = 0;
unsigned long message = 0L;

void setup() {
  // Nastavení sériové komunikace
  Serial.begin(SERIAL_SPEED);

  // Nastavení pinů arduina pro výstup
  pinMode(SERVO_PIN, OUTPUT);

  steering.attach(SERVO_PIN);
  steering.write(STEERING_OFFSET);
  delay(20);

  // nastavení přijacího pinu na interrupt
  receiver.enableReceive(0);
}

void loop() {
  // detekce přichozí komunikace
  if (receiver.available()) {
    // kontrola velikosti zprávy
    if (receiver.getReceivedValue() == 0) {
      // neznámé kódování nebo chyba v komunikaci
      if (DEBUG) { 
        Serial.println("Neznama zprava!"); 
      }
    }
    // úspěšně přijatá zpráva
    else {
      message = receiver.getReceivedValue();

      if (DEBUG) { 
        Serial.print("Prijata zprava: ");
        Serial.println(message); 
      };

      // Parsování hodnot ze zprávy
      y = message & PARSING_MASK;
      message = message >> 16;
      x = message & PARSING_MASK;

      if (DEBUG) {
        Serial.print("X: ");
        Serial.println(x);
        Serial.print("Y: ");
        Serial.println(y);
      }

      // Vypočítání a nastavení požadovaného úhlu
      steering.write(calculateSteering(x));
      

      // Vypočítání a nastavení rychlosti
      calculateSpeed(y, motorSpeed);
      setSpeed(motorSpeed);

      if (DEBUG) {
        Serial.print("Motory: ");
        Serial.print(motorSpeed[0]);
        Serial.print(" - ");
        Serial.print(motorSpeed[1]);
        Serial.print(" - ");
        Serial.print(motorSpeed[2]);
        Serial.print(" - ");
        Serial.println(motorSpeed[3]);
      }
    }
    // reset modulu pro přijetí nových zpráv
    receiver.resetAvailable();
  }
}

// Funkce pro vypočítání požadovaného úhlu
int calculateSteering(byte value) {

  return map(value, 0, 180, 0, 60) - 30 + STEERING_OFFSET;
}


void calculateSpeed(byte value, byte motors[]) {
// Funkce pro vypočítání rychlosti
      if (value == 90) {
        motors[0] = 0;
        motors[1] = 0;
        motors[2] = 0;
        motors[3] = 0;
      } else {
        if (value > 90) {
          // jízda dopředu
          motors[0] = map(value - 90, 0, 90, 0, 255);
          motors[1] = 0;
          motors[2] = map(value - 90, 0, 90, 0, 255);
          motors[3] = 0;
        } else {
          // jízda dozadu
          motors[0] = 0;
          motors[1] = map(value, 0, 90, 255, 0);
          motors[2] = 0;
          motors[3] = map(value, 0, 90, 255, 0);
        }
      }
}


void setSpeed(byte motors[]) {

  // Zápis hodnot na výstupní piny arduina pro řízení motorů)

    analogWrite(MOTOR_LEFT_FORWARD, motors[0]);
    analogWrite(MOTOR_LEFT_BACKWARD, motors[1]);
    analogWrite(MOTOR_RIGHT_FORWARD, motors[2]);
    analogWrite(MOTOR_RIGHT_BACKWARD, motors[3]);
}
