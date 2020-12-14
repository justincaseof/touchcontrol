#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Servo.h>

int LED_blue = LED_BUILTIN; // == D4
int LED_ON = 0;
int LED_OFF = 1;
char buf[64];

int RECV_PIN = D3; //an IR detector connected to D3
IRrecv irrecv(RECV_PIN);
decode_results results;

int SERVO_PIN = D1;
Servo servo;
/**
 * Servo MC-410
 *   --> min-value: ~26
 *   --> max-value: ~166
 */
int SERVO_MIN = 42;
int SERVO_MAX = 166;

int SERVO_IDLE = SERVO_MAX - 6;
int servo_angle_targetpos = SERVO_MIN;
int pushdelay = 1000; // minimum delay to make arm travel from 26 to 60 and back

void onIRinput();

void setup() {
    Serial.begin(9600);
    
    // LED
    pinMode(LED_blue, OUTPUT);
    irrecv.enableIRIn(); // Start the receiver

    // Servo
    servo.attach(SERVO_PIN);
    servo.write(SERVO_IDLE);  // zero-pos
    
}

/*
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
}
*/

void irStuff() {
    if (irrecv.decode(&results)) {
        //dump(&results);
        onIRinput();      // handle this.
        irrecv.resume(); // Receive the next value
    }
}

int EVENT_NONE = 0;
int EVENT_IR_EVENT_BUTTON_OK = 1;
int event = EVENT_NONE;

void onIRinput() {
    // FF18E7 --> UP
    // FF4AB5 --> DOWN
    // FF10EF --> LEFT
    // FF5AA5 --> RIGHT
    // FF38C7 --> OK

    if (results.value == 0xFF38C7) {
        Serial.println("OK");
        event = EVENT_IR_EVENT_BUTTON_OK;
    } else if (results.value == 0xFF18E7) {
        pushdelay += 10;
    } else if (results.value == 0xFF4AB5) {
        pushdelay -= 10;
    } else if (results.value == 0xFF5AA5) {
        servo_angle_targetpos += 10;
    } else if (results.value == 0xFF10EF) {
        servo_angle_targetpos -= 10;
    }
    
    // verify
    if (pushdelay < 10) {
      pushdelay = 10;
    }
    if (servo_angle_targetpos < SERVO_MIN) {
      servo_angle_targetpos = SERVO_MIN;
    }
    if (servo_angle_targetpos > SERVO_MAX) {
      servo_angle_targetpos = SERVO_MAX;
    }

    // log
    sprintf(buf, "# pushdelay=%d, pushpos=%d", pushdelay, servo_angle_targetpos);
    Serial.println(buf);
}

void servo_driveToTarget() {
    // v1) directly set target angle
    servo.write(servo_angle_targetpos);

/*
    // v2) slowly drive to target angle
    int servo_pos_intermediate = SERVO_IDLE;
    int diff = servo_pos_intermediate - servo_angle_targetpos;
    int increments = 5;
    while (servo_pos_intermediate > servo_angle_targetpos) {
        if (servo_pos_intermediate - servo_angle_targetpos < 20 || servo_pos_intermediate - servo_angle_targetpos > 114) {
            servo_pos_intermediate -= 5;
        } else {
            servo_pos_intermediate -= 15;
        }
        servo.write(servo_pos_intermediate);

        delay(10);
    }
    */
}

void servo_driveToIdle() {
    // v1) directly set target angle
    servo.write(SERVO_IDLE);
}

void servoStuff() {
    // reset?
    if (event == EVENT_IR_EVENT_BUTTON_OK) {
        servo_driveToTarget();
        delay(pushdelay);
        servo_driveToIdle();
        event = 0;  // reset event
    } else {
        servo_driveToIdle();
    }
}

/*
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
*/
int run = 0;
void loop() {
    servoStuff();
    irStuff();
    //servoTest();

    if (run++%16 == 0) {
      digitalWrite(LED_blue, LED_ON);
      delay(50);
      digitalWrite(LED_blue, LED_OFF);
      delay(50);
    } else {
      delay(100);
    }
}


