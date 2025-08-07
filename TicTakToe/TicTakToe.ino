#include "Arduino.h"
#include "Joystick.h"
#include "MaxMatrix.h"
#include "Button.h"

// Pin Definitions
#define JOYSTICK_PIN_SW 2
#define JOYSTICK_PIN_VRX A10
#define JOYSTICK_PIN_VRY A0
#define LEDMATRIX_PIN_DIN 50
#define LEDMATRIX_PIN_CLK 51
#define LEDMATRIX_PIN_CS 53
#define PUSHBUTTON_PIN_2 3

// Hardware initialization
Joystick joystick(JOYSTICK_PIN_VRX, JOYSTICK_PIN_VRY, JOYSTICK_PIN_SW);
MaxMatrix ledMatrix(LEDMATRIX_PIN_DIN, LEDMATRIX_PIN_CS, LEDMATRIX_PIN_CLK);
Button pushButton(PUSHBUTTON_PIN_2);

// Game board (0 = empty, 1 = X, 2 = O)
byte board[3][3] = { 0 };

// Cursor position
int cursorX = 0;
int cursorY = 0;

// Turn: true = X, false = O
bool currentPlayer = true;

// Debounce
unsigned long lastMoveTime = 0;
const int moveDelay = 250;

// Cursor blink
bool cursorVisible = true;
unsigned long lastBlinkTime = 0;
const int blinkInterval = 300;

// Pixel locations for grid cells
const int cellSize = 2; // size of each cell in pixels
const int cellOffset = 1; // spacing between cells

void setup() {
  Serial.begin(9600);
  ledMatrix.init(1);
  ledMatrix.setIntensity(5);
  ledMatrix.clear();
  pushButton.init();
}

void drawSymbol(int cellX, int cellY, byte value) {
  int baseX = cellX * (cellSize + cellOffset);
  int baseY = cellY * (cellSize + cellOffset);

  if (value == 1) {
    // Draw X
    ledMatrix.setDot(baseX,     baseY,     true);
    ledMatrix.setDot(baseX + 1, baseY + 1, true);
    ledMatrix.setDot(baseX,     baseY + 1, true);
    ledMatrix.setDot(baseX + 1, baseY,     true);
  } else if (value == 2) {
    // Draw O
    ledMatrix.setDot(baseX,     baseY,     true);
    ledMatrix.setDot(baseX + 1, baseY,     true);
    ledMatrix.setDot(baseX,     baseY + 1, true);
    ledMatrix.setDot(baseX + 1, baseY + 1, true);
  }
}

void drawBoard() {
  ledMatrix.clear();
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      if (board[y][x] != 0) {
        drawSymbol(x, y, board[y][x]);
      }
    }
  }

  // Cursor blink
  if (cursorVisible) {
    int cx = cursorX * (cellSize + cellOffset);
    int cy = cursorY * (cellSize + cellOffset);
    ledMatrix.setDot(cx,     cy,     true);
    ledMatrix.setDot(cx + 1, cy,     true);
    ledMatrix.setDot(cx,     cy + 1, true);
    ledMatrix.setDot(cx + 1, cy + 1, true);
  }
}

void handleInput() {
  int x = joystick.getX();
  int y = joystick.getY();

  unsigned long now = millis();

  if (now - lastMoveTime > moveDelay) {
    if (x < 300 && cursorX > 0) {
      cursorX--;
      lastMoveTime = now;
    } else if (x > 700 && cursorX < 2) {
      cursorX++;
      lastMoveTime = now;
    }

    if (y < 300 && cursorY < 2) {
      cursorY++;
      lastMoveTime = now;
    } else if (y > 700 && cursorY > 0) {
      cursorY--;
      lastMoveTime = now;
    }
  }

  if (pushButton.onPress()) {
    if (board[cursorY][cursorX] == 0) {
      board[cursorY][cursorX] = currentPlayer ? 1 : 2;
      currentPlayer = !currentPlayer;
    }
  }
}

void loop() {
  handleInput();

  // Blink cursor
  if (millis() - lastBlinkTime > blinkInterval) {
    cursorVisible = !cursorVisible;
    lastBlinkTime = millis();
  }

  drawBoard();
  delay(50);
}
