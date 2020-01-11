#include "SevSeg.h"
#include <string.h> //used for some string handling and processing.
#include <Time.h>
#include <WiFi.h> //Connect to WiFi Network

SevSeg sevseg;

const int RESPONSE_TIMEOUT = 6000;
// fetch time once a minute
const int GETTING_PERIOD = 60 * 1000;
const uint16_t IN_BUFFER_SIZE = 1000;
const uint16_t OUT_BUFFER_SIZE = 1000;
char request_buffer[IN_BUFFER_SIZE];
char response_buffer[OUT_BUFFER_SIZE];

uint32_t loop_controller; //used for timing
uint32_t last_time;       //used for timing

char network[] = "HackMIT";
char password[] = "foracause!";

char time_str[5];

void setup()
{
  init_sevseg();
  init_wifi();
}

void init_sevseg()
{
  byte numDigits = 4;
  byte digitPins[] = {27, 14, 12, 13};
  byte segmentPins[] = {26, 23, 19, 18, 32, 25, 33, 34};
  bool resistorsOnSegments = false;     // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false;        // Default 'false' is Recommended
  bool leadingZeros = false;            // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false;         // Use 'true' if your decimal point doesn't exist or isn't connected. Then, you only need to specify 7 segmentPins[]

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
               updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(125);
  sprintf(time_str, "0000");
}

void init_wifi()
{
  Serial.begin(115200);          //begin serial
  delay(100);                    //wait a bit (100 ms)
  WiFi.begin(network, password); //attempt to connect to wifi
  uint8_t count = 0;             //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 6)
  {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected())
  { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    delay(500);
  }
  else
  { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  randomSeed(analogRead(A0)); //"seed" random number generator
}

/*-----------------------------------
 * Generate a request to the numbersapi server for a random number
 * Display the response both in the Serial Monitor
*/
void loop()
{
  sevseg.setChars(time_str);
  sevseg.refreshDisplay();
  if ((millis() - last_time) > GETTING_PERIOD)
  {
    sprintf(request_buffer, "GET http://worldtimeapi.org/api/timezone/America/New_York.txt HTTP/1.1\r\n");
    strcat(request_buffer, "Host: worldtimeapi.org\r\n"); //add more to the end
    strcat(request_buffer, "\r\n");                       //add blank line!

    do_http_GET("worldtimeapi.org", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
    // Serial.println(response_buffer);
    parse_time();
    last_time = millis();
  }
}

void parse_time()
{
  Serial.println("parsing time ~~~~~~~~~~~~~");
  int time_start = 10;
  int first_colon = 13;
  int second_colon = 16;
  int hyphen = 26;

  char *line = strstr(response_buffer, "datetime: ");
  char *end_of_line = strtok(line, "\n");

  // shift forward for "datetime: "
  line = line + 10;
  line[first_colon] = '\0';
  line[second_colon] = '\0';
  line[hyphen] = '\0';
  int hour = atoi(line + time_start + 1);
  if (hour > 12)
    hour -= 12;
  int minute = atoi(line + first_colon + 1);
  float second = atof(line + second_colon + 1);

  sprintf(time_str, "%02d%02d", hour, minute);
  Serial.println(time_str);
}

uint8_t char_append(char *buff, char c, uint16_t buff_size)
{
  int len = strlen(buff);
  if (len > buff_size)
    return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}

void do_http_GET(char *host, char *request, char *response, uint16_t response_size, uint16_t response_timeout, uint8_t serial)
{
  WiFiClient client;
  if (client.connect(host, 80))
  {
    if (serial)
      Serial.print(request);
    client.print(request);
    memset(response, 0, response_size);
    uint32_t count = millis();
    while (client.connected())
    { //while we remain connected read out data coming back
      client.readBytesUntil('\n', response, response_size);
      if (serial)
        Serial.println(response);
      if (strcmp(response, "\r") == 0)
      { //found a blank line! (end of response header)
        break;
      }
      memset(response, 0, response_size);
      if (millis() - count > response_timeout)
        break;
    }
    memset(response, 0, response_size); //empty in prep to store body
    count = millis();
    while (client.available())
    { //read out remaining text (body of response)
      char_append(response, client.read(), OUT_BUFFER_SIZE);
    }
    if (serial)
      Serial.println(response);
    client.stop();
    if (serial)
      Serial.println("-----------");
  }
  else
  {
    if (serial)
      Serial.println("connection failed :/");
    if (serial)
      Serial.println("wait 0.5 sec...");
    client.stop();
  }
}
