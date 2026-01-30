#include "settingsweb.h"
#include <WiFi.h>
// No ESPAsyncWebserver or WebServer
// Libraries are fragile and didnt work with ESP32S3

// ------------------- Globals -------------------
WiFiServer* server = nullptr;
bool settingsInverted = false;

// QR Code globals
QRCode settingsQR;
uint8_t* settingsQRData = nullptr;

// ------------------- Forward Declarations -------------------
void drawSettingsScreen();
void handleClient();
String urlDecode(String str);

// ------------------- Hotspot -------------------
void startSettingsAP() {
    #if VERBOSE
        Serial.println("Starting AP...");
    #endif
    WiFi.mode(WIFI_AP);
    WiFi.softAP("aicuflow");
    IPAddress myIP = WiFi.softAPIP();
    #if VERBOSE
        Serial.printf("AP IP: %s\n", myIP.toString().c_str());
    #endif
}

// ------------------- Draw Instructions -------------------
void drawSettingsScreen() {
    uint32_t bg = settingsInverted ? TFT_WHITE : TFT_BLACK;
    uint32_t fg = settingsInverted ? TFT_BLACK : TFT_WHITE;

    tft.fillScreen(bg);
    tft.setTextColor(fg, bg);
    tft.setTextSize(2);

    int padding = 10;
    
    tft.setCursor(padding, 20);
    tft.println(en_de("Connect", "Verbinden"));
    
    tft.setTextSize(1);
    tft.setCursor(padding, 50);
    tft.println(en_de("1. Connect WiFi:", "1. Wlan verbinden"));
    tft.setCursor(padding, 65);
    tft.println("   aicuflow");
    
    tft.setCursor(padding, 90);
    tft.println(en_de("2. Open browser:", "2. Browser öffnen:"));
    tft.setCursor(padding, 105);
    IPAddress ip = WiFi.softAPIP();
    String ipStr = ip.toString();
    tft.println("   " + ipStr);
    
    // Draw QR code centered and as large as possible with padding
    if (!settingsQRData) {
        settingsQRData = (uint8_t*)malloc(qrcode_getBufferSize(3));
    }
    
    if (settingsQRData) {
        String qrURL = "http://" + ipStr;
        qrcode_initText(&settingsQR, settingsQRData, 3, 0, qrURL.c_str());
        
        // Calculate available space with padding on all sides
        int availableWidth = screenWidth - (padding * 2);
        int availableHeight = screenHeight - 125 - (padding * 2); // 125 is where text ends
        int maxQRSize = min(availableWidth, availableHeight);
        
        int scale = maxQRSize / settingsQR.size;
        if (scale < 1) scale = 1;
        int qrPixelSize = settingsQR.size * scale;
        
        // Center the QR code
        int xStart = (screenWidth - qrPixelSize) / 2;
        int yStart = 125 + ((screenHeight - 125 - qrPixelSize) / 2);
        
        for (uint8_t y = 0; y < settingsQR.size; y++) {
            for (uint8_t x = 0; x < settingsQR.size; x++) {
                uint32_t color = qrcode_getModule(&settingsQR, x, y) ? fg : bg;
                tft.fillRect(xStart + x * scale, yStart + y * scale, scale, scale, color);
            }
        }
    }
}

