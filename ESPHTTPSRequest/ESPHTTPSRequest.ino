/*
 *  HTTP over TLS (HTTPS) example sketch
 *
 *  This example demonstrates how to use
 *  WiFiClientSecure class to access HTTPS API.
 *  We fetch and display the status of
 *  esp8266/Arduino project continuous integration
 *  build.
 *
 *  Created by Ivan Grokhotkov, 2015.
 *  This example is in public domain.
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
//#include <EEPROM.h>
#include "SSD1306.h"
#include "images.h"

#define network_number 20

ESP8266WebServer *ESPertServer;
int ESPertNumberOfNetworks = 0;
String ESPertNetworks[network_number];
String ESPertContentHeader="";
String ESPertContent="";
String ESPertContentFooter="";
String ESPertSSIDHeader="";
String ESPertSSIDFooter="";

SSD1306 display(0x3c, SDA, SCL);

#define PIXEL_PIN    14    // Digital IO pin connected to the NeoPixels.
#define BTN 12
#define PIXEL_COUNT 480 //30
#define BULB_PIXEL_COUNT 76

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

String ssid ="";
String password ="";

const char* host = "graph.facebook.com";
const int httpsPort = 443;
unsigned long likes_count;
unsigned long current_count;
unsigned char flag = 0;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
//const char* fingerprint = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C";

void drawLogo(){
  display.clear();
  display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits); 
  display.display();
}

void displayLikes(unsigned long likes){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 22, String(likes)+" Likes"); 
  display.display();
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void light(int num, uint32_t c, uint8_t wait){
  if(num > strip.numPixels())
    num = strip.numPixels();
  for(uint16_t i=(strip.numPixels()-num); i<strip.numPixels(); i++){
      strip.setPixelColor(i, c);
      yield();
  }
  strip.show();
  //delay(wait);
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    int countdown = wait;
    strip.setPixelColor(i, c);
    yield();
    strip.show();
  }
  strip.show();
}

void rocketup(uint32_t c, uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels(); i=i+10) {
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    uint16_t j;
    for(j=i; j<i+10; j++){
      strip.setPixelColor(j, c);
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j<i+10; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
  }
}

void rocketdown(uint32_t c, uint8_t wait){
  for(int i=strip.numPixels()-1; i>-10; i=i-10) {
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    int j;
    for(j=i; j>i-10; j--){
      strip.setPixelColor(j, c);
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j>i-10; j--){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
  }
}

void rainbowRocketup(uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels(); i=i+10) {
    uint16_t j;
    for(j=i; j<i+10; j++){
      strip.setPixelColor(j, Wheel((10+j) & 255));
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j<i+10; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
  }
}

void rainbowRocketdown(uint8_t wait){
  for(int i=strip.numPixels()-1; i>-10; i=i-10) {
    int j;
    for(j=i; j>i-10; j--){
      strip.setPixelColor(j, Wheel((10+j) & 255));
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j>i-10; j--){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
  }
}

void chessboxUp(uint32_t c, uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels(); i=i+5) {
    uint16_t j;
    for(j=i; j<i+2; j++){
      strip.setPixelColor(j, c);
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j<i+1; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
    delay(wait);
    yield();
  }
}

void chessboxDown(uint32_t c, uint8_t wait){
  for(int i=strip.numPixels()-1; i>0; i=i-5) {
    int j;
    for(j=i; j>i-2; j--){
      strip.setPixelColor(j, c);
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j>i-1; j--){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
    delay(wait);
    yield();
  }
}

void rainbowChessboxUp(uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels(); i=i+5) {
    uint16_t j;
    for(j=i; j<i+2; j++){
      strip.setPixelColor(j, Wheel((10+j) & 255));
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j<i+1; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
    delay(wait);
  }
}

void ringUp(uint32_t c, uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels(); i=i+20) {
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    uint16_t j;
    for(j=i; j<i+1; j++){
      strip.setPixelColor(j, c);
    }
    strip.show();
    delay(wait);
    yield();
  }
}
void ringDown(uint32_t c, uint8_t wait){
  for(int i=strip.numPixels()-1; i>0; i=i-20) {
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    int j;
    for(j=i; j>i-1; j--){
      strip.setPixelColor(j, c);
    }
    strip.show();
    delay(wait);
    yield();
  }
}
void rainbowRingUp(uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels(); i=i+20) {
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    uint16_t j;
    for(j=i; j<i+1; j++){
      strip.setPixelColor(j, Wheel((10+j) & 255));
    }
    strip.show();
    delay(wait);
    yield();
  }
}

void snakeup(uint8_t len, uint32_t c, uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels()+15*len; i=i+15) {//i=i+20
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    uint16_t j;
    for(j=i; j<i+1; j++){
      strip.setPixelColor(j, c);
    }
    strip.show();
    delay(wait);
    /*for(j=i; j<i+1; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
    }*/
    if(i>=15*len){
       strip.setPixelColor(i-15*len, strip.Color(0, 0, 0));
       strip.show();
       delay(wait);
    }
    yield();
  }
}

