#ifndef ROBOT_ARM_H
#define ROBOT_ARM_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Прототипы функций
void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

#endif 