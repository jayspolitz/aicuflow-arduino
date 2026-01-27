#include "create_flow.h"

static String aicuFlowCreatedPageFlowName = "";
static bool aicuFlowCreatedPageDone = false;

String setupArduinoFlow() {
    tft.setCursor(0, 0); // reset needed
    connectWifiOrTimeout();
    connectAPI();
    
    DynamicJsonDocument flows(8192);
    if (!aicu.listFlows(flows)) {
        Serial.println("listFlows failed");
        return "";
    }

    int maxIndex = -1;
    const char* baseName = "aicuflow-arduino";

    JsonArray data = flows.as<JsonArray>();
    for (JsonObject f : data) {
        const char* name = f["name"];
        if (!name) continue;

        String s(name);
        if (!s.startsWith(baseName)) continue;

        if (s == baseName) {
            maxIndex = max(maxIndex, 0);
        } else if (s.startsWith(String(baseName) + "-")) {
            int idx = s.substring(strlen(baseName) + 1).toInt();
            maxIndex = max(maxIndex, idx);
        }
    }

    String newName;
    if (maxIndex < 0) {
        newName = baseName;
    } else {
        newName = String(baseName) + "-" + String(maxIndex + 1);
    }

    Serial.print("Creating flow: ");
    Serial.println(newName);

    DynamicJsonDocument created(2048);
    if (!aicu.createFlow(newName.c_str(), "auto-created from arduino setup", created)) {
        Serial.println("createFlow failed");
        return "";
    }

    return created["data"]["id"].as<String>();
}

void onCreateFlowPageOpen() {
  aicuFlowCreatedPageDone = false;
  aicuFlowCreatedPageFlowName = "";

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  aicuFlowCreatedPageFlowName = setupArduinoFlow();

  if (aicuFlowCreatedPageFlowName.length() == 0) {
    tft.setTextSize(2);
    tft.drawString(
      en_de("Flow creation failed", "Flow Erstellung fehlgeschlagen"),
      screenWidth / 2,
      screenHeight / 2
    );
    return;
  }

  int centerY = screenHeight / 2;

  tft.setTextSize(screenHeight < 120 ? 1 : 2);
  tft.drawString(
    en_de("Flow created", "Flow erstellt"),
    screenWidth / 2,
    centerY - (screenHeight < 120 ? 10 : 18)
  );

  tft.setTextSize(1);
  tft.drawString(
    aicuFlowCreatedPageFlowName,
    screenWidth / 2,
    centerY + (screenHeight < 120 ? 6 : 14)
  );
  
  aicuFlow = aicuFlowCreatedPageFlowName;
  saveSettings();

  tft.drawString(
    en_de("flow connected", "Flow verbunden"),
    screenWidth / 2,
    centerY + (screenHeight < 120 ? 6+16 : 14+16)
  );

  aicuFlowCreatedPageDone = true;
}

void onCreateFlowPageUpdate() {
  if (!aicuFlowCreatedPageDone) return;
  closePageIfAnyButtonIsPressed();
}