void snakeDown(uint8_t len, uint32_t c, uint8_t wait){
  for(int i=strip.numPixels()-1; i>-1*15*len; i=i-15) {//i=i+20
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    int j;
    for(j=i; j>i-1; j--){
      strip.setPixelColor(j, c);
    }
    strip.show();
    delay(wait);
    /*for(j=i; j<i+1; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
    }*/
    int idx = strip.numPixels()-15*len;
    if(i < idx){
       strip.setPixelColor(i+15*len, strip.Color(0, 0, 0));
       strip.show();
       delay(wait);
    }
    yield();
  }
}

void rainbowSnakeUp(uint8_t len,uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels()+15*len; i=i+15) {//i=i+20
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    uint16_t j;
    for(j=i; j<i+1; j++){
      strip.setPixelColor(j, Wheel((10+j) & 255));
    }
    strip.show();
    delay(wait);
    /*for(j=i; j<i+1; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
    }*/
    if(i>=15*len){
       strip.setPixelColor(i-15*len, strip.Color(0, 0, 0));
       strip.show();
       delay(wait);
    }
    yield();
  }
}


void caterpillarUp(uint32_t c, uint8_t wait){
  light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
  for(uint16_t i=0; i<strip.numPixels()+60; i=i+20) {
    uint16_t j;
    for(j=i;j<i+12; j++){
      if(j>(strip.numPixels()-76-1))
        strip.setPixelColor(j, strip.Color(30,0,0));
      else
        strip.setPixelColor(j, c);
      yield();
    }
    strip.show();
    delay(wait);
    if(i >= 60){
      for(j=i-60;j<i+12-60; j++){
        strip.setPixelColor(j, strip.Color(0,0,0));
        yield();
      }
      strip.show();
      yield();
      delay(wait);
    }
  }
}

void caterpillarDown(uint32_t c, uint8_t wait){
  light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
  for(int i=strip.numPixels()-1; i>-60; i=i-20) {
    int j;
    for(j=i;j>i-12; j--){
      if(j>(strip.numPixels()-76-1))
        strip.setPixelColor(j, strip.Color(30,0,0));
      else
        strip.setPixelColor(j, c);
      yield();
    }
    strip.show();
    delay(wait);
    int idx = strip.numPixels()-60;
    if(i < idx){
      for(j=i+60;j>i-12+60; j--){
        strip.setPixelColor(j, strip.Color(0,0,0));
        yield();
      }
      strip.show();
      yield();
      delay(wait);
    }
  }
}

