#include "colortest.h"
#include "gol.h"

// Grid variables
const int golCellSize = 4;
int golGridWidth, golGridHeight;
bool** golGrid = nullptr;
bool** golNextGrid = nullptr;

unsigned long golLastUpdateTime = 0;
const int golUpdateDelay = 100; // ms between generations

int golCurrentPattern = 0;
bool golLeftButtonWasPressed = false;
bool golRightButtonWasPressed = false;
bool golGridsAllocated = false;

const uint16_t golAliveColor = rgb565(0, 255, 0);  // Green
const uint16_t golDeadColor = rgb565(0, 0, 0);     // Black

void golAllocateGrids() {
  if (golGridsAllocated) return;
  
  golGridWidth = screenWidth / golCellSize;
  golGridHeight = screenHeight / golCellSize;
  
  golGrid = new bool*[golGridWidth];
  golNextGrid = new bool*[golGridWidth];
  for (int golX = 0; golX < golGridWidth; golX++) {
    golGrid[golX] = new bool[golGridHeight];
    golNextGrid[golX] = new bool[golGridHeight];
  }
  golGridsAllocated = true;
}

void golFreeGrids() {
  if (!golGridsAllocated) return;
  
  for (int golX = 0; golX < golGridWidth; golX++) {
    delete[] golGrid[golX];
    delete[] golNextGrid[golX];
  }
  delete[] golGrid;
  delete[] golNextGrid;
  
  golGrid = nullptr;
  golNextGrid = nullptr;
  golGridsAllocated = false;
}

void golClearGrid() {
  for (int golX = 0; golX < golGridWidth; golX++) {
    for (int golY = 0; golY < golGridHeight; golY++) {
      golGrid[golX][golY] = false;
    }
  }
}

void golSetCell(int golX, int golY, bool golState) {
  if (golX >= 0 && golX < golGridWidth && golY >= 0 && golY < golGridHeight) {
    golGrid[golX][golY] = golState;
  }
}

// Pattern: Glider Gun (Gosper's)
void golLoadGliderGun() {
  golClearGrid();
  int golOffsetX = 5;
  int golOffsetY = 5;
  
  // Left square
  golSetCell(golOffsetX + 0, golOffsetY + 4, true);
  golSetCell(golOffsetX + 0, golOffsetY + 5, true);
  golSetCell(golOffsetX + 1, golOffsetY + 4, true);
  golSetCell(golOffsetX + 1, golOffsetY + 5, true);
  
  // Left part
  golSetCell(golOffsetX + 10, golOffsetY + 4, true);
  golSetCell(golOffsetX + 10, golOffsetY + 5, true);
  golSetCell(golOffsetX + 10, golOffsetY + 6, true);
  golSetCell(golOffsetX + 11, golOffsetY + 3, true);
  golSetCell(golOffsetX + 11, golOffsetY + 7, true);
  golSetCell(golOffsetX + 12, golOffsetY + 2, true);
  golSetCell(golOffsetX + 12, golOffsetY + 8, true);
  golSetCell(golOffsetX + 13, golOffsetY + 2, true);
  golSetCell(golOffsetX + 13, golOffsetY + 8, true);
  golSetCell(golOffsetX + 14, golOffsetY + 5, true);
  golSetCell(golOffsetX + 15, golOffsetY + 3, true);
  golSetCell(golOffsetX + 15, golOffsetY + 7, true);
  golSetCell(golOffsetX + 16, golOffsetY + 4, true);
  golSetCell(golOffsetX + 16, golOffsetY + 5, true);
  golSetCell(golOffsetX + 16, golOffsetY + 6, true);
  golSetCell(golOffsetX + 17, golOffsetY + 5, true);
  
  // Right part
  golSetCell(golOffsetX + 20, golOffsetY + 2, true);
  golSetCell(golOffsetX + 20, golOffsetY + 3, true);
  golSetCell(golOffsetX + 20, golOffsetY + 4, true);
  golSetCell(golOffsetX + 21, golOffsetY + 2, true);
  golSetCell(golOffsetX + 21, golOffsetY + 3, true);
  golSetCell(golOffsetX + 21, golOffsetY + 4, true);
  golSetCell(golOffsetX + 22, golOffsetY + 1, true);
  golSetCell(golOffsetX + 22, golOffsetY + 5, true);
  golSetCell(golOffsetX + 24, golOffsetY + 0, true);
  golSetCell(golOffsetX + 24, golOffsetY + 1, true);
  golSetCell(golOffsetX + 24, golOffsetY + 5, true);
  golSetCell(golOffsetX + 24, golOffsetY + 6, true);
  
  // Right square
  golSetCell(golOffsetX + 34, golOffsetY + 2, true);
  golSetCell(golOffsetX + 34, golOffsetY + 3, true);
  golSetCell(golOffsetX + 35, golOffsetY + 2, true);
  golSetCell(golOffsetX + 35, golOffsetY + 3, true);
}