// ------------------- URL Decode -------------------
String urlDecode(String str) {
    String decoded = "";
    for (unsigned int i = 0; i < str.length(); i++) {
        if (str[i] == '+') {
            decoded += ' ';
        } else if (str[i] == '%' && i + 2 < str.length()) {
            char hex[3] = {str[i+1], str[i+2], '\0'};
            unsigned char decodedChar = (unsigned char)strtol(hex, NULL, 16);
            decoded += (char)decodedChar;
            i += 2;
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

// ------------------- Handle HTTP Client -------------------
void handleClient() {
    WiFiClient client = server->available();
    if (!client) return;

    String request = "";
    unsigned long timeout = millis() + 1000;
    
    while (client.connected() && millis() < timeout) {
        if (client.available()) {
            char c = client.read();
            request += c;
            if (request.endsWith("\r\n\r\n")) break;
        }
    }

    if (request.indexOf("POST /save") >= 0) {
        String body = "";
        while (client.available()) {
            body += (char)client.read();
        }

        int idx;
        if ((idx = body.indexOf("wlanSSID=")) >= 0) {
            int end = body.indexOf("&", idx);
            wlanSSID = urlDecode(body.substring(idx + 9, end > 0 ? end : body.length()));
        }
        if ((idx = body.indexOf("wlanPass=")) >= 0) {
            int end = body.indexOf("&", idx);
            wlanPass = urlDecode(body.substring(idx + 9, end > 0 ? end : body.length()));
        }
        if ((idx = body.indexOf("aicuMail=")) >= 0) {
            int end = body.indexOf("&", idx);
            aicuMail = urlDecode(body.substring(idx + 9, end > 0 ? end : body.length()));
        }
        if ((idx = body.indexOf("aicuPass=")) >= 0) {
            int end = body.indexOf("&", idx);
            aicuPass = urlDecode(body.substring(idx + 9, end > 0 ? end : body.length()));
        }
        if ((idx = body.indexOf("aicuFlow=")) >= 0) {
            int end = body.indexOf("&", idx);
            aicuFlow = urlDecode(body.substring(idx + 9, end > 0 ? end : body.length()));
        }
        if ((idx = body.indexOf("locale=")) >= 0) {
            int end = body.indexOf("&", idx);
            locale = urlDecode(body.substring(idx + 7, end > 0 ? end : body.length()));
        }
        if ((idx = body.indexOf("streamFileName=")) >= 0) {
            int end = body.indexOf("&", idx);
            streamFileName = urlDecode(body.substring(idx + 15, end > 0 ? end : body.length()));
        }
        if ((idx = body.indexOf("deviceName=")) >= 0) {
            String decodedName = urlDecode(body.substring(idx + 11, body.length()));
            // Limit device name to 12 characters
            if (decodedName.length() > 12) {
                decodedName = decodedName.substring(0, 12);
            }
            deviceName = decodedName;
        }

        saveSettings();
        applySettings();

        #if VERBOSE
            Serial.println("Settings saved");
        #endif

        // Send success page
        String html = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
        html += "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width'><style>";
        html += "body{font-family:Arial;margin:0;padding:0;background:#222;color:#eee;display:flex;justify-content:center;align-items:center;height:100vh;text-align:center;}";
        html += ".container{padding:40px;}";
        html += "h1{color:#0066cc;font-size:48px;margin:0 0 20px 0;}";
        html += "p{font-size:18px;line-height:1.6;margin:10px 0;}";
        html += "</style></head><body>";
        html += "<div class='container'>";
        html += "<h1>Settings Saved!</h1>";
        html += "<p>Device configuration has been updated.</p>";
        html += "<p>Setup Hotspot will be closed.</p>";
        html += "<p>You can now start measuring.</p>";
        html += "<p style='margin-top:30px;color:#888;font-size:14px;'>This page will close automatically.</p>";
        html += "</div>";
        html += "<script>setTimeout(function(){window.close();},3000);</script>";
        html += "</body></html>";
        
        client.print(html);
        client.stop();
        
        delay(500);
        toStartPage();
        
    } else {
        String html = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
        html += "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width'><style>";
        html += "body{font-family:Arial;margin:20px;background:#222;color:#eee;}";
        html += "input,select{width:100%;padding:8px;margin:5px 0;box-sizing:border-box;background:#333;color:#eee;border:1px solid #555;}";
        html += "button{width:100%;padding:12px;margin-top:10px;background:#0066cc;color:#fff;border:none;cursor:pointer;}";
        html += "button:hover{background:#0052a3;}";
        html += "label{display:block;margin-top:10px;font-weight:bold;}";
        html += ".optional{color:#888;font-size:12px;font-weight:normal;}";
        html += "hr{border:none;border-top:1px solid #444;margin:20px 0;}";
        html += ".char-count{font-size:11px;color:#888;margin-top:2px;}";
        html += "</style></head><body>";
        html += "<h2>Aicuflow IoT Device Setup</h2>";
        html += "<form method='post' action='/save' accept-charset='UTF-8'>";
        html += "<label>WiFi Name:</label>";
        html += "<input name='wlanSSID' value='" + wlanSSID + "' required><br>";
        html += "<label>WiFi Password:</label>";
        html += "<input name='wlanPass' type='password' value='" + wlanPass + "' required><br>";
        html += "<hr>";
        html += "<label>Email:</label>";
        html += "<input name='aicuMail' type='email' value='" + aicuMail + "'><br>";
        html += "<label>Password:</label>";
        html += "<input name='aicuPass' type='password' value='" + aicuPass + "'><br>";
        html += "<label>Flow ID:</label>";
        html += "<input name='aicuFlow' value='" + aicuFlow + "'><br>";
        html += "<hr>";
        html += "<label>Device language: <span class='optional'>(optional)</span></label>";
        html += "<select name='locale'>";
        html += "<option value='en'" + String(locale == "en" ? " selected" : "") + ">English</option>";
        html += "<option value='de'" + String(locale == "de" ? " selected" : "") + ">Deutsch</option>";
        html += "</select><br>";
        html += "<label>Filename written to: <span class='optional'>(optional)</span></label>";
        html += "<input name='streamFileName' value='" + streamFileName + "'><br>";
        html += "<label>Device Name: <span class='optional'>(max 12 chars)</span></label>";
        html += "<input name='deviceName' id='deviceName' value='" + deviceName + "' maxlength='12'>";
        html += "<div class='char-count' id='charCount'>0/12</div>";
        html += "<button type='submit'>SAVE</button>";
        html += "</form>";
        html += "<script>";
        html += "const input=document.getElementById('deviceName');";
        html += "const counter=document.getElementById('charCount');";
        html += "function updateCount(){counter.textContent=input.value.length+'/12';}";
        html += "input.addEventListener('input',updateCount);";
        html += "updateCount();";
        html += "</script>";
        html += "</body></html>";
        
        client.print(html);
        client.stop();
    }
}

// ------------------- App Entry Points -------------------
void onSettingsWebPageOpen() {
    static bool started = false;
    if(!started) {
        startSettingsAP();
        server = new WiFiServer(80);
        server->begin();
        #if VERBOSE
            Serial.println("Server started");
        #endif
        started = true;
    }
    drawSettingsScreen();
}

void onSettingsWebPageUpdate() {
    if(server) handleClient();
    
    static unsigned long lastPress = 0;
    static bool leftWasLow = false;
    static bool rightWasLow = false;
    
    unsigned long now = millis();
    bool leftIsLow = digitalRead(LEFT_BUTTON) == LOW;
    bool rightIsLow = digitalRead(RIGHT_BUTTON) == LOW;

    if (now - lastPress > 200) {
        // Trigger on button press (transition to LOW)
        if (leftIsLow && !leftWasLow) {
            settingsInverted = !settingsInverted;
            drawSettingsScreen();
            lastPress = now;
        }

        if (rightIsLow && !rightWasLow) {
            closePage();
            lastPress = now;
        }
    }
    
    leftWasLow = leftIsLow;
    rightWasLow = rightIsLow;
}