void rainbowCaterpillar(uint8_t wait){
  light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
  for(uint16_t i=0; i<strip.numPixels()+60; i=i+20) {
    uint16_t j;
    for(j=i;j<i+12; j++){
      if(j>(strip.numPixels()-76-1))
        strip.setPixelColor(j, strip.Color(30,0,0));
      else
        strip.setPixelColor(j, Wheel((10+j) & 255));
      yield();
    }
    strip.show();
    delay(wait);
    if(i >= 60){
      for(j=i-60;j<i+12-60; j++){
        strip.setPixelColor(j, strip.Color(0,0,0));
        yield();
      }
      strip.show();
      yield();
      delay(wait);
    }
  }
}

void rollAndStack(uint32_t c, uint8_t wait){
  int idx = strip.numPixels();
  while(idx > PIXEL_COUNT - BULB_PIXEL_COUNT+1){
    yield();
  for(int x=idx; x<strip.numPixels(); x++){ 
     strip.setPixelColor(x, c);
     yield();
  }
  strip.show();
  for(uint16_t i=0; i<idx; i=i+11) {
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    uint16_t j;
    for(j=i; j<i+11; j++){
      strip.setPixelColor(j, c);
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j<i+11; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
  }
    idx = idx - 11;
  }
  light(BULB_PIXEL_COUNT, strip.Color(0, 0, 0), 1);
}

void specialpattern(uint32_t c, uint8_t wait){
  int idx = strip.numPixels();
  while(idx > 0){
  for(uint16_t i=0; i<idx; i=i+11) {
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    uint16_t j;
    for(j=i; j<i+11; j++){
      strip.setPixelColor(j, c);
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j<i+11; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
  }
    idx = idx - 11;
  }
}

void rainbowSpecial(uint8_t wait){
  int idx = strip.numPixels();
  while(idx > 0){
  for(uint16_t i=0; i<idx; i=i+11) {
    //light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
    uint16_t j;
    for(j=i; j<i+11; j++){
      strip.setPixelColor(j,  Wheel((10+j) & 255));
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j<i+11; j++){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
  }
    idx = idx - 11;
  }
}

void rainbowSpecial2(uint8_t wait){
  int idx = 0;
  while(idx < strip.numPixels()){
  for(int i=(strip.numPixels()-idx-1); i>0; i=i-11) {
    yield();
    int j;
    for(j=i; j>i-11; j--){
      strip.setPixelColor(j,  Wheel((10+j) & 255));
      yield();
    }
    strip.show();
    delay(wait);
    for(j=i; j>i-11; j--){
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      yield();
    }
  }
    idx = idx + 11;
  }
}

int getRandomNumber(int startNum, int endNum) {
  randomSeed(ESP.getCycleCount());
  return random(startNum, endNum);
}

void rollStack(){
  rollAndStack(strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)), 10);
}

void chessboxUp(){
  uint8_t choice = ESP.getCycleCount()%2;
  if(choice){
    chessboxUp(strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)), getRandomNumber(10,50));
  }
  else
    rainbowChessboxUp(getRandomNumber(10,50));
  delay(500);
  chessboxUp(strip.Color(0,0,0),10);
}

void snakeUp(){
  uint8_t choice = ESP.getCycleCount()%2;
  if(choice){
    snakeup(6, strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)),  getRandomNumber(10,50));
  }
  else
    rainbowSnakeUp(6, getRandomNumber(10,50));
}

void ringUp(){
  uint8_t choice = ESP.getCycleCount()%2;
  if(choice){
    ringUp(strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)),  getRandomNumber(10,50));
  }
  else
    rainbowRingUp( getRandomNumber(10,50));
  delay(500);
  ringUp(strip.Color(0,0,0),10);
}

void caterpillarUp(){
  uint8_t choice = ESP.getCycleCount()%2;
  if(choice){
    caterpillarUp(strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)),  getRandomNumber(10,50));
  }
  else
    rainbowCaterpillar( getRandomNumber(10,50));
}

void rocketUp(){
  uint8_t choice = ESP.getCycleCount()%2;
  if(choice){
    rocketup(strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)),  getRandomNumber(10,50));
  }
  else
    rainbowRocketup( getRandomNumber(10,50));
}

