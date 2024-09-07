#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_TinyUSB.h>

int count = 0;

void setup() {
  Serial1.begin(115200);
  Serial1.println("Starting");

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  Serial1.println(count++);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}