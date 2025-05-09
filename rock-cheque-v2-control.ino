#include "Arduino.h"

// Addressable LED Library
#include <FastLED.h>

// GPIO Expander Library
#include "clsPCA9555.h"

// I2C
#include "Wire.h"

// OLED Library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Pins for Multiplexed Addressable LEDs
#define LED_PIN 19
#define MULT_0 25
#define MULT_1 26
#define MULT_2 27

// Interrupt pin for Button Press / Controller Plugged IN
#define INT_PIN 18

// On board LEDs
#define LED_RED 4
#define LED_GREEN 5

// On board buttons
#define BUTTON_0    34
#define BUTTON_1    35

// I2C Addresses
#define OLED_ADDR 0x3C
#define INPUT_ADDR 0x20
#define OUTPUT_ADDR 0x21

// GPIO Expander Objects
PCA9555 INPUTS(0x20);
PCA9555 OUTPUTS(0x21);

// OLED Object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Multiplexer Addressing
const int mult_0[] = {0, 1, 0, 1, 0, 1, 0, 1};
const int mult_1[] = {0, 0, 1, 1, 0, 0, 1, 1};
const int mult_2[] = {0, 0, 0, 0, 1, 1, 1, 1};

// Remote States
CRGB remoteColours[] = { CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue };

bool remoteConnected[] = {0, 0, 0, 0, 0, 0, 0, 0};
bool buttonState[] = {0, 0, 0, 0, 0, 0, 0, 0};

// Input Maps
uint8_t remoteSenseMap[] = {0, 1, 2, 3, 8, 9, 10, 11};
uint8_t remoteButtonMap[] = {4, 5, 6, 7, 12, 13, 14, 15};

// Output Maps
uint8_t remoteIndicatorMap[] = {0, 1, 2, 3, 4, 5, 6, 7};
uint8_t remoteVibrateMap[] = {15, 14, 13, 12, 11, 10, 9, 8};

// Storage variable for FastLED
CRGB leds[1];

// GPIO Interrupt Flag
bool interruptFlag = false;

// OLED Logo
// 'RockChequeLogo', 128x32px
const unsigned char epd_bitmap_RockChequeLogo [] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa0, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa1, 0xff, 0x80, 0x00, 0x00, 0x60, 0x00, 0x3f, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa1, 0xff, 0xc0, 0x00, 0x00, 0x70, 0x00, 0x7f, 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa1, 0xc1, 0xc0, 0x00, 0x00, 0x70, 0x00, 0xe1, 0xe7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa1, 0xc1, 0xc3, 0xe0, 0x7c, 0x71, 0x80, 0xe0, 0xe7, 0xf0, 0x3e, 0x0f, 0x63, 0x0c, 0x1e, 0x05, 
	0xa1, 0xc1, 0xcf, 0xf0, 0xfe, 0x73, 0x81, 0xc0, 0x07, 0xf8, 0x7f, 0x1f, 0xf3, 0x0e, 0x3f, 0x85, 
	0xa1, 0xff, 0x8c, 0x39, 0xc7, 0x77, 0x01, 0xc0, 0x07, 0x18, 0xe3, 0x18, 0xf3, 0x0e, 0x61, 0x85, 
	0xa1, 0xff, 0x1c, 0x39, 0x82, 0x7e, 0x01, 0xc0, 0x07, 0x1c, 0xc3, 0xb8, 0x73, 0x0e, 0xe1, 0x85, 
	0xa1, 0xff, 0x1c, 0x3b, 0x80, 0x7e, 0x01, 0xc0, 0x07, 0x1d, 0xff, 0xb8, 0x73, 0x0e, 0xff, 0x85, 
	0xa1, 0xe7, 0x9c, 0x3b, 0x80, 0x7f, 0x00, 0xc0, 0xe7, 0x1d, 0xfe, 0x38, 0x73, 0x0e, 0xff, 0x05, 
	0xa1, 0xe3, 0xdc, 0x39, 0x83, 0x73, 0x00, 0xe0, 0xe7, 0x1c, 0xc0, 0x38, 0x73, 0x0e, 0xe0, 0x05, 
	0xa1, 0xe3, 0xce, 0x31, 0xc7, 0x73, 0x80, 0x71, 0xc7, 0x1c, 0xe3, 0x1c, 0xf3, 0x9e, 0x71, 0x85, 
	0xa1, 0xe1, 0xe7, 0xf0, 0xfe, 0x71, 0x80, 0x7f, 0x87, 0x1c, 0x7f, 0x1f, 0xf1, 0xfe, 0x3f, 0x85, 
	0xa0, 0xc0, 0xc3, 0xc0, 0x38, 0x20, 0x80, 0x1e, 0x06, 0x18, 0x1c, 0x07, 0x70, 0xf6, 0x1e, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
	0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

// Game State
enum State {
    IDLE,
    READY,
    BUZZED,
    ANSWER,
};
State gameState = READY;

// Selected Player
unsigned long flashMillis = 0;
bool flashState = false;
unsigned long vibrateMillis = 0;
bool vibrateState = false;
uint8_t vibrateCount = 0;
bool vibrating = false;
int selectedPlayer = -1;