// Pattern: Pulsar
void golLoadPulsar() {
  golClearGrid();
  int golCx = golGridWidth / 2;
  int golCy = golGridHeight / 2;
  
  // Top
  for (int golI = -6; golI <= -4; golI++) {
    golSetCell(golCx + golI, golCy - 12, true);
    golSetCell(golCx + golI, golCy - 7, true);
  }
  for (int golI = 4; golI <= 6; golI++) {
    golSetCell(golCx + golI, golCy - 12, true);
    golSetCell(golCx + golI, golCy - 7, true);
  }
  
  // Bottom
  for (int golI = -6; golI <= -4; golI++) {
    golSetCell(golCx + golI, golCy + 12, true);
    golSetCell(golCx + golI, golCy + 7, true);
  }
  for (int golI = 4; golI <= 6; golI++) {
    golSetCell(golCx + golI, golCy + 12, true);
    golSetCell(golCx + golI, golCy + 7, true);
  }
  
  // Left
  for (int golI = -6; golI <= -4; golI++) {
    golSetCell(golCx - 12, golCy + golI, true);
    golSetCell(golCx - 7, golCy + golI, true);
  }
  for (int golI = 4; golI <= 6; golI++) {
    golSetCell(golCx - 12, golCy + golI, true);
    golSetCell(golCx - 7, golCy + golI, true);
  }
  
  // Right
  for (int golI = -6; golI <= -4; golI++) {
    golSetCell(golCx + 12, golCy + golI, true);
    golSetCell(golCx + 7, golCy + golI, true);
  }
  for (int golI = 4; golI <= 6; golI++) {
    golSetCell(golCx + 12, golCy + golI, true);
    golSetCell(golCx + 7, golCy + golI, true);
  }
}

// Pattern: Pentadecathlon
void golLoadPentadecathlon() {
  golClearGrid();
  int golCx = golGridWidth / 2;
  int golCy = golGridHeight / 2;
  
  golSetCell(golCx, golCy - 4, true);
  golSetCell(golCx, golCy - 3, true);
  golSetCell(golCx - 1, golCy - 2, true);
  golSetCell(golCx + 1, golCy - 2, true);
  golSetCell(golCx, golCy - 1, true);
  golSetCell(golCx, golCy, true);
  golSetCell(golCx, golCy + 1, true);
  golSetCell(golCx, golCy + 2, true);
  golSetCell(golCx - 1, golCy + 3, true);
  golSetCell(golCx + 1, golCy + 3, true);
  golSetCell(golCx, golCy + 4, true);
  golSetCell(golCx, golCy + 5, true);
}

// Pattern: Acorn
void golLoadAcorn() {
  golClearGrid();
  int golCx = golGridWidth / 2;
  int golCy = golGridHeight / 2;
  
  golSetCell(golCx - 1, golCy, true);
  golSetCell(golCx + 1, golCy + 1, true);
  golSetCell(golCx - 2, golCy + 2, true);
  golSetCell(golCx - 1, golCy + 2, true);
  golSetCell(golCx + 2, golCy + 2, true);
  golSetCell(golCx + 3, golCy + 2, true);
  golSetCell(golCx + 4, golCy + 2, true);
}

// Pattern: Lightweight Spaceship (LWSS) fleet
void golLoadSpaceshipFleet() {
  golClearGrid();
  
  for (int golI = 0; golI < 3; golI++) {
    int golOffsetX = 10 + golI * 15;
    int golOffsetY = 10 + golI * 8;
    
    golSetCell(golOffsetX + 1, golOffsetY + 0, true);
    golSetCell(golOffsetX + 4, golOffsetY + 0, true);
    golSetCell(golOffsetX + 0, golOffsetY + 1, true);
    golSetCell(golOffsetX + 0, golOffsetY + 2, true);
    golSetCell(golOffsetX + 4, golOffsetY + 2, true);
    golSetCell(golOffsetX + 0, golOffsetY + 3, true);
    golSetCell(golOffsetX + 1, golOffsetY + 3, true);
    golSetCell(golOffsetX + 2, golOffsetY + 3, true);
    golSetCell(golOffsetX + 3, golOffsetY + 3, true);
  }
}

