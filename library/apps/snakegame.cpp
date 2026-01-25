#include "colortest.h"
#include "snakegame.h"
#include <vector>
#include <Preferences.h>

// Preferences object for storing high score
Preferences snakeScoreStore;

// Game states
enum GameState { PLAYING, DEAD };
GameState gameState = PLAYING;

// Direction enum
enum Direction { UP, RIGHT, DOWN, LEFT };

// Snake segment
struct Segment {
  int x, y;
};

// Game variables
std::vector<Segment> snake;
Direction currentDir = RIGHT;
Segment food;
unsigned long lastMoveTime = 0;
const int moveDelay = 150; // ms between moves
int score = 0;
int highScore = 0;
bool gameInitialized = false;

// Button state tracking
bool leftButtonWasPressed = false;
bool rightButtonWasPressed = false;

const uint16_t SNAKE_COLOR = rgb565(0, 255, 0);  // Green
const uint16_t FOOD_COLOR = rgb565(255, 0, 0);   // Red
const uint16_t BG_COLOR = rgb565(0, 0, 0);       // Black
const int CELL_SIZE = 8;
int gridWidth, gridHeight;

void spawnFood() {
  bool validPosition = false;
  while (!validPosition) {
    food.x = random(0, gridWidth);
    food.y = random(0, gridHeight);
    
    validPosition = true;
    for (const auto& seg : snake) {
      if (seg.x == food.x && seg.y == food.y) {
        validPosition = false;
        break;
      }
    }
  }
}

void initGame() {
  gridWidth = screenWidth / CELL_SIZE;
  gridHeight = screenHeight / CELL_SIZE;
  
  snake.clear();
  snake.push_back({gridWidth / 2, gridHeight / 2});
  snake.push_back({gridWidth / 2 - 1, gridHeight / 2});
  snake.push_back({gridWidth / 2 - 2, gridHeight / 2});
  snake.push_back({gridWidth / 2 - 3, gridHeight / 2});
  
  currentDir = RIGHT;
  gameState = PLAYING;
  score = 0;
  
  leftButtonWasPressed = false;
  rightButtonWasPressed = false;
  
  spawnFood();
  gameInitialized = true;
}

void drawCell(int x, int y, uint16_t color) {
  tft.fillRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
}

Direction rotateLeft(Direction dir) {
  return (Direction)((dir + 3) % 4);
}

Direction rotateRight(Direction dir) {
  return (Direction)((dir + 1) % 4);
}

void updateGame() {
  // Handle rotation input with press-and-release detection
  bool leftIsDown = (digitalRead(LEFT_BUTTON) == LOW);
  bool rightIsDown = (digitalRead(RIGHT_BUTTON) == LOW);
  
  // Detect button release (was pressed, now not pressed)
  if (leftButtonWasPressed && !leftIsDown) {
    currentDir = rotateLeft(currentDir);
  }
  if (rightButtonWasPressed && !rightIsDown) {
    currentDir = rotateRight(currentDir);
  }
  
  // Update button states
  leftButtonWasPressed = leftIsDown;
  rightButtonWasPressed = rightIsDown;
  
  // Move snake
  if (millis() - lastMoveTime > moveDelay) {
    lastMoveTime = millis();
    
    // Calculate new head position
    Segment newHead = snake[0];
    switch (currentDir) {
      case UP:    newHead.y--; break;
      case DOWN:  newHead.y++; break;
      case LEFT:  newHead.x--; break;
      case RIGHT: newHead.x++; break;
    }
    
    // Wrap around screen edges
    if (newHead.x < 0) newHead.x = gridWidth - 1;
    if (newHead.x >= gridWidth) newHead.x = 0;
    if (newHead.y < 0) newHead.y = gridHeight - 1;
    if (newHead.y >= gridHeight) newHead.y = 0;
    
    // Check self collision
    for (const auto& seg : snake) {
      if (seg.x == newHead.x && seg.y == newHead.y) {
        gameState = DEAD;
        
        // Update high score if needed
        if (score > highScore) {
          highScore = score;
          snakeScoreStore.begin("snakegame", false);
          snakeScoreStore.putInt("highscore", highScore);
          snakeScoreStore.end();
        }
        return;
      }
    }
    
    // Check food collision
    bool ateFood = (newHead.x == food.x && newHead.y == food.y);
    
    // Add new head
    snake.insert(snake.begin(), newHead);
    drawCell(newHead.x, newHead.y, SNAKE_COLOR);
    
    if (ateFood) {
      score++;
      spawnFood();
      drawCell(food.x, food.y, FOOD_COLOR);
    } else {
      // Remove tail
      Segment tail = snake.back();
      snake.pop_back();
      drawCell(tail.x, tail.y, BG_COLOR);
    }
  }
}

void showDeathScreen() {
  tft.fillScreen(rgb565(40, 0, 0)); // Dark red background
  
  tft.setTextSize(3);
  tft.setTextColor(rgb565(255, 0, 0));
  int textY = screenHeight / 2 - 50;
  String gameOver = "GAME OVER";
  int textWidth = gameOver.length() * 18;
  tft.setCursor((screenWidth - textWidth) / 2, textY);
  tft.println(gameOver);
  
  tft.setTextSize(2);
  tft.setTextColor(rgb565(255, 255, 255));
  String scoreText = "Score: " + String(score);
  textWidth = scoreText.length() * 12;
  tft.setCursor((screenWidth - textWidth) / 2, textY + 40);
  tft.println(scoreText);
  
  // Show high score
  tft.setTextColor(rgb565(255, 215, 0)); // Gold color
  String highScoreText = "Best: " + String(highScore);
  textWidth = highScoreText.length() * 12;
  tft.setCursor((screenWidth - textWidth) / 2, textY + 65);
  tft.println(highScoreText);
  
  tft.setTextSize(1);
  tft.setTextColor(rgb565(200, 200, 200));
  String restart = "Press any button to exit";
  textWidth = restart.length() * 6;
  tft.setCursor((screenWidth - textWidth) / 2, textY + 100);
  tft.println(restart);
}

void onSnakeGamePageOpen() {
  // Load high score from preferences
  snakeScoreStore.begin("snakegame", true); // read-only
  highScore = snakeScoreStore.getInt("highscore", 0);
  snakeScoreStore.end();
  
  tft.fillScreen(BG_COLOR);
  gameInitialized = false;
  initGame();
  
  // Draw initial snake
  for (const auto& seg : snake) {
    drawCell(seg.x, seg.y, SNAKE_COLOR);
  }
  
  // Draw food
  drawCell(food.x, food.y, FOOD_COLOR);
}

void onSnakeGamePageUpdate() {
  if (gameState == PLAYING) {
    updateGame();
    
    if (gameState == DEAD) {
      showDeathScreen();
    }
  } else if (gameState == DEAD) {
    // Use the standard close function on death screen
    closePageIfAnyButtonIsPressed();
  }
}