#include "wireframe3d.h"
#include "colortest.h"
#include <math.h>

// 3D rendering variables
struct d3Vertex {
  float x, y, z;
};

struct d3Edge {
  int v1, v2;
};

struct d3Shape {
  d3Vertex* vertices;
  int vertexCount;
  d3Edge* edges;
  int edgeCount;
};

d3Shape d3CurrentShape;
int d3CurrentShapeIndex = 0;
float d3RotationAngle = 0.0;
bool d3ShowHiddenEdges = true; // Changed to true by default
bool d3LeftButtonWasPressed = false;
bool d3RightButtonWasPressed = false;

const float d3CameraDistance = 500.0; // Moved camera much further back
const float d3CameraTiltX = 0.3; // Tilt looking slightly down
const float d3CameraTiltY = 0.2; // Slight side tilt for better view
const float d3RotationSpeed = 0.03;
const float d3Scale = 50.0; // Overall size scale
const uint16_t d3WireframeColor = rgb565(0, 255, 100);
const uint16_t d3BackgroundColor = rgb565(0, 0, 0);

void d3FreeShape() {
  if (d3CurrentShape.vertices) delete[] d3CurrentShape.vertices;
  if (d3CurrentShape.edges) delete[] d3CurrentShape.edges;
  d3CurrentShape.vertices = nullptr;
  d3CurrentShape.edges = nullptr;
}

// Create a cube
void d3CreateCube() {
  d3FreeShape();
  
  d3CurrentShape.vertexCount = 8;
  d3CurrentShape.vertices = new d3Vertex[8];
  
  d3CurrentShape.vertices[0] = {-1, -1, -1};
  d3CurrentShape.vertices[1] = { 1, -1, -1};
  d3CurrentShape.vertices[2] = { 1,  1, -1};
  d3CurrentShape.vertices[3] = {-1,  1, -1};
  d3CurrentShape.vertices[4] = {-1, -1,  1};
  d3CurrentShape.vertices[5] = { 1, -1,  1};
  d3CurrentShape.vertices[6] = { 1,  1,  1};
  d3CurrentShape.vertices[7] = {-1,  1,  1};
  
  d3CurrentShape.edgeCount = 12;
  d3CurrentShape.edges = new d3Edge[12];
  d3CurrentShape.edges[0] = {0, 1}; d3CurrentShape.edges[1] = {1, 2};
  d3CurrentShape.edges[2] = {2, 3}; d3CurrentShape.edges[3] = {3, 0};
  d3CurrentShape.edges[4] = {4, 5}; d3CurrentShape.edges[5] = {5, 6};
  d3CurrentShape.edges[6] = {6, 7}; d3CurrentShape.edges[7] = {7, 4};
  d3CurrentShape.edges[8] = {0, 4}; d3CurrentShape.edges[9] = {1, 5};
  d3CurrentShape.edges[10] = {2, 6}; d3CurrentShape.edges[11] = {3, 7};
}

// Create an octahedron
void d3CreateOctahedron() {
  d3FreeShape();
  
  d3CurrentShape.vertexCount = 6;
  d3CurrentShape.vertices = new d3Vertex[6];
  
  d3CurrentShape.vertices[0] = { 0,  1.5,  0};
  d3CurrentShape.vertices[1] = { 1,  0,  0};
  d3CurrentShape.vertices[2] = { 0,  0,  1};
  d3CurrentShape.vertices[3] = {-1,  0,  0};
  d3CurrentShape.vertices[4] = { 0,  0, -1};
  d3CurrentShape.vertices[5] = { 0, -1.5,  0};
  
  d3CurrentShape.edgeCount = 12;
  d3CurrentShape.edges = new d3Edge[12];
  d3CurrentShape.edges[0] = {0, 1}; d3CurrentShape.edges[1] = {0, 2};
  d3CurrentShape.edges[2] = {0, 3}; d3CurrentShape.edges[3] = {0, 4};
  d3CurrentShape.edges[4] = {1, 2}; d3CurrentShape.edges[5] = {2, 3};
  d3CurrentShape.edges[6] = {3, 4}; d3CurrentShape.edges[7] = {4, 1};
  d3CurrentShape.edges[8] = {5, 1}; d3CurrentShape.edges[9] = {5, 2};
  d3CurrentShape.edges[10] = {5, 3}; d3CurrentShape.edges[11] = {5, 4};
}