// Buzzers Armed
unsigned long armedVibrateMillis = 0;


void setup()
{

    // Start serial to PC
    Serial.begin(115200);

    // Pin Modes
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);

    pinMode(BUTTON_0, INPUT_PULLUP);
    pinMode(BUTTON_1, INPUT_PULLUP);

    pinMode(INT_PIN, INPUT);

    pinMode(LED_PIN, OUTPUT);

    pinMode(MULT_0, OUTPUT);
    pinMode(MULT_1, OUTPUT);
    pinMode(MULT_2, OUTPUT);

    // Setup FastLED
    FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, 1);
    showAllArray(remoteColours);

    // Setup Multiplexer
    digitalWrite(MULT_0, LOW);
    digitalWrite(MULT_1, LOW);
    digitalWrite(MULT_2, LOW);

    // Setup GPIO Expanders
    INPUTS.begin();
    INPUTS.setClock(400000);
    OUTPUTS.begin();
    OUTPUTS.setClock(400000);
    attachInterrupt(INT_PIN, EXPANDER_ISR, FALLING);
    INPUTS.digitalRead(0); // Reading resets interrupt flag PORT0
    INPUTS.digitalRead(8); // Reading resets interrupt flag PORT1

    for (uint8_t i = 0; i < 16; i++)
    {
        INPUTS.pinMode(i, INPUT);
        OUTPUTS.pinMode(i, OUTPUT);
        OUTPUTS.digitalWrite(i, LOW);
    }

    // Setup OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.setRotation(2);
    
    display.setTextSize(2);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println(F("ROCK      CHEQUE"));
    display.display();

    displayLogo();

    // Boot flash sequence
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, HIGH);
    delay(100);
    digitalWrite(LED_RED, LOW);
    delay(100);
    digitalWrite(LED_RED, HIGH);
    delay(100);
    digitalWrite(LED_RED, LOW);
    delay(100);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);

    // Get connected remotes
    for (uint8_t i = 0; i < 8; i++)
    {
        remoteConnected[i] = remoteSense(i);
    }

    // Serial.println("")
}

