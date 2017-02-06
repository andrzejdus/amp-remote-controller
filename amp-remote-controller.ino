#include <math.h>
#include <IRremote.h>

const int VOL_UP = 1;
const int VOL_DOWN = -1;
const int RECV_PIN = 0;
const int MOTOR_0_PIN = 1;
const int MOTOR_1_PIN = 2;
const int LED_PIN = 4;

IRrecv irrecv(RECV_PIN);
decode_results results;

unsigned long lastClick = 0;

void setup()
{
  #ifndef __AVR_ATtiny85__
    Serial.begin(9600);
  #endif

  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_0_PIN, OUTPUT);
  pinMode(MOTOR_1_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(MOTOR_0_PIN, HIGH);
  digitalWrite(MOTOR_1_PIN, LOW);  

  lastClick = mm();
  
  irrecv.enableIRIn(); // Start the receiver
//  irrecv.blink13(1);
  
  updateVolume(0);

  TCCR1 = (1<<CTC1)|(7<<CS10);   // CTC  mode, div64
  OCR1C = 0.001 * F_CPU/32 - 1;  // 1ms, F_CPU @16MHz, div64
  TIMSK |= (1<<OCIE1A);
    
  sei();
}

volatile unsigned long milliseconds;


ISR(TIMER1_COMPA_vect)
{

    milliseconds++;
}

unsigned long mm()
{
       
    return milliseconds;
}

int lastDirection = 0;

void loop() {

  if (mm() - lastClick > 60) {
    #ifndef __AVR_ATtiny85__
      Serial.println(mm() - lastClick);
    #endif
    digitalWrite(MOTOR_0_PIN, LOW);
    digitalWrite(MOTOR_1_PIN, LOW);
  }

  if (irrecv.decode(&results)) {
    #ifndef __AVR_ATtiny85__
      Serial.println(results.value, HEX);
    #endif
    
    switch (results.value) {
      case 0x2FDB04F:
        updateVolume(VOL_UP);
        lastDirection = VOL_UP;
 
        lastClick = mm();
        digitalWrite(MOTOR_1_PIN, LOW);
        digitalWrite(MOTOR_0_PIN, HIGH);
        break;
      case 0x2FDA857:
        updateVolume(VOL_DOWN);
        lastDirection = VOL_DOWN;

        lastClick = mm();
        digitalWrite(MOTOR_0_PIN, LOW);
        digitalWrite(MOTOR_1_PIN, HIGH);
        break;
      case 0xFFFFFFFF:
        updateVolume(lastDirection);

        lastClick = mm();
        if (lastDirection == VOL_DOWN) {
          digitalWrite(MOTOR_0_PIN, LOW);
          digitalWrite(MOTOR_1_PIN, HIGH);
        }
        if (lastDirection == VOL_UP) {
          digitalWrite(MOTOR_1_PIN, LOW);
          digitalWrite(MOTOR_0_PIN, HIGH);
        }
        break;
      default:
        break;
    }

    

//    digitalWrite(LED_PIN, HIGH);
//    delay(50);
//    digitalWrite(LED_PIN, LOW);

    irrecv.resume(); // Receive the next value
  }
}

int volume = 100;
void updateVolume(int direction) {
    volume += int(direction * exp(volume / 75.0));
    volume = max(0, volume);
    volume = min(100, volume);
    
    analogWrite(LED_PIN, volume);
//    digitalWrite(LED_PIN, HIGH);
    
    #ifndef __AVR_ATtiny85__
      Serial.println(volume);
    #endif


//    if (direction < 0) {
//      digitalWrite(MOTOR_0_PIN, HIGH);
//    } else {
//      digitalWrite(MOTOR_1_PIN, HIGH);
//    }
//    _delay_ms(10);
//    digitalWrite(MOTOR_0_PIN, LOW);
//    digitalWrite(MOTOR_1_PIN, LOW);
}