// Create an icosahedron
void d3CreateIcosahedron() {
  d3FreeShape();
  
  d3CurrentShape.vertexCount = 12;
  d3CurrentShape.vertices = new d3Vertex[12];
  
  float d3Phi = (1.0 + sqrt(5.0)) / 2.0;
  
  d3CurrentShape.vertices[0] = {-1,  d3Phi,  0};
  d3CurrentShape.vertices[1] = { 1,  d3Phi,  0};
  d3CurrentShape.vertices[2] = {-1, -d3Phi,  0};
  d3CurrentShape.vertices[3] = { 1, -d3Phi,  0};
  d3CurrentShape.vertices[4] = { 0, -1,  d3Phi};
  d3CurrentShape.vertices[5] = { 0,  1,  d3Phi};
  d3CurrentShape.vertices[6] = { 0, -1, -d3Phi};
  d3CurrentShape.vertices[7] = { 0,  1, -d3Phi};
  d3CurrentShape.vertices[8] = { d3Phi,  0, -1};
  d3CurrentShape.vertices[9] = { d3Phi,  0,  1};
  d3CurrentShape.vertices[10] = {-d3Phi,  0, -1};
  d3CurrentShape.vertices[11] = {-d3Phi,  0,  1};
  
  d3CurrentShape.edgeCount = 30;
  d3CurrentShape.edges = new d3Edge[30];
  d3CurrentShape.edges[0] = {0, 11}; d3CurrentShape.edges[1] = {0, 5};
  d3CurrentShape.edges[2] = {0, 1}; d3CurrentShape.edges[3] = {0, 7};
  d3CurrentShape.edges[4] = {0, 10}; d3CurrentShape.edges[5] = {1, 5};
  d3CurrentShape.edges[6] = {1, 7}; d3CurrentShape.edges[7] = {1, 9};
  d3CurrentShape.edges[8] = {1, 8}; d3CurrentShape.edges[9] = {2, 3};
  d3CurrentShape.edges[10] = {2, 6}; d3CurrentShape.edges[11] = {2, 10};
  d3CurrentShape.edges[12] = {2, 11}; d3CurrentShape.edges[13] = {2, 4};
  d3CurrentShape.edges[14] = {3, 4}; d3CurrentShape.edges[15] = {3, 6};
  d3CurrentShape.edges[16] = {3, 8}; d3CurrentShape.edges[17] = {3, 9};
  d3CurrentShape.edges[18] = {4, 5}; d3CurrentShape.edges[19] = {4, 9};
  d3CurrentShape.edges[20] = {4, 11}; d3CurrentShape.edges[21] = {5, 9};
  d3CurrentShape.edges[22] = {5, 11}; d3CurrentShape.edges[23] = {6, 7};
  d3CurrentShape.edges[24] = {6, 8}; d3CurrentShape.edges[25] = {6, 10};
  d3CurrentShape.edges[26] = {7, 8}; d3CurrentShape.edges[27] = {7, 10};
  d3CurrentShape.edges[28] = {8, 9}; d3CurrentShape.edges[29] = {10, 11};
}

