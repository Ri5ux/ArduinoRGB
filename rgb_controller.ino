#include <SoftwareSerial.h>
#include <EEPROM.h>

const byte lights[360]={
  0,   0,   0,   0,   0,   1,   1,   2, 
  2,   3,   4,   5,   6,   7,   8,   9, 
 11,  12,  13,  15,  17,  18,  20,  22, 
 24,  26,  28,  30,  32,  35,  37,  39, 
 42,  44,  47,  49,  52,  55,  58,  60, 
 63,  66,  69,  72,  75,  78,  81,  85, 
 88,  91,  94,  97, 101, 104, 107, 111, 
114, 117, 121, 124, 127, 131, 134, 137, 
141, 144, 147, 150, 154, 157, 160, 163, 
167, 170, 173, 176, 179, 182, 185, 188, 
191, 194, 197, 200, 202, 205, 208, 210, 
213, 215, 217, 220, 222, 224, 226, 229, 
231, 232, 234, 236, 238, 239, 241, 242, 
244, 245, 246, 248, 249, 250, 251, 251, 
252, 253, 253, 254, 254, 255, 255, 255, 
255, 255, 255, 255, 254, 254, 253, 253, 
252, 251, 251, 250, 249, 248, 246, 245, 
244, 242, 241, 239, 238, 236, 234, 232, 
231, 229, 226, 224, 222, 220, 217, 215, 
213, 210, 208, 205, 202, 200, 197, 194, 
191, 188, 185, 182, 179, 176, 173, 170, 
167, 163, 160, 157, 154, 150, 147, 144, 
141, 137, 134, 131, 127, 124, 121, 117, 
114, 111, 107, 104, 101,  97,  94,  91, 
 88,  85,  81,  78,  75,  72,  69,  66, 
 63,  60,  58,  55,  52,  49,  47,  44, 
 42,  39,  37,  35,  32,  30,  28,  26, 
 24,  22,  20,  18,  17,  15,  13,  12, 
 11,   9,   8,   7,   6,   5,   4,   3, 
  2,   2,   1,   1,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0};


const int LED_R = 9;
const int LED_G = 10;
const int LED_B = 11;

/**
 * COMMAND REFERENCE
 * 
 * R <value>  - Set the RED value. (0 - 254)
 * G <value>  - Set the GREEN value. (0 - 254)
 * B <value>  - Set the BLUE value. (0 - 254)
 * RGB SET <r> <g> <b>  - Set the RED, GREEN, and BLUE values at once. (0 - 254)
 * RGB GET  - Get the current values for RED, GREEN, and BLUE. (0 - 254)
 * SPEED <value>  - Set the update speed of the LED modes in updates per millisecond
 * MODE <value> - Set the current LED mode. List specified below. (0 - 4)
 * FADEUPI <value> - Set the incremental value for the fade modes during the fade UP phase. (0 - 254)
 * FADEDOI <value> - Set the incremental value for the fade modes during the fade DOWN phase. (0 - 254)
 * DUALCOLOR <r1> <g1> <b1> <r2> <g2> <b2>  - Set RGB1 and RGB2 for the dual color fade mode. (0 - 254)
 * RESET  - Reset the RGB controller.
 */

/*
 * 0 - Manual RGB
 * 1 - Sinewave Rainbow
 * 2 - Manual RGB with Fade-In-Out
 * 3 - Rainbow with Fade-In-Out
 * 4 - Dual Color Fade-In-Out Instant
 * 5 - Dual Color Fade-In-Out
 */
byte mode = 1;
long updateTimestamp;
int rgbSpeed = 5; /** Update color every x milliseconds **/
int rainbowAngle = 0;
short fadeUpInc = 2;
short fadeDownInc = 2;
short fadeProgress = 0;
boolean fadeInvert;
short lastRandom;
byte rVal = 60;
byte gVal = 0;
byte bVal = 255;
byte* dualColor1 = new byte[3]{255, 0, 0};
byte* dualColor2 = new byte[3]{0, 0, 255};
boolean firstColor = true;
boolean btTerminal = false;

const int BLUETOOTH_RX = 2;
const int BLUETOOTH_TX = 3;

SoftwareSerial bluetooth(BLUETOOTH_RX, BLUETOOTH_TX);

void(* resetFunc) (void) = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  //writeDataToEEPROM();
  readDataFromEEPROM();
  updateRGB();

  Serial.println("RGB Controller initialized");
  bluetooth.begin(9600);
  Serial.println("Bluetooth adapter connection initiated at 9600 BAUD");
}

byte readValueFromEEPROM(String valueName, int address) {
  byte value = EEPROM.read(address);
  Serial.print("[");Serial.print(address);Serial.print("]");Serial.print("[");Serial.print(value);Serial.print("]");Serial.println(valueName);

  return value;
}