void chessboxDown(){
  chessboxDown(strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)), getRandomNumber(10,50));
  delay(500);
  chessboxDown(strip.Color(0,0,0),10);
}

void snakeDown(){
    snakeDown(6, strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)),  getRandomNumber(10,50));
}

void ringDown(){
  
  ringDown(strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)),  getRandomNumber(10,50));
  delay(500);
  ringDown(strip.Color(0,0,0),10);
}

void caterpillarDown(){
  caterpillarDown(strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)),  getRandomNumber(10,50));
}

void rocketDown(){
  uint8_t choice = ESP.getCycleCount()%2;
  if(choice){
    rocketdown(strip.Color(getRandomNumber(0,255), getRandomNumber(0,255), getRandomNumber(0,255)),  getRandomNumber(10,50));
  }
  else
    rainbowRocketdown( getRandomNumber(10,50));
}

void (*rising_funcs[6])(void) = {
  rollStack,
  chessboxUp,
  snakeUp,
  ringUp,
  caterpillarUp,
  rocketUp
};

void (*falling_funcs[5])(void)={
  chessboxDown,
  snakeDown,
  ringDown,
  caterpillarDown,
  rocketDown
};

void test(){
  (*rising_funcs[getRandomNumber(0,5)])();
  (*falling_funcs[ESP.getCycleCount()%5])();
  light(PIXEL_COUNT, strip.Color(0, 0, 0), 0);
  //rainbowRocketdown(10);
  //rollAndStack(strip.Color(0, 0, 30), 10);
  //rainbowSpecial(10);
  //rainbowSpecial2(10);
  flag = 0;
}

unsigned long update(){
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return 0;
  }
  StaticJsonBuffer<200> jsonBuffer;

  String url = "/v2.5/cytrontech?fields=likes&access_token=EAAICnyQvv40BAFWl9ZCJCT4AidZBMS1iTnPilsZCe0xLACippBisY7vZBDEiF0lWpTz5Mdg6CvvMQIjSne8ENKEPQtjIIFIfvSDpKUJDahdAGjaPSnPZCs5CkiXZB8f9beGKutr9QM4BGHr0jrPWEseIZAa0QRNOHwZD";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: CytronTechESPressoLitev2.0\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println("closing connection");

  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return 0;
  }
  unsigned long likes = root["likes"];
  String id = root["id"];

  Serial.print("Likes: ");Serial.println(likes);
  Serial.println("==========");
  Serial.print("ID: ");Serial.println(id);
  Serial.println("==========");

  displayLikes(likes);

  return likes;
}

void scanNetworks(){
  //scan wifi
  ESPertNumberOfNetworks = WiFi.scanNetworks();
  if(ESPertNumberOfNetworks > network_number) ESPertNumberOfNetworks = network_number;
  for(int i = 0; i < ESPertNumberOfNetworks; ++i)
    ESPertNetworks[i] = WiFi.SSID(i);
}

