#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>

class WifiManager {
private:
    static const int WIFI_TIMEOUT = 300000; // 5 минут в миллисекундах
    
    const char* ap_ssid = "RobotArm_Setup";
    const char* ap_password = "12345678";
    const char* PREF_NAMESPACE = "wificonfig"; // Namespace для Preferences
    
    WebServer server;
    DNSServer dnsServer;
    Preferences preferences;
    
    String stored_ssid;
    String stored_password;
    
    unsigned long lastConnectedTime;
    bool isAPMode;

    void startAP();
    void handleRoot();
    void handleSave();
    void saveCredentials(const String &ssid, const String &password);
    void loadCredentials();
    void clearCredentials();
    
public:
    WifiManager();
    void begin();
    void handle();
    bool isConnected();
    String getIP();
};

#endif 