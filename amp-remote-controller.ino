#include <IRremote.h>

const int VOL_UP = 1;
const int VOL_DOWN = -1;
const int RECV_PIN = 0;
const int MOTOR_0_PIN = 1;
const int MOTOR_1_PIN = 2;
const int LED_PIN = 4;

volatile unsigned long milliseconds;
unsigned long lastReceived = 0;
int lastDirection = 0;

bool isBlinking = false;
bool isLedOn = true;
unsigned long lastBlink = 0; 

IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  #ifndef __AVR_ATtiny85__
    Serial.begin(9600);
  #endif

  pinMode(MOTOR_0_PIN, OUTPUT);
  pinMode(MOTOR_1_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  stopBlinking();
  
  lastReceived = getMiliseconds();
  stopMotor();
  
  irrecv.enableIRIn(); // Start the receiver
  
  TCCR1 = (1<<CTC1)|(7<<CS10);   // CTC  mode, div64
  OCR1C = 0.001 * F_CPU/32 - 1;  // 1ms, F_CPU @16MHz, div64
  TIMSK |= (1<<OCIE1A);
    
  sei();
}

void loop() {
  if (getMiliseconds() - lastReceived > 70) {
    #ifndef __AVR_ATtiny85__
      Serial.println(getMiliseconds() - lastReceived);
    #endif

    stopMotor();
    stopBlinking();
  }

  updateBlinking();

  if (irrecv.decode(&results)) {
    #ifndef __AVR_ATtiny85__
      Serial.println(results.value, HEX);
    #endif
    
    switch (results.value) {
      case 0x2FDB04F:
        lastDirection = VOL_UP;
        lastReceived = getMiliseconds();
        startMotor(lastDirection);
        startBlinking();

        break;
      case 0x2FDA857:
        lastDirection = VOL_DOWN;
        lastReceived = getMiliseconds();
        startMotor(lastDirection);
        startBlinking();
        
        break;
      case 0xFFFFFFFF:
        lastReceived = getMiliseconds();

        break;
      default:
        break;
    }

    irrecv.resume();
  }
}

void startMotor(int direction) {
  if (direction == VOL_UP) {
    digitalWrite(MOTOR_1_PIN, LOW);
    digitalWrite(MOTOR_0_PIN, HIGH);
  } else if (direction == VOL_DOWN) {
    digitalWrite(MOTOR_0_PIN, LOW);
    digitalWrite(MOTOR_1_PIN, HIGH);
  }
}

void stopMotor() {
  digitalWrite(MOTOR_0_PIN, LOW);
  digitalWrite(MOTOR_1_PIN, LOW);  
}


void startBlinking() {
  isBlinking = true;
  isLedOn = true;
  digitalWrite(LED_PIN, HIGH);
  lastBlink = getMiliseconds();
}

void stopBlinking() {
  isBlinking = false;
  isLedOn = true;
  digitalWrite(LED_PIN, HIGH);
}

void updateBlinking() {
  unsigned long currentMiliseconds = getMiliseconds();
  if (isBlinking && currentMiliseconds - lastBlink > 50) {
    digitalWrite(LED_PIN, isLedOn ? LOW : HIGH);
    isLedOn = !isLedOn;
    lastBlink = currentMiliseconds;
  }  
}

unsigned long getMiliseconds() {
  return milliseconds;
}

ISR(TIMER1_COMPA_vect) {
  milliseconds++;
}

