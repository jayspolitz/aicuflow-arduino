#include "mandelbrot.h"
#include "colortest.h"

const int mandelMaxIterations = 128;
bool mandelDrawn = false;

// Mandelbrot set bounds
double mandelMinReal = -2.5;
double mandelMaxReal = 1.0;
double mandelMinImag = -1.25;
double mandelMaxImag = 1.25;

// Sub-pixel sampling offsets for RGB channels (creates chromatic effect)
const float mandelRedOffsetX = -0.33;
const float mandelRedOffsetY = 0.0;
const float mandelGreenOffsetX = 0.0;
const float mandelGreenOffsetY = 0.0;
const float mandelBlueOffsetX = 0.33;
const float mandelBlueOffsetY = 0.0;

int mandelCalculateIterations(double mandelCReal, double mandelCImag) {
  double mandelZReal = 0.0;
  double mandelZImag = 0.0;
  int mandelIter = 0;
  
  while (mandelIter < mandelMaxIterations) {
    double mandelZReal2 = mandelZReal * mandelZReal;
    double mandelZImag2 = mandelZImag * mandelZImag;
    
    if (mandelZReal2 + mandelZImag2 > 4.0) {
      break;
    }
    
    double mandelTemp = mandelZReal2 - mandelZImag2 + mandelCReal;
    mandelZImag = 2.0 * mandelZReal * mandelZImag + mandelCImag;
    mandelZReal = mandelTemp;
    
    mandelIter++;
  }
  
  return mandelIter;
}

uint8_t mandelIterationsToColor(int mandelIter) {
  if (mandelIter >= mandelMaxIterations) {
    return 0;
  }
  
  // Smooth color gradient
  float mandelT = (float)mandelIter / mandelMaxIterations;
  mandelT = sqrt(mandelT); // Non-linear mapping for better distribution
  
  return (uint8_t)(mandelT * 255);
}

void mandelRenderRow(int mandelY) {
  double mandelRangeReal = mandelMaxReal - mandelMinReal;
  double mandelRangeImag = mandelMaxImag - mandelMinImag;
  
  for (int mandelX = 0; mandelX < screenWidth; mandelX++) {
    // Calculate positions for each color channel with sub-pixel offsets
    double mandelBaseReal = mandelMinReal + (mandelX / (double)screenWidth) * mandelRangeReal;
    double mandelBaseImag = mandelMinImag + (mandelY / (double)screenHeight) * mandelRangeImag;
    
    double mandelPixelWidth = mandelRangeReal / screenWidth;
    double mandelPixelHeight = mandelRangeImag / screenHeight;
    
    // Red channel
    double mandelRedReal = mandelBaseReal + mandelRedOffsetX * mandelPixelWidth;
    double mandelRedImag = mandelBaseImag + mandelRedOffsetY * mandelPixelHeight;
    int mandelRedIter = mandelCalculateIterations(mandelRedReal, mandelRedImag);
    uint8_t mandelRedValue = mandelIterationsToColor(mandelRedIter);
    
    // Green channel
    double mandelGreenReal = mandelBaseReal + mandelGreenOffsetX * mandelPixelWidth;
    double mandelGreenImag = mandelBaseImag + mandelGreenOffsetY * mandelPixelHeight;
    int mandelGreenIter = mandelCalculateIterations(mandelGreenReal, mandelGreenImag);
    uint8_t mandelGreenValue = mandelIterationsToColor(mandelGreenIter);
    
    // Blue channel
    double mandelBlueReal = mandelBaseReal + mandelBlueOffsetX * mandelPixelWidth;
    double mandelBlueImag = mandelBaseImag + mandelBlueOffsetY * mandelPixelHeight;
    int mandelBlueIter = mandelCalculateIterations(mandelBlueReal, mandelBlueImag);
    uint8_t mandelBlueValue = mandelIterationsToColor(mandelBlueIter);
    
    // Enhanced color mapping for more vibrant results
    mandelRedValue = (mandelRedValue * 3) % 256;
    mandelGreenValue = (mandelGreenValue * 2) % 256;
    mandelBlueValue = (mandelBlueValue * 4) % 256;
    
    uint16_t mandelPixelColor = rgb565(mandelRedValue, mandelGreenValue, mandelBlueValue);
    tft.drawPixel(mandelX, mandelY, mandelPixelColor);
  }
}

void onMandelbrotPageOpen() {
  tft.fillScreen(TFT_BLACK);
  mandelDrawn = false;
}

void onMandelbrotPageUpdate() {
  if (!mandelDrawn) {
    // Progressive rendering - render one row at a time to keep UI responsive
    static int mandelCurrentRow = 0;
    
    if (mandelCurrentRow < screenHeight) {
      mandelRenderRow(mandelCurrentRow);
      mandelCurrentRow++;
    } else {
      mandelDrawn = true;
      mandelCurrentRow = 0;
    }
  }
  
  closePageIfAnyButtonIsPressed();
}