// Create a dodecahedron
void d3CreateDodecahedron() {
  d3FreeShape();
  
  d3CurrentShape.vertexCount = 20;
  d3CurrentShape.vertices = new d3Vertex[20];
  
  float d3Phi = (1.0 + sqrt(5.0)) / 2.0;
  float d3InvPhi = 1.0 / d3Phi;
  
  d3CurrentShape.vertices[0] = { 1,  1,  1};
  d3CurrentShape.vertices[1] = { 1,  1, -1};
  d3CurrentShape.vertices[2] = { 1, -1,  1};
  d3CurrentShape.vertices[3] = { 1, -1, -1};
  d3CurrentShape.vertices[4] = {-1,  1,  1};
  d3CurrentShape.vertices[5] = {-1,  1, -1};
  d3CurrentShape.vertices[6] = {-1, -1,  1};
  d3CurrentShape.vertices[7] = {-1, -1, -1};
  d3CurrentShape.vertices[8] = { 0,  d3InvPhi,  d3Phi};
  d3CurrentShape.vertices[9] = { 0,  d3InvPhi, -d3Phi};
  d3CurrentShape.vertices[10] = { 0, -d3InvPhi,  d3Phi};
  d3CurrentShape.vertices[11] = { 0, -d3InvPhi, -d3Phi};
  d3CurrentShape.vertices[12] = { d3InvPhi,  d3Phi,  0};
  d3CurrentShape.vertices[13] = { d3InvPhi, -d3Phi,  0};
  d3CurrentShape.vertices[14] = {-d3InvPhi,  d3Phi,  0};
  d3CurrentShape.vertices[15] = {-d3InvPhi, -d3Phi,  0};
  d3CurrentShape.vertices[16] = { d3Phi,  0,  d3InvPhi};
  d3CurrentShape.vertices[17] = { d3Phi,  0, -d3InvPhi};
  d3CurrentShape.vertices[18] = {-d3Phi,  0,  d3InvPhi};
  d3CurrentShape.vertices[19] = {-d3Phi,  0, -d3InvPhi};
  
  d3CurrentShape.edgeCount = 30;
  d3CurrentShape.edges = new d3Edge[30];
  d3CurrentShape.edges[0] = {0, 8}; d3CurrentShape.edges[1] = {0, 12};
  d3CurrentShape.edges[2] = {0, 16}; d3CurrentShape.edges[3] = {1, 9};
  d3CurrentShape.edges[4] = {1, 12}; d3CurrentShape.edges[5] = {1, 17};
  d3CurrentShape.edges[6] = {2, 8}; d3CurrentShape.edges[7] = {2, 10};
  d3CurrentShape.edges[8] = {2, 16}; d3CurrentShape.edges[9] = {3, 11};
  d3CurrentShape.edges[10] = {3, 13}; d3CurrentShape.edges[11] = {3, 17};
  d3CurrentShape.edges[12] = {4, 8}; d3CurrentShape.edges[13] = {4, 14};
  d3CurrentShape.edges[14] = {4, 18}; d3CurrentShape.edges[15] = {5, 9};
  d3CurrentShape.edges[16] = {5, 14}; d3CurrentShape.edges[17] = {5, 19};
  d3CurrentShape.edges[18] = {6, 10}; d3CurrentShape.edges[19] = {6, 15};
  d3CurrentShape.edges[20] = {6, 18}; d3CurrentShape.edges[21] = {7, 11};
  d3CurrentShape.edges[22] = {7, 15}; d3CurrentShape.edges[23] = {7, 19};
  d3CurrentShape.edges[24] = {8, 10}; d3CurrentShape.edges[25] = {9, 11};
  d3CurrentShape.edges[26] = {12, 14}; d3CurrentShape.edges[27] = {13, 15};
  d3CurrentShape.edges[28] = {16, 17}; d3CurrentShape.edges[29] = {18, 19};
}

