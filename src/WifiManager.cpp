#include "WifiManager.h"

WifiManager::WifiManager() : server(80), isAPMode(false) {
    lastConnectedTime = 0;
}

void WifiManager::begin() {
    loadCredentials(); 
    
    if (stored_ssid.length() > 0) {
        // Пытаемся подключиться к сохраненной сети
        WiFi.begin(stored_ssid.c_str(), stored_password.c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            lastConnectedTime = millis();
            return;
        }
    }
    
    // Если нет сохраненных данных или подключение не удалось
    startAP();
}

void WifiManager::startAP() {
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);
    
    // Настройка DNS сервера для перенаправления всех запросов на портал
    dnsServer.start(53, "*", WiFi.softAPIP());
    
    // Настройка веб-сервера
    server.on("/", [this]() { handleRoot(); });
    server.on("/save", [this]() { handleSave(); });
    server.begin();
    
    isAPMode = true;
}

void WifiManager::handleRoot() {
    String html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Robot Arm WiFi Setup</title>
            <meta name="viewport" content="width=device-width, initial-scale=1">
            <style>
                body { font-family: Arial; padding: 20px; }
                input { width: 100%; padding: 12px; margin: 8px 0; }
                button { background-color: #4CAF50; color: white; padding: 14px 20px; border: none; width: 100%; }
            </style>
        </head>
        <body>
            <h2>Robot Arm WiFi Setup</h2>
            <form action="/save" method="post">
                <input type="text" name="ssid" placeholder="WiFi Name">
                <input type="password" name="password" placeholder="WiFi Password">
                <button type="submit">Save</button>
            </form>
        </body>
        </html>
    )";
    server.send(200, "text/html", html);
}

void WifiManager::handleSave() {
    String new_ssid = server.arg("ssid");
    String new_password = server.arg("password");
    
    if (new_ssid.length() > 0) {
        saveCredentials(new_ssid, new_password);
        
        String html = "Settings saved. Device will restart in 5 seconds...";
        server.send(200, "text/html", html);
        
        delay(5000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "SSID cannot be empty");
    }
}

void WifiManager::saveCredentials(const String &ssid, const String &password) {
    preferences.begin(PREF_NAMESPACE, false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
}

void WifiManager::loadCredentials() {
    preferences.begin(PREF_NAMESPACE, true);
    stored_ssid = preferences.getString("ssid", "");
    stored_password = preferences.getString("password", "");
    preferences.end();
}

void WifiManager::clearCredentials() {
    preferences.begin(PREF_NAMESPACE, false);
    preferences.clear();
    preferences.end();
    stored_ssid = "";
    stored_password = "";
}

void WifiManager::handle() {
    if (isAPMode) {
        dnsServer.processNextRequest();
        server.handleClient();
    } else {
        if (WiFi.status() == WL_CONNECTED) {
            lastConnectedTime = millis();
        } else {
            // Если связи нет больше 5 минут
            if (millis() - lastConnectedTime > WIFI_TIMEOUT) {
                clearCredentials();
                startAP();
            } else {
                // Пытаемся переподключиться
                WiFi.begin(stored_ssid.c_str(), stored_password.c_str());
                delay(5000);
            }
        }
    }
}

bool WifiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WifiManager::getIP() {
    if (isAPMode) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
} 