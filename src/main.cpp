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
Ticker ticker;
Timer timer_delay;
Timer timer_show;
AsyncWebServer server(80);
Servo targetServo;
DNSServer dns;

void tick()
{
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void servoStart(){
  targetServo.write(10);
}

void servoStop(){
  targetServo.write(170);
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void defaultPage(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print("<DOCTYPE html><html><head><title>Pistol Practice</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  response->print("<style>body {font-size: 24px; font-family: verdata, sans-serif; }#button {width: auto; height: 2em; font-size: 24px; font-family: Verdana, sans-serif;}</style></head><body>");
  response->print("<h1>Welcome to the Pistol shooting practice</h1>");
  response->print("<p>Enter number of seconds for countdown and number of seconds for target to show.</p>");
  response->print("<form action=\"shoot\"><label for=\"countDown\">Timer sec:</><br/><input type=\"number\" id=\"countDown\" name=\"countDown\" value=\"9\"><br/>");
  response->print("<label for=\"showTarget\">Show Target sec:</><br/><input type=\"number\" id=\"showTarget\" name=\"showTarget\" value=\"4\"></br>");
  response->print("<input type=\"submit\" value=\"Start\" id=\"button\"></form>");
  response->print("</body></html>");
  request->send(response);
}

void showTarget(AsyncWebServerRequest *request){
  String countDown = request->getParam("countDown")->value();
  String showFor = request->getParam("showTarget")->value();
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
  response->print("if (distance < 0) {    clearInterval(x);    document.getElementById(\"demo\").innerHTML=\"SHOOT\";  }");
response->print("}, 1);");
response->print("</script><p><form action=\"/\"><input type=\"submit\" value=\"Shoot again\" id=\"button\"/></form></p></body></html>");
request->send(response);
Serial.print("Count down (s): ");
Serial.println(countDown);
Serial.print("Show for (s): ");
Serial.println(showFor);
timer_delay.after(countDown.toInt() * 1000, servoStop);
timer_show.after((showFor.toInt()+countDown.toInt()) * 1000, servoStart);
}

// Define routing
/**
void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><body><h1>Welcome to the Pistol shooting practice<br><h2><a href=\"shoot\">Shoot</a> </body></html>"));

    });
    server.on(F("/helloWorld"), HTTP_GET, getHelloWord);
    server.on(F("/shoot"), HTTP_GET, showTarget);
}
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

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
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();
  MDNS.begin("shooter");
  //keep LED on
  digitalWrite(LED_BUILTIN, LOW);
  server.begin();
  Serial.println("Http server started.");
  targetServo.attach(2);
  targetServo.write(170);
  delay(1000);
  targetServo.write(10);


    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        defaultPage(request);
    });

    server.on("/shoot", HTTP_GET, [](AsyncWebServerRequest *request){
        showTarget(request);

    });

    server.onNotFound(notFound);


}

void loop(void) {
  //server.handleClient();
  MDNS.update();
  timer_delay.update();
  timer_show.update();
}