// Create a stella octangula
void d3CreateStellaOctangula() {
  d3FreeShape();
  
  d3CurrentShape.vertexCount = 8;
  d3CurrentShape.vertices = new d3Vertex[8];
  
  // First tetrahedron
  d3CurrentShape.vertices[0] = { 1,  1,  1};
  d3CurrentShape.vertices[1] = { 1, -1, -1};
  d3CurrentShape.vertices[2] = {-1,  1, -1};
  d3CurrentShape.vertices[3] = {-1, -1,  1};
  
  // Second tetrahedron (inverted)
  d3CurrentShape.vertices[4] = {-1, -1, -1};
  d3CurrentShape.vertices[5] = {-1,  1,  1};
  d3CurrentShape.vertices[6] = { 1, -1,  1};
  d3CurrentShape.vertices[7] = { 1,  1, -1};
  
  d3CurrentShape.edgeCount = 12;
  d3CurrentShape.edges = new d3Edge[12];
  d3CurrentShape.edges[0] = {0, 1}; d3CurrentShape.edges[1] = {0, 2};
  d3CurrentShape.edges[2] = {0, 3}; d3CurrentShape.edges[3] = {1, 2};
  d3CurrentShape.edges[4] = {1, 3}; d3CurrentShape.edges[5] = {2, 3};
  d3CurrentShape.edges[6] = {4, 5}; d3CurrentShape.edges[7] = {4, 6};
  d3CurrentShape.edges[8] = {4, 7}; d3CurrentShape.edges[9] = {5, 6};
  d3CurrentShape.edges[10] = {5, 7}; d3CurrentShape.edges[11] = {6, 7};
}

// Create a torus knot
void d3CreateTorusKnot() {
  d3FreeShape();
  
  const int d3Segments = 48;
  d3CurrentShape.vertexCount = d3Segments;
  d3CurrentShape.vertices = new d3Vertex[d3Segments];
  
  float d3P = 2; // Wraps around major radius
  float d3Q = 3; // Wraps around minor radius
  float d3MajorRadius = 1.2;
  float d3MinorRadius = 0.4;
  
  for (int d3I = 0; d3I < d3Segments; d3I++) {
    float d3T = (d3I / (float)d3Segments) * 2.0 * PI;
    
    float d3R = d3MajorRadius + d3MinorRadius * cos(d3Q * d3T);
    d3CurrentShape.vertices[d3I].x = d3R * cos(d3P * d3T);
    d3CurrentShape.vertices[d3I].y = d3R * sin(d3P * d3T);
    d3CurrentShape.vertices[d3I].z = d3MinorRadius * sin(d3Q * d3T);
  }
  
  d3CurrentShape.edgeCount = d3Segments;
  d3CurrentShape.edges = new d3Edge[d3Segments];
  for (int d3I = 0; d3I < d3Segments; d3I++) {
    d3CurrentShape.edges[d3I] = {d3I, (d3I + 1) % d3Segments};
  }
}

void d3LoadShape(int d3ShapeIndex) {
  switch(d3ShapeIndex % 6) {
    case 0: d3CreateCube(); break;
    case 1: d3CreateOctahedron(); break;
    case 2: d3CreateIcosahedron(); break;
    case 3: d3CreateDodecahedron(); break;
    case 4: d3CreateStellaOctangula(); break;
    case 5: d3CreateTorusKnot(); break;
  }
}

d3Vertex d3TransformVertex(d3Vertex d3V, float d3Angle) {
  d3Vertex d3Result;
  
  // Scale
  d3Result.x = d3V.x * d3Scale;
  d3Result.y = d3V.y * d3Scale;
  d3Result.z = d3V.z * d3Scale;
  
  // Rotate around Z-axis
  float d3CosZ = cos(d3Angle);
  float d3SinZ = sin(d3Angle);
  float d3TempX = d3Result.x * d3CosZ - d3Result.y * d3SinZ;
  float d3TempY = d3Result.x * d3SinZ + d3Result.y * d3CosZ;
  d3Result.x = d3TempX;
  d3Result.y = d3TempY;
  
  // Apply camera tilt X (looking down)
  float d3CosX = cos(d3CameraTiltX);
  float d3SinX = sin(d3CameraTiltX);
  d3TempY = d3Result.y * d3CosX - d3Result.z * d3SinX;
  float d3TempZ = d3Result.y * d3SinX + d3Result.z * d3CosX;
  d3Result.y = d3TempY;
  d3Result.z = d3TempZ;
  
  // Apply camera tilt Y (slight side angle)
  float d3CosY = cos(d3CameraTiltY);
  float d3SinY = sin(d3CameraTiltY);
  d3TempX = d3Result.x * d3CosY + d3Result.z * d3SinY;
  d3TempZ = -d3Result.x * d3SinY + d3Result.z * d3CosY;
  d3Result.x = d3TempX;
  d3Result.z = d3TempZ;
  
  // Move away from camera
  d3Result.z += d3CameraDistance;
  
  return d3Result;
}

