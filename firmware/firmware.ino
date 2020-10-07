/*
 * Pop'n Arcade Controller
 * https://github.com/it9gamelog/popncontroller
 *
 * MIT License
 * 
 * Copyright (c) 2020 IT9GameLog
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "CapacitiveSensor.h"
#include "PopnController.h"

// Button Pins
CapacitiveSensor sensors[] = {
  CapacitiveSensor(0, 1),
  CapacitiveSensor(0, 2),
  CapacitiveSensor(0, 3),
  CapacitiveSensor(0, A5),
  CapacitiveSensor(0, A4),
  CapacitiveSensor(0, A3),
  CapacitiveSensor(0, A2),
  CapacitiveSensor(0, A1),
  CapacitiveSensor(0, A0)
};

// LED Pins
uint8_t led_pins[] = {4, 5, 6, 7, 8, 9, 10, 11, 12};

const auto size = sizeof(sensors) / sizeof(CapacitiveSensor);

void setup() {
  // Serial.begin(9600);
  PopnController.send();
  for (auto i = 0; i < size; i++)
    pinMode(led_pins[i], OUTPUT);
}

void loop() {
  long start = millis();

  // This loop is about 10ms
  // Each takes about 1ms to sense
  for (auto i = 0; i < size; i++)
  {
    int r = sensors[i].sense(15);
    Serial.print(r);
    Serial.print('\t');
    if (r > 35)
    {
      PopnController.press(i);
    } else
    {
      PopnController.release(i);
    }
  }

  // Send latest keyboard report
  PopnController.send();

  // Update LEDs
  uint16_t leds = PopnController.getLeds();
  // Serial.print(leds);  
  for (auto i = 0; i < size; i++)
  {
    digitalWrite(led_pins[i], leds & 1);
    leds >>= 1;
  }

  Serial.println(millis() - start);
}
