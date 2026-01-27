#include "serialmonitor.h"

int lastSerialCheckTime = 0;
int serialCheckInterval = 50; // check every 50ms for responsiveness
String serialBuffer = "";
int scrollOffset = 0;
int lineHeight = 10;
int maxVisibleLines = 0;
std::vector<String> serialLines;
const int MAX_LINES = 100; // Keep last 100 lines in memory

void onSerialMonitorPageOpen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  
  // Calculate max visible lines based on screen height
  maxVisibleLines = (screenHeight / lineHeight) - 1;
  
  // Clear any existing data
  serialLines.clear();
  serialBuffer = "";
  scrollOffset = 0;
  
  // Draw header
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.fillRect(0, 0, screenWidth, lineHeight, TFT_BLUE);
  tft.setCursor(2, 1);
  tft.print(en_de("Serial Monitor", "Serieller Mon."));
  
  // Set text color for content
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  // Initialize Serial if not already
  if (!Serial) {
    Serial.begin(115200);
  }
}

void drawSerialMonitorContent() {
  // Clear content area (preserve header)
  tft.fillRect(0, lineHeight, screenWidth, screenHeight - lineHeight, TFT_BLACK);
  
  // Calculate how many lines we can show
  int startLine = max(0, (int)serialLines.size() - maxVisibleLines - scrollOffset);
  int endLine = min((int)serialLines.size(), startLine + maxVisibleLines);
  
  // Draw visible lines
  for (int i = startLine; i < endLine; i++) {
    int yPos = lineHeight + ((i - startLine) * lineHeight);
    tft.setCursor(2, yPos + 1);
    
    // Truncate line if too long for screen
    String line = serialLines[i];
    int maxChars = (screenWidth - 4) / 6; // Approximate char width
    if (line.length() > maxChars) {
      line = line.substring(0, maxChars - 3) + "...";
    }
    
    tft.print(line);
  }
  
  // Draw scroll indicator if there are more lines
  if (serialLines.size() > maxVisibleLines) {
    int indicatorHeight = max(10, (maxVisibleLines * screenHeight) / serialLines.size());
    int indicatorY = lineHeight + ((scrollOffset * (screenHeight - lineHeight - indicatorHeight)) / 
                     max(1, (int)serialLines.size() - maxVisibleLines));
    
    tft.fillRect(screenWidth - 3, lineHeight, 3, screenHeight - lineHeight, TFT_DARKGREY);
    tft.fillRect(screenWidth - 3, indicatorY, 3, indicatorHeight, TFT_WHITE);
  }
}

void onSerialMonitorPageUpdate() {
  bool needsRedraw = false;
  
  // Check for serial data
  while (Serial.available()) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 0) {
        // Add line to buffer
        serialLines.push_back(serialBuffer);
        serialBuffer = "";
        needsRedraw = true;
        
        // Keep only MAX_LINES in memory
        if (serialLines.size() > MAX_LINES) {
          serialLines.erase(serialLines.begin());
        }
        
        // Auto-scroll to bottom
        scrollOffset = 0;
      }
    } else if (c >= 32 || c == '\t') { // Printable characters and tab
      serialBuffer += c;
      
      // If buffer gets too long, force a line break
      if (serialBuffer.length() > 200) {
        serialLines.push_back(serialBuffer);
        serialBuffer = "";
        needsRedraw = true;
        
        if (serialLines.size() > MAX_LINES) {
          serialLines.erase(serialLines.begin());
        }
      }
    }
  }
  
  // Handle button input for scrolling
  static unsigned long lastButtonPress = 0;
  unsigned long now = millis();
  
  if (now - lastButtonPress > 150) { // Debounce
    if (digitalRead(LEFT_BUTTON) == LOW) {
      // Scroll up
      if (scrollOffset < serialLines.size() - maxVisibleLines) {
        scrollOffset++;
        needsRedraw = true;
      }
      lastButtonPress = now;
    }
    
    if (digitalRead(RIGHT_BUTTON) == LOW) {
      // Scroll down / return to main menu
      if (scrollOffset > 0) {
        scrollOffset--;
        needsRedraw = true;
        lastButtonPress = now;
      } else {
        // If at bottom, close on right button
        closePageIfAnyButtonIsPressed();
        return;
      }
    }
  }
  
  // Redraw if needed
  if (needsRedraw) {
    drawSerialMonitorContent();
  }
  
  delay(10); // Small delay to prevent excessive CPU usage
}