#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESPAsyncWiFiManager.h>
#include <Servo.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include "ESPAsyncWebServer.h"
//for LED status
#include <Ticker.h>
#include <Timer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>

#define OLED_RESET 0  // GPIO0

Ticker ticker;
Timer timer_show;
AsyncWebServer server(80);
Servo targetServo;
DNSServer dns;
Adafruit_SSD1306 display(OLED_RESET);


const String COUNT_DOWN_KEY = "countDown";
const String SHOW_TARGET_KEY = "showTarget";
const int DEFAULT_COUNT_DOWN = 10;
const int DEFAULT_SHOW_TARGET = 4;
const uint8_t SERVO_PIN = D3;
const int SERVO_START_POS = 8;
const int SERVO_END_POS = 93;


void tick()
{
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

void showOnDisplay(String text){
  display.clearDisplay();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1,0);
  display.println(text);
  display.display();
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  String ip = WiFi.softAPIP().toString();
  Serial.println(ip);
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  showOnDisplay("Config mode: \n" + ip + "\n" + myWiFiManager->getConfigPortalSSID());
  ticker.attach(0.2, tick);
}

void servoStart(){
  targetServo.write(SERVO_START_POS);
}

void servoStop(){
  targetServo.write(SERVO_END_POS);
}

void showTarget(AsyncWebServerRequest *request){
  String showFor = request->getParam(SHOW_TARGET_KEY)->value();
  request->send(200, "text/plain", "ok");
  servoStop();
  timer_show.after(showFor.toInt()* 1000, servoStart);
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void defaultPage(AsyncWebServerRequest *request) {
  String countDown = String(DEFAULT_COUNT_DOWN);
  String showTarget = String(DEFAULT_SHOW_TARGET);

  if(request->hasParam(COUNT_DOWN_KEY)){
    countDown = request->getParam(COUNT_DOWN_KEY)->value();
  }
  if(request->hasParam(SHOW_TARGET_KEY)){
    showTarget = request->getParam(SHOW_TARGET_KEY)->value();
  }
  Serial.println("Showtarget = " + showTarget);
  Serial.println("countDown = " + countDown);
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print("<DOCTYPE html><html><head><title>Pistol Practice</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  response->print("<style>h1 {font-size: 36px; } body {font-size: 24px; font-family: verdata, sans-serif; }#button {width: auto; height: 2em; font-size: 24px; font-family: Verdana, sans-serif;}</style></head><body>");
  response->print("<h1>Welcome Pistol shooting practice</h1>");
  response->print("<p>Enter number of seconds for countdown and number of seconds for target to show.</p>");
  response->print("<form action=\"countDown\"><label for=\"countDown\">Timer sec:</><br/><input type=\"number\" id=\"countDown\" name=\"countDown\" value=\"" + countDown + "\"><br/>");
  response->print("<label for=\"showTarget\">Show Target sec:</><br/><input type=\"number\" id=\"showTarget\" name=\"showTarget\" value=\"" + showTarget + "\"></br>");
  response->print("<input type=\"submit\" value=\"Start\" id=\"button\"></form>");
  response->print("</body></html>");
  request->send(response);
}

void countDown(AsyncWebServerRequest *request){
  String countDown = request->getParam(COUNT_DOWN_KEY)->value();
  String showFor = request->getParam(SHOW_TARGET_KEY)->value();
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print("<!DOCTYPE HTML><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  response->print("<style>p {  text-align: center;  font-size: 60px;  margin-top: 0px;}#button {width: auto; height: 2em; font-size: 24px; font-family: Verdana, sans-serif;}");
  response->print("</style></head><body><p id=\"demo\"></p>");
  response->print("<script>");
  // Set the date we're counting down to
  response->print("var countDownDate = new Date().getTime()+" + countDown + "000;");
  // Update the count down every 1 second
  response->print("var x = setInterval(function() {");

  // Get today's date and time
  response->print("var now = new Date().getTime();");

  // Find the distance between now and the count down date
  response->print("var distance = countDownDate - now;");

  // Time calculations for days, hours, minutes and seconds

  response->print("var seconds = Math.floor((distance % (1000 * 60)) / 1000);");

  // Output the result in an element with id="demo"
  response->print("document.getElementById(\"demo\").innerHTML = seconds;");

  // If the count down is over, write some text
  response->print("if (distance < 0) {    clearInterval(x);   var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange=function(){ ");
  response->print("if(this.readyState == 4 && this.status == 200) { document.getElementById(\"demo\").innerHTML=\"SHOOT\"; }}; xhttp.open(\"GET\", \"showTarget?" + SHOW_TARGET_KEY + "=" + showFor + "\", true); xhttp.send();}");
  response->print("}, 1);");
  response->print("</script><p><form action='/'><input type=\"submit\" value=\"Shoot again\" id=\"button\"/>");
  response->print("<input type='hidden' name=\"" + COUNT_DOWN_KEY + "\" value=\"" + countDown + "\"/><input type='hidden' name=\"" + SHOW_TARGET_KEY + "\" value=\"" + showFor + "\"/></form></p></body></html>");
  request->send(response);
  Serial.print("Count down (s): ");
  Serial.println(countDown);
  Serial.print("Show for (s): ");
  Serial.println(showFor);

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

  //set led pin as output
  pinMode(LED_BUILTIN, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  AsyncWiFiManager wifiManager(&server,&dns);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    showOnDisplay("Connection failure");
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  showOnDisplay("Connected to: \n" + WiFi.SSID() + "\nas \n"+ WiFi.localIP().toString());
  ticker.detach();
  MDNS.begin("shooter");
  //keep LED on
  digitalWrite(LED_BUILTIN, LOW);
  server.begin();
  Serial.println("Http server started.");
  targetServo.attach(SERVO_PIN);
  targetServo.write(SERVO_END_POS);
  delay(1000);
  targetServo.write(SERVO_START_POS);


    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        defaultPage(request);
    });

    server.on("/countDown", HTTP_GET, [](AsyncWebServerRequest *request){
        countDown(request);

    });

    server.on("/showTarget", HTTP_GET, [](AsyncWebServerRequest * request){
      showTarget(request);
    });

    server.onNotFound(notFound);


}

void loop(void) {
  //server.handleClient();
  MDNS.update();
  timer_show.update();
}