void writeValueToEEPROM(String valueName, byte value, int address) {
  EEPROM.write(address, value);
  Serial.print("[");Serial.print(address);Serial.print("]");Serial.print("[");Serial.print(value);Serial.print("]");Serial.println(valueName);
}

void readDataFromEEPROM() {
  Serial.println("Reading data from internal EEPROM...");
  int ADDR = 0;
  
  mode = readValueFromEEPROM("MODE", ADDR++);
  rgbSpeed = readValueFromEEPROM("SPEED", ADDR++);
  rVal = readValueFromEEPROM("R", ADDR++);
  gVal = readValueFromEEPROM("G", ADDR++);
  rVal = readValueFromEEPROM("B", ADDR++);
  fadeUpInc = readValueFromEEPROM("FADE_UP_INC", ADDR++);
  fadeDownInc = readValueFromEEPROM("FADE_DOWN_INC", ADDR++);
  dualColor1[0] = readValueFromEEPROM("DUAL_COLOR_1_R", ADDR++);
  dualColor1[1] = readValueFromEEPROM("DUAL_COLOR_1_G", ADDR++);
  dualColor1[2] = readValueFromEEPROM("DUAL_COLOR_1_B", ADDR++);
  dualColor2[0] = readValueFromEEPROM("DUAL_COLOR_1_R", ADDR++);
  dualColor2[1] = readValueFromEEPROM("DUAL_COLOR_1_G", ADDR++);
  dualColor2[2] = readValueFromEEPROM("DUAL_COLOR_1_B", ADDR++);
}

void writeDataToEEPROM() {
  Serial.println("Writing data to internal memory...");
  int ADDR = 0;
  
  writeValueToEEPROM("MODE", (mode), ADDR++);
  writeValueToEEPROM("SPEED", (rgbSpeed), ADDR++);
  writeValueToEEPROM("R", (rVal), ADDR++);
  writeValueToEEPROM("G", (gVal), ADDR++);
  writeValueToEEPROM("B", (rVal), ADDR++);
  writeValueToEEPROM("FADE_UP_INC", (fadeUpInc), ADDR++);
  writeValueToEEPROM("FADE_DOWN_INC", (fadeDownInc), ADDR++);
  writeValueToEEPROM("DUAL_COLOR_1_R", (dualColor1[0]), ADDR++);
  writeValueToEEPROM("DUAL_COLOR_1_G", (dualColor1[1]), ADDR++);
  writeValueToEEPROM("DUAL_COLOR_1_B", (dualColor1[2]), ADDR++);
  writeValueToEEPROM("DUAL_COLOR_1_R", (dualColor2[0]), ADDR++);
  writeValueToEEPROM("DUAL_COLOR_1_G", (dualColor2[1]), ADDR++);
  writeValueToEEPROM("DUAL_COLOR_1_B", (dualColor2[2]), ADDR++);
}

void processBluetoothData() {
  if (btTerminal) {
    if (Serial.available()) {
      bluetooth.write(Serial.read());
    }
  }
}

void loop() {
  processBluetoothData();
  processSerialTerminal();
  updateModes();
}

void updateModes()
{
  if ((millis() - updateTimestamp) >= rgbSpeed)
  {
    updateTimestamp = millis();
    
    if (mode == 1) updateRainbow();
    if (mode == 2 || mode == 3 || mode == 4 || mode == 5) updateFade();
  }
}

String readStringUntil(byte i, const char ch)
{
  switch (i)
  {
    case 0:
      return Serial.readStringUntil(ch);
    break;
    case 1:
      return bluetooth.readStringUntil(ch);
    break;
    default:
      return Serial.readStringUntil(ch);
    break;
  }
}

template <class T> T print(byte i, const T str)
{
  Serial.print(str);
  bluetooth.print(str);
}

template <class T> T println(byte i, const T str)
{
  Serial.println(str);
  bluetooth.println(str);
}

