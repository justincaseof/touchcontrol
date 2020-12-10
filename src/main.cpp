#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Servo.h>

int LED_blue = LED_BUILTIN; // == D4
int LED_ON = 0;
int LED_OFF = 1;
int run = 0;
char buf[64];

int RECV_PIN = D3; //an IR detector connected to D3
IRrecv irrecv(RECV_PIN);
decode_results results;

int SERVO_PIN = D1;
Servo servo;
void onIRinput();

void setup() {
    Serial.begin(9600);
    
    // LED
    pinMode(LED_blue, OUTPUT);
    irrecv.enableIRIn(); // Start the receiver

    // Servo
    servo.attach(SERVO_PIN);
    
}

void dump(decode_results *results) {
  // Dumps out the decode_results structure.
  // Call this after IRrecv::decode()
  uint16_t count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  } else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  } else if (results->decode_type == RC5X) {
    Serial.print("Decoded RC5X: ");
  } else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  } else if (results->decode_type == RCMM) {
    Serial.print("Decoded RCMM: ");
  } else if (results->decode_type == PANASONIC) {
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->address, HEX);
    Serial.print(" Value: ");
  } else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  } else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  } else if (results->decode_type == AIWA_RC_T501) {
    Serial.print("Decoded AIWA RC T501: ");
  } else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  } else if (results->decode_type == NIKAI) {
    Serial.print("Decoded Nikai: ");
  }
  serialPrintUint64(results->value, 16);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  /*
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): {");

  for (uint16_t i = 1; i < count; i++) {
    if (i % 100 == 0)
      yield();  // Preemptive yield every 100th entry to feed the WDT.
    if (i & 1) {
      Serial.print(results->rawbuf[i] * kRawTick, DEC);
    } else {
      Serial.print(", ");
      Serial.print((uint32_t) results->rawbuf[i] * kRawTick, DEC);
    }
  }
  Serial.println("};");
  */
}

void irStuff() {
    if (irrecv.decode(&results)) {
        //dump(&results);
        onIRinput();      // handle this.
        irrecv.resume(); // Receive the next value
    }
}

int event = 0;
void onIRinput() {
    // FF18E7 --> UP
    // FF4AB5 --> DOWN
    // FF10EF --> LEFT
    // FF5AA5 --> RIGHT
    // FF38C7 --> OK

    if (results.value == 0xFF38C7) {
        Serial.println("OK");
        event = 1;
    }
}

/**
 * Servo MC-410
 *   --> min-value: ~26
 *   --> max-value: ~166
 */
int SERVO_MIN = 26;
int SERVO_MAX = 166;
void servoStuff() {
    // reset?
    if (event) {
        servo.write(SERVO_MAX); // this is the maximum
        delay(800);
        servo.write(SERVO_MIN);
        event = 0;
    } else {
      servo.write(SERVO_MIN);  // this is the servo's "reset" value after poweron
    }
}

void servoTest() {
  for(int pos = 0; pos < 180; pos += 1) {  // von 0 bis 180 Grad, in Schritten von einem Grad
    servo.write(pos);                   // sagt dem Servomotor, in welche Position sich drehen soll      
    delay(150);                            // wartet 15 Millisekunden   
    
    sprintf(buf, "#%i", pos);
    Serial.println(buf);
  }    
  for(int pos = 180; pos>=1; pos-=1) {     // und das gleiche zurück
    servo.write(pos);
    delay(150);

    sprintf(buf, "#%i", pos);
    Serial.println(buf);
  }
}

void loop() {
    servoStuff();
    irStuff();
    //servoTest();

    digitalWrite(LED_blue, LED_ON);
    delay(50);
    digitalWrite(LED_blue, LED_OFF);
    delay(50);
}


