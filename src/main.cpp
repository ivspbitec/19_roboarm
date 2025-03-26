#include <Arduino.h>

// Пины
const int LED_PIN = 8;
const int SERVO_PIN = 4;        // GPIO4 для сервопривода
const int POT_PIN = 3;          // GPIO3 для потенциометра

// PWM параметры
const int SERVO_FREQ = 50;      // 50 Гц — стандартная частота для серв
const int SERVO_CHANNEL = 0;    
const int TIMER_WIDTH = 14;     // 14 бит (0–16383)

void setup() {
  Serial.begin(74880);
  pinMode(LED_PIN, OUTPUT);

  // Настройка PWM
  ledcSetup(SERVO_CHANNEL, SERVO_FREQ, TIMER_WIDTH);
  ledcAttachPin(SERVO_PIN, SERVO_CHANNEL);

  // Настройка ADC
  analogReadResolution(12); // 0–4095

  Serial.println("System initialized");
}

void loop() {
  // Мигание светодиода
  static unsigned long previousMillis = 0;
  static bool ledState = false;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }

  // Чтение потенциометра
  int potValue = analogRead(POT_PIN); // 0–4095

  // Преобразуем значение в длительность импульса (в микросекундах)
  int pulseWidth = map(potValue, 0, 4095, 500, 2400); // от 0.5 до 2.4 мс

  // Переводим в значение PWM (14-битная шкала для 20мс периода)
  int duty = (pulseWidth * 16383L) / 20000;

  ledcWrite(SERVO_CHANNEL, duty);

  delay(20); // период сервосигнала — 50 Гц
}