void loop()
{
    // -------- Serial Handling --------
    if (Serial.available() > 0) {
        char inByte = Serial.read();

        switch (inByte) {
            // Assign colour to remote
            case 'C':
            {
                uint8_t index = Serial.parseInt();
                Serial.read();  // Remote Delimiter
                int primary = Serial.parseInt();
                Serial.print(">");
                Serial.print(" Updated remote ");
                Serial.print(index);
                Serial.print(" with colour 0x");
                Serial.print(primary, HEX);
                Serial.print(" in decimal ");
                Serial.println(primary, DEC);
                remoteColours[index] = primary;
                showOn(index, remoteColours[index]);
                break;
            }
            // Connection test
            case 'T':
            {
                Serial.println("OK");
                break;
            }
            // Arm buzzers
            case 'A':
            {
                if (gameState == IDLE) {
                    armedVibrateMillis = millis();
                    vibrateAll(true);
                    gameState = READY;
                    Serial.println("> Buzzers ARMED! Moving to READY state");
                }
                break;
            }
            // Correct answer
            case 'Y':
            {
                if (gameState == BUZZED) {
                    showAll(CRGB::Lime);
                    gameState = ANSWER;
                    Serial.println("> Correct! Moving to ANSWER state");
                }
                break;
            }
            // Incorrect answer
            case 'N':
            {
                if (gameState == BUZZED) {
                    showAll(CRGB::Red);
                    gameState = ANSWER;
                    Serial.println("> Incorrect! Moving to ANSWER state");
                }
                break;
            }
            // Reset
            case 'R':
            {
                showAllArray(remoteColours);
                gameState = IDLE;
                Serial.println("> Moving to IDLE state");
                break;
            }
        }
    }

    // digitalWrite(LED_RED, digitalRead(INT_PIN));

    // -------- Interrupt Handler --------
    if (interruptFlag)
    {
        interruptFlag = false; // Clear the interrupt flag
        INPUTS.digitalRead(0); // Reading resets interrupt flag PORT0
        INPUTS.digitalRead(8); // Reading resets interrupt flag PORT1

        // Handle remotes connecting
        for (uint8_t i = 0; i < 8; i++)
        {
            bool pin = remoteSense(i);
            if (pin != remoteConnected[i]) {
                remoteConnected[i] = pin;
                if (pin)
                {
                    Serial.print("Remote ");
                    Serial.print(i);
                    Serial.println(" connected!");

                    delay(50);
                    showOn(i, remoteColours[i]);
                }
                else
                {
                    Serial.print("Remote ");
                    Serial.print(i);
                    Serial.println(" disconnected!");
                }

            }
        }



        for (uint8_t i = 0; i < 8; i++)
        {
            if (remoteConnected[i])
            { // Only keep useful data
                bool pin = remoteButton(i);
                if (pin != buttonState[i]) {
                    buttonState[i] = pin;
                    switch (gameState) {
                        case IDLE:
                        {
                            if (pin) {
                                Serial.print("P");
                                Serial.println(i);
                            } else {
                                // Serial.print("R");   // Button released not used for anything
                                // Serial.println(i);
                            }
                            break;
                        }
                        case READY:
                        {
                            if (pin)
                            {
                                Serial.print("B");
                                Serial.println(i);

                                Serial.print("> Button ");
                                Serial.print(i);
                                Serial.println(" pressed!");

                                vibrateAll(false); // Cancel armed vibration if buzzer pressed immediately

                                gameState = BUZZED;
                                flashMillis = millis();
                                vibrateMillis = millis();
                                vibrateCount = 0;
                                selectedPlayer = i;
                                flashState = true;
                                vibrateState = true;
                                vibrating = true;

                                showAll(remoteColours[i]);
                                // showOn(i, remoteColours[i]);
                                vibrateRemote(i, true);
                                indicateJack(i, true);
                            }
                            else
                            {
                                // Serial.print("Button ");
                                // Serial.print(i);
                                // Serial.println(" released!");

                                // showOn(i, remoteColours[i]);
                                // vibrateRemote(i, false);
                            }
                            buttonState[i] = pin;

                            break;
                        }
                        case BUZZED:
                        {
                            if (pin) {
                                Serial.print("L");  // Too late
                                Serial.println(i);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    
    if (gameState == BUZZED) {
        // Flashing when someone is answering
        if (millis() - flashMillis > 500) {
            flashMillis += 500;
            flashState = !flashState;
            showOn(selectedPlayer, flashState ? remoteColours[selectedPlayer] : (CRGB)0);
            indicateJack(selectedPlayer, flashState);
        }
        // Vibrate when someone buzzes in
        if (vibrating && (millis() - vibrateMillis > 100)) {
            vibrateMillis += 100;               // Adjust time since last
            vibrateCount++;                     // Increase amount of vibrations
            if (vibrateCount > 2)
                vibrateState = !vibrateState;   // Toggle vibration
            vibrateRemote(selectedPlayer, vibrateState);
            if (vibrateCount >= 6 && !vibrateState) {  // Stop vibration
                vibrating = false;
            }
        }
        // FOR TESTING
        if (!digitalRead(BUTTON_0) || !digitalRead(BUTTON_1)) {
            showAll(digitalRead(BUTTON_0) ? CRGB::Lime : CRGB::Red);
            gameState = ANSWER;
            Serial.println("> Moving to ANSWER state");

            delay(2000);

            showAllArray(remoteColours);
            gameState = IDLE;
            Serial.println("> Moving to IDLE state");

            delay(1000);

            armedVibrateMillis = millis();
            vibrateAll(true);
            gameState = READY;
            Serial.println("> Buzzers ARMED! Moving to READY state");

        }
    } else if (gameState == READY) {
        if (millis() - armedVibrateMillis > 100) {
            vibrateAll(false);
            indicateAll(false);
        }
    }
}

// -------- Buzzer LED Functions --------
void showAll(CRGB col)
{
    leds[0] = col;
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(MULT_0, mult_0[i]);
        digitalWrite(MULT_1, mult_1[i]);
        digitalWrite(MULT_2, mult_2[i]);
        FastLED.show();
    }
}

void showAllArray(CRGB col[])
{
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(MULT_0, mult_0[i]);
        digitalWrite(MULT_1, mult_1[i]);
        digitalWrite(MULT_2, mult_2[i]);
        leds[0] = col[i];
        FastLED.show();
    }
}

void showOn(int index, CRGB col)
{
    leds[0] = col;
    digitalWrite(MULT_0, mult_0[index]);
    digitalWrite(MULT_1, mult_1[index]);
    digitalWrite(MULT_2, mult_2[index]);
    FastLED.show();
}

// -------- GPIO Expander Functions --------
void EXPANDER_ISR()
{
    interruptFlag = true; // Set a flag to deal with in the main loop, doing anything external here will cause a watchdog timeout
}

// Enable/disable vibration motor
void vibrateRemote(int index, bool state)
{
    OUTPUTS.digitalWrite(remoteVibrateMap[index], state);
}

void vibrateAll(bool state)
{
    for (int i = 0; i < 8; i++) {
        OUTPUTS.digitalWrite(remoteVibrateMap[i], state);
    }
}

// Illuminate indicator on RJ45 jack
void indicateJack(int index, bool state)
{
    OUTPUTS.digitalWrite(remoteIndicatorMap[index], state);
}

void indicateAll(bool state) {
    for (int i = 0; i < 8; i++) {
        OUTPUTS.digitalWrite(remoteIndicatorMap[i], state);
    }
}

// Detect remote connected
bool remoteSense(int index)
{
    return INPUTS.digitalRead(remoteSenseMap[index]);
}

// Check button state
bool remoteButton(int index)
{
    return INPUTS.digitalRead(remoteButtonMap[index]);
}

// -------- OLED Functions --------
void displayLogo() {
    display.clearDisplay();
    display.drawBitmap(0, 0, epd_bitmap_RockChequeLogo, 128, 32, 1);
    display.display();
}