void processSerialTerminal() {
  int serialAvailable = Serial.available();
  int bluetoothAvailable = bluetooth.available();

  if (serialAvailable > 0 && !btTerminal || bluetoothAvailable > 0) {
    for (int i = 0; i < 2; i++) {
      String command = readStringUntil(i, ' ');
      
      if(command == "r") {
        rVal = readStringUntil(i, '\n').toInt();
        print(i, "R set ");
        println(i, rVal);
        updateRGB();
      } else if(command == "g") {
        gVal = readStringUntil(i, '\n').toInt();
        print(i, "G set ");
        println(i, gVal);
        updateRGB();
      } else if(command == "b") {
        bVal = readStringUntil(i, '\n').toInt();
        print(i, "B set ");
        println(i, bVal);
        updateRGB();
      } else if(command == "rgb") {
        String subCommand = readStringUntil(i, ' ');

        if (subCommand == "set") {
          int r = readStringUntil(i, ' ').toInt();
          int g = readStringUntil(i, ' ').toInt();
          int b = readStringUntil(i, '\n').toInt();
          printRGBValues(i);
          setRGB(r, g, b);
          updateRGB();
        } else if (subCommand == "get") {
          printRGBValues(i);
        }
      } else if(command == "speed") {
        int val = readStringUntil(i, '\n').toInt();
        print(i, "SPEED set to ");
        println(i, val);
        rgbSpeed = val;
      } else if(command == "mode") {
        int val = readStringUntil(i, '\n').toInt();
        print(i, "MODE set to ");
        println(i, val);
        mode = val;
      } else if(command == "save") {
        writeDataToEEPROM();
      } else if(command == "bt_terminal") {
        btTerminal = !btTerminal;
        bluetooth.begin(9600);
        println(i, "Enabled BT terminal forwarding");
      } else if(command == "fadeupi") {
        short val = readStringUntil(i, '\n').toInt();
        print(i, "FADE_UP_INCREMENT set to ");
        println(i, val);
        fadeUpInc = val;
      } else if(command == "fadedoi") {
        short val = readStringUntil(i, '\n').toInt();
        print(i, "FADE_DOWN_INCREMENT set to ");
        println(i, val);
        fadeDownInc = val;
      } else if(command == "dualcolor") {
        short r1 = readStringUntil(i, ' ').toInt();
        short g1 = readStringUntil(i, ' ').toInt();
        short b1 = readStringUntil(i, ' ').toInt();
        short r2 = readStringUntil(i, ' ').toInt();
        short g2 = readStringUntil(i, ' ').toInt();
        short b2 = readStringUntil(i, '\n').toInt();
        println(i, "DUAL_COLOR set.");
        dualColor1 = new byte[3] {r1, g1, b1};
        dualColor2 = new byte[3] {r2, g2, b2};
      } else if(command == "reset") {
        resetFunc();
      }
    }
  }
}

void updateRainbow()
{
  if (!rainbowAngle < 360) {
    rainbowAngle++;
  }
  
  if (rainbowAngle >= 360) {
    rainbowAngle = 0;
    //Serial.print("Cycle reset ");
    //Serial.println(updateTimestamp);
  }
  
  analogWrite(LED_R, rVal = sineR(rainbowAngle));
  analogWrite(LED_G, gVal = sineG(rainbowAngle));
  analogWrite(LED_B, bVal = sineB(rainbowAngle));
}

void updateFade()
{
  if (fadeProgress >= 254) {
    fadeInvert = true;
  }
  
  if (fadeProgress <= 0) {
    fadeInvert = false;

    if (mode == 3) {
      lastRandom = generateRandomColor(lastRandom);
    }

    if (mode == 4) {
      rVal = dualColor2[0];
      gVal = dualColor2[1];
      bVal = dualColor2[2];
    }

    if (mode == 5) {
      firstColor = !firstColor;
        
      if (firstColor) {
        rVal = dualColor1[0];
        gVal = dualColor1[1];
        bVal = dualColor1[2];
      } else {
        rVal = dualColor2[0];
        gVal = dualColor2[1];
        bVal = dualColor2[2];
      }
    }
  }
    
  if (!fadeInvert && fadeProgress <= 254) {
    short proposedValue = fadeProgress + fadeUpInc;
    fadeProgress = proposedValue > 254 ? 254 : proposedValue;
  }
  
  if (fadeInvert && fadeProgress > 0) {
    short proposedValue = fadeProgress - fadeDownInc;
    fadeProgress = proposedValue < 0 ? 0 : proposedValue;
  }
  
  analogWrite(LED_R, calculateBrightness(rVal, fadeProgress));
  analogWrite(LED_G, calculateBrightness(gVal, fadeProgress));
  analogWrite(LED_B, calculateBrightness(bVal, fadeProgress));
}

byte calculateBrightness(byte x, byte brightness) {
  return x * brightness / 254;
}

void printRGBValues(byte i)
{
  print(i, "RGB ");
  print(i, rVal);
  print(i, " ");
  print(i, gVal);
  print(i, " ");
  println(i, bVal);
}

void updateRGB()
{
  // Common anode only (255 - color). For common cathode, flip the output.
  analogWrite(LED_R, rVal);
  analogWrite(LED_G, gVal);
  analogWrite(LED_B, bVal);
}

void setRGB(byte r, byte g, byte b)
{
  rVal = r;
  gVal = g;
  bVal = b;
}

short generateRandomColor(short lastRand)
{
  int newRandom = random(360);

  if (newRandom != lastRand && abs(lastRand - newRandom) >= 45)
  {
    rVal = sineR(newRandom);
    gVal = sineG(newRandom);
    bVal = sineB(newRandom);
    return lastRand = newRandom;
  }
  else
  {
    return generateRandomColor(lastRand);
  }
}

byte sineR(short angle) { return lights[(angle+120)%360]; }
byte sineG(short angle) { return lights[angle]; }
byte sineB(short angle) { return lights[(angle+240)%360]; }
