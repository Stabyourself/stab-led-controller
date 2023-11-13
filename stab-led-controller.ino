#include "FastLED.h"

#define DATA_PIN 6
// #define CLK_PIN   4
#define LED_TYPE WS2812B
#define NUM_LEDS 288
#define BRIGHTNESS 50

CRGB leds[NUM_LEDS];

byte colors[6][3] = {
    {0, 255, 0},
    {255, 32, 135},
    {255, 0, 0},
    {0, 0, 255},
    {255, 255, 0},
    {0, 255, 255}};

int player1color = 0;
int player2color = 1;

CRGBPalette16 currentPalette;
void updateColors() {
  TDynamicRGBGradientPalette_byte palette[] = {
      0, colors[player1color][0], colors[player1color][1], colors[player1color][2],
      62, colors[player1color][0], colors[player1color][1], colors[player1color][2],
      66, colors[player2color][0], colors[player2color][1], colors[player2color][2],
      188, colors[player2color][0], colors[player2color][1], colors[player2color][2],
      194, colors[player1color][0], colors[player1color][1], colors[player1color][2],
      255, colors[player1color][0], colors[player1color][1], colors[player1color][2]};

  currentPalette.loadDynamicGradientPalette(palette);
}

void setup() {
  updateColors();

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip)
      .setDither(BRIGHTNESS < 255);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  Serial.begin(115200);
}

#define PIXELS_PER_STRIP 60
void modeTitle() {
  for (int i = 0; i < NUM_LEDS / 2; i++) {
    int pixelPos = (i * (256 / PIXELS_PER_STRIP) - int(millis() * 0.2)) % 256;
    leds[i] = ColorFromPalette(currentPalette, pixelPos);
    leds[NUM_LEDS - i - 1] = ColorFromPalette(currentPalette, pixelPos);
  }
}

void (*modes[])() = {modeTitle};
int currentMode = 0;

unsigned long effectStart;
int currentEffect = -1;

void effect(int num) {
  effectStart = millis();
  currentEffect = num;
}

#define TETRISONTIME 200
#define TETRISOFFTIME 200
#define TETRISFLASHES 3
void effectTetris() {
  int time = (millis() - effectStart);
  bool on = time % (TETRISONTIME + TETRISOFFTIME) < TETRISONTIME;

  if (on) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::White;
    }
  }

  if (time > (TETRISONTIME + TETRISOFFTIME) * TETRISFLASHES - TETRISOFFTIME) {
    currentEffect = -1;
  }
}

void (*effects[])() = {effectTetris};

void loop() {
  // check if data is available
  while (Serial.available() > 0) {
    // read the incoming byte:
    int command = Serial.read();
    Serial.println("Command: " + String(command));

    switch (command) {
      case 0 ... 127:  // mode change
        currentMode = command;
        break;
      case 128 ... 133:  // p1 color
        player1color = command - 128;
        updateColors();
        break;
      case 134 ... 139:  // p2 color
        player2color = command - 134;
        updateColors();
        break;
      case 140 ... 255:  //  effects
        effect(command - 140);
        break;
    }
  }

  modes[currentMode]();

  if (currentEffect != -1) {
    effects[currentEffect]();
  }

  FastLED.show();
}