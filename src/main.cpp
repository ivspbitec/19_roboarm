#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "WifiManager.h"

// Настройки WiFi
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Настройки MQTT
const char* mqtt_server = "YOUR_MQTT_BROKER";
const int mqtt_port = 1883;
const char* mqtt_user = "YOUR_MQTT_USER";      // если требуется
const char* mqtt_password = "YOUR_MQTT_PASS";   // если требуется

// Топики MQTT
const char* servo_topic = "robot_arm/servo/#";  // Топик для управления сервоприводами
const char* status_topic = "robot_arm/status";  // Топик для статуса

// Определение пинов для сервоприводов
#define SERVO1_PIN 2  // Основание
#define SERVO2_PIN 3  // Плечо
#define SERVO3_PIN 4  // Локоть
#define SERVO4_PIN 5  // Захват

// Создание объектов сервоприводов
Servo servo1;  // Основание
Servo servo2;  // Плечо
Servo servo3;  // Локоть
Servo servo4;  // Захват

// Текущие углы сервоприводов
int currentAngle1 = 90;
int currentAngle2 = 90;
int currentAngle3 = 90;
int currentAngle4 = 90;

// Создание клиентов WiFi и MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Добавить перед другими глобальными переменными
WifiManager wifiManager;

// Функция подключения к WiFi
void setup_wifi() {
    wifiManager.begin();
    while (!wifiManager.isConnected()) {
        wifiManager.handle();
        delay(10);
    }
    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(wifiManager.getIP());
}

// Функция переподключения к MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP32C3_RobotArm_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      client.publish(status_topic, "Robot arm online");
      client.subscribe(servo_topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Обработчик MQTT сообщений
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  String topicStr = String(topic);
  int angle = message.toInt();
  
  // Проверяем, какой сервопривод нужно повернуть
  if (topicStr == "robot_arm/servo/1") {
    servo1.write(constrain(angle, 0, 180));
    currentAngle1 = angle;
    client.publish("robot_arm/status", ("Servo 1 set to " + String(angle)).c_str());
  }
  else if (topicStr == "robot_arm/servo/2") {
    servo2.write(constrain(angle, 0, 180));
    currentAngle2 = angle;
    client.publish("robot_arm/status", ("Servo 2 set to " + String(angle)).c_str());
  }
  else if (topicStr == "robot_arm/servo/3") {
    servo3.write(constrain(angle, 0, 180));
    currentAngle3 = angle;
    client.publish("robot_arm/status", ("Servo 3 set to " + String(angle)).c_str());
  }
  else if (topicStr == "robot_arm/servo/4") {
    servo4.write(constrain(angle, 0, 180));
    currentAngle4 = angle;
    client.publish("robot_arm/status", ("Servo 4 set to " + String(angle)).c_str());
  }
}

void setup() {
   delay(1000); // Дать время USB переподключиться
  Serial.begin(115200);
  while (!Serial); // Особенно важно для USB CDC
 

  Serial.println("\n\n=====================================");
  Serial.println("    Robot Arm Controller v1.0");
  Serial.println("=====================================");
  Serial.println("Initializing system...");
  
  // Подключение к WiFi
  Serial.println("Starting WiFi configuration...");
  setup_wifi();
  
  // Настройка MQTT
  Serial.println("Configuring MQTT connection...");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  // Подключение сервоприводов к пинам
  Serial.println("Initializing servo motors...");
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);
  servo3.setPeriodHertz(50);
  servo4.setPeriodHertz(50);
  
  servo1.attach(SERVO1_PIN, 500, 2400);
  servo2.attach(SERVO2_PIN, 500, 2400);
  servo3.attach(SERVO3_PIN, 500, 2400);
  servo4.attach(SERVO4_PIN, 500, 2400);
  
  // Установка начального положения
  Serial.println("Setting initial servo positions...");
  servo1.write(currentAngle1);
  servo2.write(currentAngle2);
  servo3.write(currentAngle3);
  servo4.write(currentAngle4);
  
  Serial.println("\nSystem initialization complete!");
  Serial.println("Robot arm is ready for operation");
  Serial.println("=====================================\n");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  wifiManager.handle();
  client.loop();
  
  delay(20);
} 