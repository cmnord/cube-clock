#include "SevSeg.h"

SevSeg sevseg;

void setup()
{

  byte numDigits = 4;
  byte digitPins[] = {27, 14, 12, 13};
  // byte segmentPins[] = {9, 2, 3, 5, 6, 8, 7, 4};
  byte segmentPins[] = {26, 23, 22, 21, 32, 25, 33, 34};
  bool resistorsOnSegments = false;     // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false;        // Default 'false' is Recommended
  bool leadingZeros = false;            // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false;         // Use 'true' if your decimal point doesn't exist or isn't connected. Then, you only need to specify 7 segmentPins[]

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
               updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(125);
}

void loop()
{
  sevseg.setNumber(8888, 3);
  sevseg.refreshDisplay();
}
