#include <LedControl.h>

// DIN, CLK, CS, numero moduli
LedControl lc = LedControl(11, 13, 10, 2);

// ===== FONT 5x7 =====
byte numeri[10][7] = {
  {B11111,B10001,B10011,B10101,B11001,B10001,B11111},
  {B00100,B01100,B00100,B00100,B00100,B00100,B01110},
  {B11111,B00001,B00001,B11111,B10000,B10000,B11111},
  {B11111,B00001,B00001,B01111,B00001,B00001,B11111},
  {B10001,B10001,B10001,B11111,B00001,B00001,B00001},
  {B11111,B10000,B10000,B11111,B00001,B00001,B11111},
  {B11111,B10000,B10000,B11111,B10001,B10001,B11111},
  {B11111,B00001,B00010,B00100,B01000,B01000,B01000},
  {B11111,B10001,B10001,B11111,B10001,B10001,B11111},
  {B11111,B10001,B10001,B11111,B00001,B00001,B11111}
};

#define SHIFT 2

// ===== SLOT VAR =====
unsigned long tempoU = 0;
unsigned long tempoD = 0;

int numeroU = 0;
int numeroD = 0;

int intervalloU = 200;
int intervalloD = 200;

int minVel = 40;
int maxVel = 300;

enum Stato {ACCELERA, VELOCE, RALLENTA, STOP};

Stato statoU = ACCELERA;
Stato statoD = ACCELERA;

int contU = 0;
int contD = 0;

bool startDecine = false;

// ===== TARGET RANDOM =====
int targetU = 0;
int targetD = 0;

// ===== JACKPOT =====
bool jackpot = false;
unsigned long tempoJackpot = 0;
int faseJackpot = 0;

// ===== SETUP =====
void setup() {
  for(int i=0;i<2;i++) {
    lc.shutdown(i,false);
    lc.setIntensity(i,8);
    lc.clearDisplay(i);
  }

  randomSeed(analogRead(A5));

  targetU = random(0,10);
  targetD = random(0,10);
}

// ===== MOSTRA CIFRA =====
void mostraDigit(int display, int num) {
  for(int r=0; r<7; r++) {
    lc.setRow(display, r, numeri[num][r] << SHIFT);
  }
}

// ===== MOSTRA NUMERO =====
void mostraNumero(int u, int d) {
  mostraDigit(0, u);
  mostraDigit(1, d);
}

// ===== LOOP =====
void loop() {

  if (!jackpot) {

    // ===== UNITÀ =====
    if (millis() - tempoU > intervalloU && statoU != STOP) {
      tempoU = millis();

      numeroU = (numeroU + 1) % 10;
      mostraDigit(0, numeroU);

      switch(statoU) {

        case ACCELERA:
          intervalloU -= 10;
          if (intervalloU <= minVel) {
            intervalloU = minVel;
            statoU = VELOCE;
            contU = 0;
            startDecine = true;
          }
          break;

        case VELOCE:
          contU++;
          if (contU > 25) statoU = RALLENTA;
          break;

        case RALLENTA:
          intervalloU += 10;
          if (intervalloU >= maxVel && numeroU == targetU) {
            statoU = STOP;
          }
          break;

        default: break;
      }
    }

    // ===== DECINE =====
    if (startDecine && millis() - tempoD > intervalloD && statoD != STOP) {
      tempoD = millis();

      numeroD = (numeroD + 1) % 10;
      mostraDigit(1, numeroD);

      switch(statoD) {

        case ACCELERA:
          intervalloD -= 10;
          if (intervalloD <= minVel) {
            intervalloD = minVel;
            statoD = VELOCE;
            contD = 0;
          }
          break;

        case VELOCE:
          contD++;
          if (contD > 40) statoD = RALLENTA;
          break;

        case RALLENTA:
          intervalloD += 10;
          if (intervalloD >= maxVel && numeroD == targetD) {
            statoD = STOP;
          }
          break;

        default: break;
      }
    }

    // ===== ATTIVA JACKPOT =====
    if (statoU == STOP && statoD == STOP) {
      jackpot = true;
      tempoJackpot = millis();
      faseJackpot = 0;
    }
  }

  // ===== ANIMAZIONE JACKPOT =====
  if (jackpot) {

    if (millis() - tempoJackpot > 100) {
      tempoJackpot = millis();
      faseJackpot++;

      switch(faseJackpot) {

        case 1: case 3: case 5: case 7:
          lc.clearDisplay(0);
          lc.clearDisplay(1);
          break;

        case 2: case 4: case 6: case 8:
          mostraNumero(numeroU, numeroD);
          break;

        case 9: case 10:
          for(int r=0;r<7;r++) {
            lc.setRow(0, r, B11111 << SHIFT);
            lc.setRow(1, r, B11111 << SHIFT);
          }
          break;

        case 11: case 12:
          for(int r=0;r<7;r++) {
            lc.setRow(0, r, B10101 << SHIFT);
            lc.setRow(1, r, B01010 << SHIFT);
          }
          break;

        case 13:
          mostraNumero(numeroU, numeroD);
          break;

        case 20:
          delay(2000);

          // RESET SLOT
          statoU = ACCELERA;
          statoD = ACCELERA;

          intervalloU = 200;
          intervalloD = 200;

          startDecine = false;

          targetU = random(0,10);

          // probabilità jackpot 😄
          if(random(0,100) < 15) {
            targetD = targetU;
          } else {
            targetD = random(0,10);
          }

          jackpot = false;
          break;
      }
    }
  }
}