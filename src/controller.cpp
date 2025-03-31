#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "WifiManager.h"

// Пин потенциометра
#define POT_PIN 1  // ADC1_CH0

// Настройки MQTT
const char* mqtt_server = "YOUR_MQTT_BROKER";
const int mqtt_port = 1883;
const char* mqtt_user = "YOUR_MQTT_USER";      // если требуется
const char* mqtt_password = "YOUR_MQTT_PASS";   // если требуется

// Топик MQTT для отправки данных
const char* control_topic = "robot_arm/servo/1";  // Управление первым сервоприводом

// Создание клиентов WiFi и MQTT
WiFiClient espClient;
PubSubClient client(espClient);
WifiManager wifiManager;

// Переменные для работы с потенциометром
int lastPotValue = 0;
int potValue = 0;
unsigned long lastSendTime = 0;
const int SEND_INTERVAL = 50;  // Интервал отправки данных (мс)
const int VALUE_THRESHOLD = 5; // Минимальное изменение для отправки

// Функция переподключения к MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP32C3_Controller_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.flush();
  delay(50);

  Serial.println("\n\n=====================================");
  Serial.println("    Robot Arm Controller v1.0");
  Serial.println("=====================================");
  
  // Настройка пина потенциометра
  pinMode(POT_PIN, INPUT);
  
  // Подключение к WiFi
  Serial.println("Starting WiFi configuration...");
  wifiManager.begin();
  while (!wifiManager.isConnected()) {
    wifiManager.handle();
    delay(10);
  }
  
  // Настройка MQTT
  Serial.println("Configuring MQTT connection...");
  client.setServer(mqtt_server, mqtt_port);
  
  Serial.println("\nController initialization complete!");
  Serial.println("Ready to send control signals");
  Serial.println("=====================================\n");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  wifiManager.handle();
  client.loop();

  // Чтение значения потенциометра
  potValue = analogRead(POT_PIN);
  
  // Преобразование значения в диапазон 0-180 для сервопривода
  int servoAngle = map(potValue, 0, 4095, 0, 180);
  
  // Отправка данных только если:
  // 1. Прошло достаточно времени с последней отправки
  // 2. Значение изменилось больше чем на пороговое значение
  if ((millis() - lastSendTime > SEND_INTERVAL) && 
      (abs(servoAngle - lastPotValue) > VALUE_THRESHOLD)) {
    
    // Преобразование значения в строку и отправка
    String angleStr = String(servoAngle);
    client.publish(control_topic, angleStr.c_str());
    
    // Обновление последних значений
    lastPotValue = servoAngle;
    lastSendTime = millis(); 
    
    // Вывод в Serial для отладки
    Serial.print("Sent angle: ");
    Serial.println(servoAngle);
  }
  
  delay(10);
} 