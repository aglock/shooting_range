#ifndef main_h
#define main_h

#if !defined(ESP8266)
#include <ESP8266.h>
#endif
const char MY_HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>Pistol shooting</title>";
//"<style>h1 {font-size: 36px; } body {font-size: 24px; font-family: verdata, sans-serif; }#button {width: auto; height: 2em; font-size: 24px; font-family: Verdana, sans-serif;}</style></head><body>"
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_BODY_TEXT[] PROGMEM       = "<h1>Pistol shooting practice</h1><p>Fire away or Chose program</p>";
const char HTTP_SHOOT_FORM[] PROGMEM      = "<form action=\"choice\"><input type=\"submit\" id=\"showTarget\" name=\"showTarget\" value=\"Visa m&aring;l\"><br/><input type=\"submit\" id=\"hideTarget\" name=\"hideTarget\" value=\"D&ouml;lj m&aring;l\"><br/><input type=\"submit\" id=\"Snabbskytte\" name=\"snabbskytte\" value=\"Snabbskytte\"><br/><input type=\"submit\" id=\"Konfigurera\" name=\"config\" value=\"Konfigurera\"></form>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";

const char HTTP_FASTSHOOTING_BODY[] PROGMEM = "<h1>Pistol shooting practice</h1><p>Snabbskytte, m&aring;l visning 3 sek, g&ouml;mt i 7sek.</p>";
const char HTTP_RETURN_FORM[] PROGMEM     = "<form action=\"/\"><input type=\"submit\" id=\"return\" name=\"return\" value=\"Till meny\"/></form>";

const char HTTP_CONFIG_BODY[] PROGMEM     ="<h1>Pistol shooting configuration</h1><p>G&ouml;r inst&auml;llningar och spara</p>";
//const char HTTP_CONFIG_SCRIPT[] PROGMEM          = "<script>function setStartPos(){ var xhttp = new XMLHttpRequest();xhttp.onreadystatechange = function(){if(this.readyState == 4 && this.status == 200){document.getElementById('label_start').innerHTML = document.getElementById('startpos').value;}}xhttp.open(\"GET\", \"setPosition?position='document.getElementById('startpos').value'\", true); xhttp.send();}</script>";
const char HTTP_CONFIG_SCRIPT[] PROGMEM          = "<script>function setStartPos(){ var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange=function(){ if(this.readyState==4 && this.status ==200) {document.getElementById('label_start').innerHTML = document.getElementById('startpos').value;}}; xhttp.open(\"GET\", \"setPosition?position=\"+document.getElementById('startpos').value, true); xhttp.send();}function setEndPos(){ var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange=function(){ if(this.readyState==4 && this.status ==200) {document.getElementById('label_end').innerHTML = document.getElementById('endpos').value;}}; xhttp.open(\"GET\", \"setPosition?position=\"+document.getElementById('endpos').value, true); xhttp.send();}</script>";


const char HTTP_SERVO_CONFIG_FORM[] PROGMEM = "<form action=\"/config\">Start position<br/><label for\"startpos\" id=\"label_start\">{{startpos}}</label><input type=\"range\" onInput=\"setStartPos()\" id=\"startpos\" name=\"startpos\" value=\"{{startpos}}\" min=\"1\" max=\"180\"/><br/>Stop position<br/><label for\"endpos\" id=\"label_end\">{{endpos}}</label><input type=\"range\" onInput=\"setEndPos()\" id=\"endpos\" name=\"endpos\" value=\"{{endpos}}\" min=\"1\" max=\"180\"/><br/><input type=\"submit\" id=\"save_btn\" name=\"save_btn\" value=\"Spara\"/><input type=\"submit\" id=\"return_btn\" name=\"return_btn\" value=\"&Aring;terg&aring;\"></form>";
#endif