void initServer(){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 22, "CytronTech");
  display.display();
  WiFi.disconnect();
  WiFi.softAP("CytronTech");
  delay(2000);
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  display.drawString(64, 38, ipStr);
  display.display();

  String title = String("Hello from CytronTech at ") + ipStr;

  ESPertContentHeader = "";
  ESPertContentHeader += String("<html>\r\n");
  ESPertContentHeader += String("  <head>\r\n");
  ESPertContentHeader += String("    <title>") + title + " </title>\r\n";
  ESPertContentHeader += String("    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>\r\n");
  ESPertContentHeader += String("    <meta name='viewport' content='width=device-width, initial-scale=1'>\r\n");
  ESPertContentHeader += String("  </head>\r\n");
  ESPertContentHeader += String("  <body>\r\n");
  ESPertContentHeader += String("    <div align=center>\r\n");
  ESPertContentHeader += String("      <form id='settings' name='settings' action='setting' method='POST'>\r\n");
  ESPertContentHeader += String("        <table cellspacing=0 cellpadding=2 style='border:thin solid black'>\r\n");
  ESPertContentHeader += String("          <tr style='background-color:#666666; min-width:298px; max-width:298px'><td style='min-width:298px; max-width:298px' align=center colspan=2><font color=#ffffff>") + title + "</font></td></tr>\r\n";
  ESP.wdtFeed();

  ESPertContentFooter = "";
  ESPertContentFooter += String("        </table>\r\n");
  ESPertContentFooter += String("      </form>\r\n");
  ESPertContentFooter += String("    </div>\r\n");
  ESPertContentFooter += String("  </body>\r\n");
  ESPertContentFooter += String("</html>\r\n");
  ESP.wdtFeed();

  ESPertSSIDHeader = "";
  ESPertSSIDHeader += String("          <tr style='background-color:#aaaaaa'>\r\n");
  ESPertSSIDHeader += String("            <td align=right>SSID:</td>\r\n");
  ESPertSSIDHeader += String("            <td>\r\n");
  ESPertSSIDHeader += String("              <select id=ssid name=ssid>\r\n");
  ESPertSSIDHeader += String("                <option value="">Choose a Network</option>\r\n");
  ESP.wdtFeed();

  ESPertSSIDFooter = "";
  ESPertSSIDFooter += String("              </select>\r\n");
  ESPertSSIDFooter += String("            </td>\r\n");
  ESPertSSIDFooter += String("          </tr>\r\n");
  ESP.wdtFeed();

  Serial.println("Starting Server");
  ESPertServer = new ESP8266WebServer(80);

  ESPertServer->on("/", []() {
    Serial.println("Entering Main Page");
    scanNetworks();
    ESPertContent = ESPertContentHeader + ESPertSSIDHeader;

    for (int i = 0; i < ESPertNumberOfNetworks; ++i) {
      ESPertContent += String("                <option value='") + ESPertNetworks[i] + "'" + ((ESPertNetworks[i] == ssid) ? " selected>" : ">") + ESPertNetworks[i] + "</option>\r\n";
      ESP.wdtFeed();
    }

    ESPertContent += ESPertSSIDFooter;
    ESPertContent += String("          <tr style='background-color:#cccccc'><td align=right>Password:</td><td><input type=text id=pass name=pass value='") + password + "'></td></tr>\r\n";
    ESPertContent += String("          <tr><td colspan=2 align=center><input type=submit id=submitButton name=submitButton value='Submit'></td></tr>\r\n");
    ESPertContent += ESPertContentFooter;
    ESP.wdtFeed();

    ESPertServer->send(200, "text/html", ESPertContent);
  });

  ESPertServer->on("/setting", []() {
    Serial.println("Setting done");
    ssid = ESPertServer->arg("ssid");
    password = ESPertServer->arg("pass");
    ssid.replace("+", " ");
    ssid.replace("%40", "@");

    String title = String("Hello from CytronTech");
    
    ESPertContent = String("<html>\r\n");
    ESPertContent += String("  <head>\r\n");
    ESPertContent += String("    <title>") + title + " </title>\r\n";
    ESPertContent += String("    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>\r\n");
    ESPertContent += String("    <meta name='viewport' content='width=device-width, initial-scale=1'>\r\n");
    ESPertContent += String("  </head>\r\n");
    ESPertContent += String("  <body>\r\n");
    
    if (ssid.length() > 0) {

      WiFi.begin(ssid.c_str(), password.c_str());
      int c = 0;
      while(c < 15){
        if (WiFi.status() == WL_CONNECTED) {
          ESPertContent += "WiFi found. Reseting ESP soon..."; 
          break;     
        }
        delay(1000);
        ESP.wdtFeed();
        c++;
      }
      if(c==15 && WiFi.status() != WL_CONNECTED)
        ESPertContent += "WiFi not found, please enter the main page to setup again.";
      ESPertContent += String("  </body>\r\n");
      ESPertContent += String("</html>\r\n");
      ESPertServer->send(200, "text/html", ESPertContent);
      delay(1000);

      if(WiFi.status()== WL_CONNECTED)
        ESP.reset();
    }
    else {
      ESPertContent += "Invalid network, please enter the main page to setup again.";
      ESPertContent += String("  </body>\r\n");
      ESPertContent += String("</html>\r\n");
      ESPertServer->send(200, "text/html", ESPertContent);
    }
  });
  ESPertServer->begin();
  Serial.println("HTTP server started");
  while(1){
    ESPertServer->handleClient();
    if(flag)
      test();
  }
}
/*
String eeprom_read(int index, int length) {
  String text = "";
  char ch = 1;

  for (int i = index; (i < (index + length)) && ch; ++i) {
    if (ch = EEPROM.read(i)) {
      text.concat(ch);
    }
  }

  return text;
}

int eeprom_write(int index, String text) {
  for (int i = index; i < text.length() + index; ++i) {
    EEPROM.write(i, text[i - index]);
  }

  EEPROM.write(index + text.length(), 0);
  EEPROM.commit();

  return text.length() + 1;
}
*/
bool longPress(){
  if(!digitalRead(13)){
    int i = 2000;
    while(!digitalRead(13)){
      i--;
      if(i==0) return true;
    }  
  }
  return false;
}


