#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <Servo.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//for LED status
#include <Ticker.h>
#include <EEPROM.h>
#include "main.h"

#define OLED_RESET 0  // GPIO0

Ticker ticker;
Ticker showTicker;
Ticker hideTicker;
int id_timer_show;
int id_timer_hide;
AsyncWebServer server(80);
Servo targetServo;
//Adafruit_SSD1306 display(OLED_RESET);
int _eeprom_adr = 0;
boolean fastShoot = false;


const String COUNT_DOWN_KEY = "countDown";
const String SHOW_TARGET_KEY = "showTarget";
const String HIDE_TARGET_KEY = "hideTarget";
const int DEFAULT_COUNT_DOWN = 10;
const int DEFAULT_SHOW_TARGET = 3;
const int DEFAULT_HIDE_TARGET = 7;
const int DEFAULT_SERVO_START = 8;
const int DEFAULT_SERVO_END = 93;
const uint8_t SERVO_PIN = D3;
struct ConfigData cd = { .servoStart=DEFAULT_SERVO_START, .servoStop=DEFAULT_SERVO_END, .fastShootHide=DEFAULT_HIDE_TARGET, .fastShootShow=DEFAULT_SHOW_TARGET};

void checkParameters(AsyncWebServerRequest *request){
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){ //p->isPost() is also true
    Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
    Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
    Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
}

void loadConfigFromEeprom(){
  EEPROM.get(_eeprom_adr, cd);
  if(! (cd.servoStart >=0 || cd.servoStart <= 180)){
    cd.servoStart = DEFAULT_SERVO_START;
  }
  if(! (cd.servoStop >=0 || cd.servoStart <= 180)){
    cd.servoStop = DEFAULT_SERVO_END;
  }
}

void saveConfigToEeprom(){
  EEPROM.put(_eeprom_adr, cd);
  EEPROM.commit();
}

void tick()
{
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

void servoStart(){
  long atTime = millis();
  targetServo.write(cd.servoStart);
  Serial.print("Servo Start: ");
  Serial.println(atTime);
}

void servoStop(){
  long atTime = millis();
  targetServo.write(cd.servoStop);
  Serial.print("Servo Stop: ");
  Serial.println(atTime);
}

void setServoPos(int pos){
  Serial.print("Setting servo pos at: ");
  Serial.println(pos);
  if(0<pos || pos < 180){
    targetServo.write(pos);
    Serial.println("Servo in new position");

  }
  Serial.flush();
}

void showTarget(){
  servoStart();
}

void hideTarget(){
  servoStop();
}
void fastshoot_hide(){
  hideTarget();
  hideTicker.detach();
  showTicker.attach(cd.fastShootHide, fastshoot_show);
}

void fastshoot_show(){
  showTarget();
  showTicker.detach();
  hideTicker.attach(cd.fastShootShow, fastshoot_hide);

}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void defaultPage(AsyncWebServerRequest *request) {
  showTicker.detach();
  hideTicker.detach();

  String page = FPSTR(MY_HTTP_HEAD);
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);
  page += FPSTR(HTTP_BODY_TEXT);
  page += FPSTR(HTTP_SHOOT_FORM);
  page += FPSTR(HTTP_END);
  request->send(200, "text/html", page);

}

void config(AsyncWebServerRequest *request){
  String startpos=String(cd.servoStart);
  String endpos=String(cd.servoStop);
  String showTime = String(cd.fastShootShow);
  String hideTime = String(cd.fastShootHide);
  checkParameters(request);
  if(request->hasParam("startpos")){
    startpos = request->getParam("startpos")->value();
  }
  if(request->hasParam("endpos")){
    endpos = request->getParam("endpos")->value();
  }
  if(request->hasParam("showTime")){
    showTime = request->getParam("showTime")->value();
  }
  if(request->hasParam("hideTime")){
    hideTime = request->getParam("hideTime")->value();
  }
  cd.servoStart = startpos.toInt();
  cd.servoStop = endpos.toInt();
  cd.fastShootHide = hideTime.toInt();
  cd.fastShootShow = showTime.toInt();
  saveConfigToEeprom();
  Serial.println("Start pos saved:" + startpos);
  Serial.println("End pos saved: " + endpos);
  Serial.println("Showtime saved: " + showTime);
  Serial.println("Hidetime saved: " + hideTime);

  String page = FPSTR(MY_HTTP_HEAD);
  page+= FPSTR(HTTP_STYLE);
  page+= FPSTR(HTTP_CONFIG_SCRIPT);
  page+= FPSTR(HTTP_HEAD_END);
  page+= FPSTR(HTTP_CONFIG_BODY);
  page+= FPSTR(HTTP_SERVO_CONFIG_FORM);
  page+= FPSTR(HTTP_END);
  page.replace("{{startpos}}", startpos);
  page.replace("{{endpos}}", endpos);
  page.replace("{{showTime}}", showTime);
  page.replace("{{hideTime}}", hideTime);
  request->send(200, "text/html", page);
}

void snabbskytte(AsyncWebServerRequest *request){
  //hideTarget();
  String page = FPSTR(MY_HTTP_HEAD);
  page+= FPSTR(HTTP_STYLE);
  page+= FPSTR(HTTP_HEAD_END);
  page+= FPSTR(HTTP_FASTSHOOTING_BODY);
  page+= FPSTR(HTTP_RETURN_FORM);
  page+= FPSTR(HTTP_END);
  request->send(200, "text/html", page);
  //id_timer_show = rotateTimer.every(7000, showTarget);
  //id_timer_hide = rotateTimer.every(10000, hideTarget);
  fastshoot_show();
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(3000);
  EEPROM.begin(128);
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

  //load start & stop position for servo
  loadConfigFromEeprom();
  Serial.println("Start pos from eeprom: " + cd.servoStart);
  Serial.println("End pos from eeprom: " + cd.servoStop);
  //set led pin as output
  pinMode(LED_BUILTIN, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.mode(WIFI_AP);
  WiFi.softAP("shooter1");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println(WiFi.getAutoConnect());
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  //showOnDisplay("Connected to: \n" + WiFi.SSID() + "\nas \n"+ WiFi.localIP().toString());
  ticker.detach();
  MDNS.begin("shooter");
  //keep LED on
  digitalWrite(LED_BUILTIN, LOW);
  server.begin();
  Serial.println("Http server started.");
  targetServo.attach(SERVO_PIN);
  delay(1000);


    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        defaultPage(request);
    });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
      if(request->hasParam("save_btn")){
        config(request);
      } else if (request->hasParam("reset")){
        cd.fastShootHide = DEFAULT_HIDE_TARGET;
        cd.fastShootShow = DEFAULT_SHOW_TARGET;
        cd.servoStart = DEFAULT_SERVO_START;
        cd.servoStop = DEFAULT_SERVO_END;
        config(request);
      } else {
        request->redirect("/");
      }
    });

    server.on("/setPosition", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("position")){
          String value = request->getParam("position")->value();
          setServoPos(value.toInt());
        }
        request->send(200, "text/plain", "ok");
    });

    server.on("/choice", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("showTarget")){
          showTarget();
          request->redirect("/");
        }
        if(request->hasParam("hideTarget")){
          hideTarget();
          request->redirect("/");
        }
        if(request->hasParam("snabbskytte")){
          snabbskytte(request);
        }
        if(request->hasParam("config")){
          config(request);
        }

    });

    server.onNotFound(notFound);


}

void loop(void) {
  //server.handleClient();
  MDNS.update();
}