// Pattern: Random
void golLoadRandom() {
  golClearGrid();
  randomSeed(millis());
  for (int golX = 0; golX < golGridWidth; golX++) {
    for (int golY = 0; golY < golGridHeight; golY++) {
      golGrid[golX][golY] = random(0, 100) < 30; // 30% chance of being alive
    }
  }
}

void golLoadPattern(int golPatternIndex) {
  switch(golPatternIndex) {
    case 0: golLoadGliderGun(); break;
    case 1: golLoadPulsar(); break;
    case 2: golLoadPentadecathlon(); break;
    case 3: golLoadAcorn(); break;
    case 4: golLoadSpaceshipFleet(); break;
    default: golLoadRandom(); break;
  }
}

void golDrawCell(int golX, int golY, uint16_t golColor) {
  tft.fillRect(golX * golCellSize, golY * golCellSize, golCellSize, golCellSize, golColor);
}

void golDrawGrid() {
  for (int golX = 0; golX < golGridWidth; golX++) {
    for (int golY = 0; golY < golGridHeight; golY++) {
      golDrawCell(golX, golY, golGrid[golX][golY] ? golAliveColor : golDeadColor);
    }
  }
}

int golCountNeighbors(int golX, int golY) {
  int golCount = 0;
  for (int golDx = -1; golDx <= 1; golDx++) {
    for (int golDy = -1; golDy <= 1; golDy++) {
      if (golDx == 0 && golDy == 0) continue;
      
      int golNx = golX + golDx;
      int golNy = golY + golDy;
      
      if (golNx >= 0 && golNx < golGridWidth && golNy >= 0 && golNy < golGridHeight) {
        if (golGrid[golNx][golNy]) golCount++;
      }
    }
  }
  return golCount;
}

void golUpdateLife() {
  // Calculate next generation
  for (int golX = 0; golX < golGridWidth; golX++) {
    for (int golY = 0; golY < golGridHeight; golY++) {
      int golNeighbors = golCountNeighbors(golX, golY);
      
      if (golGrid[golX][golY]) {
        // Cell is alive
        golNextGrid[golX][golY] = (golNeighbors == 2 || golNeighbors == 3);
      } else {
        // Cell is dead
        golNextGrid[golX][golY] = (golNeighbors == 3);
      }
    }
  }
  
  // Update grid and redraw changed cells
  for (int golX = 0; golX < golGridWidth; golX++) {
    for (int golY = 0; golY < golGridHeight; golY++) {
      if (golGrid[golX][golY] != golNextGrid[golX][golY]) {
        golGrid[golX][golY] = golNextGrid[golX][golY];
        golDrawCell(golX, golY, golGrid[golX][golY] ? golAliveColor : golDeadColor);
      }
    }
  }
}

void onGameOfLifePageOpen() {
  tft.fillScreen(golDeadColor);
  golAllocateGrids();
  golCurrentPattern = 0;
  golLoadPattern(golCurrentPattern);
  golDrawGrid();
  golLeftButtonWasPressed = false;
  golRightButtonWasPressed = false;
}

void onGameOfLifePageUpdate() {
  bool golLeftIsDown = (digitalRead(LEFT_BUTTON) == LOW);
  bool golRightIsDown = (digitalRead(RIGHT_BUTTON) == LOW);
  
  // Left button: cycle through patterns
  if (golLeftButtonWasPressed && !golLeftIsDown) {
    golCurrentPattern++;
    golLoadPattern(golCurrentPattern);
    golDrawGrid();
  }
  
  // Right button: exit - FREE GRIDS BEFORE CLOSING!
  if (golRightButtonWasPressed && !golRightIsDown) {
    golFreeGrids();
    closePageIfAnyButtonIsPressed();
    return;
  }
  
  golLeftButtonWasPressed = golLeftIsDown;
  golRightButtonWasPressed = golRightIsDown;
  
  // Update simulation
  if (millis() - golLastUpdateTime > golUpdateDelay) {
    golLastUpdateTime = millis();
    golUpdateLife();
  }
}