void setup() {

  //EEPROM.begin(512);
  pinMode(BTN, INPUT_PULLUP);
  WiFi.setAutoConnect(true);
  WiFi.mode(WIFI_STA);
  attachInterrupt(12, []{flag=1;}, FALLING);
  //display
  display.init();
  display.flipScreenVertically();
  drawLogo();

  //led strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  //initial pattern
  chessboxUp(strip.Color(30, 15, 0), 1);
  chessboxDown(strip.Color(30, 15, 0), 1);
  rainbowChessboxUp(1);
  chessboxUp(strip.Color(0, 0, 0), 1);
  chessboxDown(strip.Color(0, 0, 0), 1);
  
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  
  //WiFi.begin(ssid, password);
  int timeout = 15;
  int _stat = WiFi.status();
  while (_stat != WL_CONNECTED && timeout--) {
    delay(500);
    Serial.print(".");
    _stat = WiFi.status();
  }
  
  if(_stat != WL_CONNECTED){
    initServer();
  }
  
  Serial.println("");
  ssid = WiFi.SSID();
  Serial.println("Connected to "+ssid);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
// Clear the local pixel buffer
  likes_count = update();
  delay(1000); 

}

void loop() {

  if(longPress()) initServer();
  if(flag) test();
  /*if(!digitalRead(BTN)){
    delay(500);
    test();
  }*/
  
  current_count = update();

  if(current_count == 0) return;

  if(current_count > likes_count){
    //colorWipe(strip.Color(255, 69, 0), 1);
    light(PIXEL_COUNT, strip.Color(0, 0, 0), 1);
    (*rising_funcs[getRandomNumber(0,5)])();
    for(int i=0;i<3;i++){
      yield();
      light(BULB_PIXEL_COUNT, strip.Color(120, 30, 0), 5);
      delay(1000);
      light(BULB_PIXEL_COUNT, strip.Color(0, 0, 0), 5);
      delay(1000);
    }
    //colorWipe(strip.Color(0, 0, 0), 1);
  }
  else if(current_count < likes_count){
      for(int val = 30; val >= 0;val--){
        light(BULB_PIXEL_COUNT, strip.Color(val*4, val, 0), 5);
        delay(100);
        yield();
      }
      (*falling_funcs[ESP.getCycleCount()%5])();
  }
  
  if(current_count != likes_count){
    likes_count = current_count;
  }
  int delay_time = 5000;
  while(delay_time--){
    if(flag) {
      test();
      break;
    }
    delay(1);
  }
}
