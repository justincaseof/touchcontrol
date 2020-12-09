#include <Arduino.h>

int LED_blue = LED_BUILTIN;
int LED_ON = 0;
int LED_OFF = 1;
int run = 0;
char buf[64];

void setup() {
    Serial.begin(9600);
    pinMode(LED_blue, OUTPUT);
  
}

void doStuff() {
}

void loop() {
    while(1) {
        sprintf(buf, "#%i", run++);
        Serial.println(buf);

        doStuff();

        digitalWrite(LED_blue, LED_ON);
        delay(1000);
        digitalWrite(LED_blue, LED_OFF);
        delay(1000);
    }
}