void d3ProjectToScreen(d3Vertex d3V, int& d3ScreenX, int& d3ScreenY) {
  // Perspective projection with FOV
  float d3Fov = 400.0;
  if (d3V.z < 1.0) d3V.z = 1.0; // Prevent division by zero
  
  float d3Perspective = d3Fov / d3V.z;
  d3ScreenX = (int)(d3V.x * d3Perspective) + screenWidth / 2;
  d3ScreenY = (int)(d3V.y * d3Perspective) + screenHeight / 2;
}

void d3RenderShape() {
  tft.fillScreen(d3BackgroundColor);
  
  // Transform all vertices
  d3Vertex* d3TransformedVerts = new d3Vertex[d3CurrentShape.vertexCount];
  
  for (int d3I = 0; d3I < d3CurrentShape.vertexCount; d3I++) {
    d3TransformedVerts[d3I] = d3TransformVertex(d3CurrentShape.vertices[d3I], d3RotationAngle);
  }
  
  // Draw all edges
  for (int d3I = 0; d3I < d3CurrentShape.edgeCount; d3I++) {
    d3Edge d3E = d3CurrentShape.edges[d3I];
    
    int d3X1, d3Y1, d3X2, d3Y2;
    d3ProjectToScreen(d3TransformedVerts[d3E.v1], d3X1, d3Y1);
    d3ProjectToScreen(d3TransformedVerts[d3E.v2], d3X2, d3Y2);
    
    // Only draw if both points are roughly on screen
    if (d3X1 > -50 && d3X1 < screenWidth + 50 && d3Y1 > -50 && d3Y1 < screenHeight + 50 &&
        d3X2 > -50 && d3X2 < screenWidth + 50 && d3Y2 > -50 && d3Y2 < screenHeight + 50) {
      tft.drawLine(d3X1, d3Y1, d3X2, d3Y2, d3WireframeColor);
    }
  }
  
  delete[] d3TransformedVerts;
}

void onWireframe3dPageOpen() {
  tft.fillScreen(d3BackgroundColor);
  d3CurrentShapeIndex = 0;
  d3RotationAngle = 0.0;
  d3LoadShape(d3CurrentShapeIndex);
  d3LeftButtonWasPressed = false;
  d3RightButtonWasPressed = false;
}

void onWireframe3dPageUpdate() {
  bool d3LeftIsDown = (digitalRead(LEFT_BUTTON) == LOW);
  bool d3RightIsDown = (digitalRead(RIGHT_BUTTON) == LOW);
  
  // Left button: cycle through shapes
  if (d3LeftButtonWasPressed && !d3LeftIsDown) {
    d3CurrentShapeIndex++;
    d3LoadShape(d3CurrentShapeIndex);
    d3RotationAngle = 0.0;
  }
  
  // Right button: exit
  if (d3RightButtonWasPressed && !d3RightIsDown) {
    d3FreeShape();
    closePageIfAnyButtonIsPressed();
    return;
  }
  
  d3LeftButtonWasPressed = d3LeftIsDown;
  d3RightButtonWasPressed = d3RightIsDown;
  
  // Update rotation and render
  d3RotationAngle += d3RotationSpeed;
  if (d3RotationAngle > 2 * PI) d3RotationAngle -= 2 * PI;
  
  d3RenderShape();
}