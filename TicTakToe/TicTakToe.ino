#include "Arduino.h"
#include "MaxMatrix.h"

// Joystick Pins
#define JOY_X A10
#define JOY_Y A0

// Matrix Pins
#define LEDMATRIX_DIN 50
#define LEDMATRIX_CLK 51
#define LEDMATRIX_CS  53

// Button (for placing marks)
#define BTN_PIN 3

// LED Matrix
MaxMatrix ledMatrix(LEDMATRIX_DIN, LEDMATRIX_CS, LEDMATRIX_CLK);

// Game state
byte board[3][3] = { 0 };
int cursorX = 0, cursorY = 0;
bool currentPlayer = true;

// Timing
unsigned long lastMoveTime = 0;
unsigned long lastBlinkTime = 0;
bool cursorVisible = true;
const int moveDelay = 250;
const int blinkDelay = 300;

void setup() {
  Serial.begin(9600);
  pinMode(BTN_PIN, INPUT_PULLUP);

  ledMatrix.init(1);
  ledMatrix.setIntensity(5);
  ledMatrix.clear();

  Serial.println("Tic-Tac-Toe started.");
  printBoardState();
}

void drawSymbol(int cellX, int cellY, byte value) {
  int baseX = cellX * 3;
  int baseY = cellY * 3;

  if (value == 1) {
    // X = 2 diagonal dots
    ledMatrix.setDot(baseX,     baseY,     true);
    ledMatrix.setDot(baseX + 2, baseY + 2, true);
  } else if (value == 2) {
    // O = 4-dot ring
    ledMatrix.setDot(baseX,     baseY,     true);
    ledMatrix.setDot(baseX + 1, baseY,     true);
    ledMatrix.setDot(baseX,     baseY + 1, true);
    ledMatrix.setDot(baseX + 1, baseY + 1, true);
  }
}

void drawBoard() {
  // Clear display
  for (int x = 0; x < 8; x++)
    for (int y = 0; y < 8; y++)
      ledMatrix.setDot(x, y, false);

  // Draw Xs and Os
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      if (board[y][x] != 0) {
        drawSymbol(x, y, board[y][x]);
      }
    }
  }

  // Draw blinking cursor in center of selected cell
  if (cursorVisible) {
    int cx = cursorX * 3;
    int cy = cursorY * 3;
    ledMatrix.setDot(cx, cy, true);
  }
}

void printBoardState() {
  Serial.println("Board:");
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      char ch = '.';
      if (board[y][x] == 1) ch = 'X';
      if (board[y][x] == 2) ch = 'O';
      Serial.print(ch);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println();
}

bool checkWin(int player) {
  for (int i = 0; i < 3; i++) {
    if (board[i][0] == player && board[i][1] == player && board[i][2] == player) return true;
    if (board[0][i] == player && board[1][i] == player && board[2][i] == player) return true;
  }
  if (board[0][0] == player && board[1][1] == player && board[2][2] == player) return true;
  if (board[0][2] == player && board[1][1] == player && board[2][0] == player) return true;
  return false;
}

void flashVictory() {
  for (int i = 0; i < 4; i++) {
    for (int x = 0; x < 8; x++)
      for (int y = 0; y < 8; y++)
        ledMatrix.setDot(x, y, true);
    delay(200);
    for (int x = 0; x < 8; x++)
      for (int y = 0; y < 8; y++)
        ledMatrix.setDot(x, y, false);
    delay(200);
  }
}

void resetGame() {
  for (int y = 0; y < 3; y++)
    for (int x = 0; x < 3; x++)
      board[y][x] = 0;

  cursorX = 0;
  cursorY = 0;
  currentPlayer = true;
  Serial.println("Game reset.");
  printBoardState();
}

void handleInput() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  bool btnPressed = digitalRead(BTN_PIN) == LOW;

  unsigned long now = millis();

  // Cursor movement with wrap-around
  if (now - lastMoveTime > moveDelay) {
    if (joyX < 300) {
      cursorX = (cursorX + 2) % 3; // wrap left
      lastMoveTime = now;
      Serial.println("Moved Left");
    } else if (joyX > 700) {
      cursorX = (cursorX + 1) % 3; // wrap right
      lastMoveTime = now;
      Serial.println("Moved Right");
    }

    if (joyY < 300) {
      cursorY = (cursorY + 1) % 3; // wrap down
      lastMoveTime = now;
      Serial.println("Moved Down");
    } else if (joyY > 700) {
      cursorY = (cursorY + 2) % 3; // wrap up
      lastMoveTime = now;
      Serial.println("Moved Up");
    }
  }

  // Button press detection (edge-triggered)
  static bool lastBtnState = HIGH;
  if (lastBtnState == HIGH && btnPressed && board[cursorY][cursorX] == 0) {
    int thisPlayer = currentPlayer ? 1 : 2;
    board[cursorY][cursorX] = thisPlayer;

    Serial.print("Placed "); Serial.println(thisPlayer == 1 ? "X" : "O");
    printBoardState();

    if (checkWin(thisPlayer)) {
      Serial.print("Player "); Serial.print(thisPlayer == 1 ? "X" : "O"); Serial.println(" wins!");
      flashVictory();
      resetGame();
      delay(500);  // Small pause after reset
    } else {
      currentPlayer = !currentPlayer;
    }
  }
  lastBtnState = btnPressed;
}

void loop() {
  handleInput();

  // Cursor blinking logic
  if (millis() - lastBlinkTime > blinkDelay) {
    cursorVisible = !cursorVisible;
    lastBlinkTime = millis();
  }

  drawBoard();
  delay(30);
}
