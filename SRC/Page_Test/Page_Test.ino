
#include <ESP8266WiFi.h>
#include "FS.h"

#define OFF_FILE "/off.html"
#define ON_FILE "/on.html"

typedef enum{
  OFF = 0,
  ON,
  ERR,
}response_vals;

const char* ssid = "Tejas";
const char* password = "7041679923";
 
int ledPin = LED_BUILTIN; /*  On GPIO3*/
bool spiffsActive = false;
bool on_display = 0;
bool off_display = 0;
bool one_time_only = 1;
File off_f;
File on_f;
int value = HIGH; /*  Pin on which LED is there is working in inverted form*/
int response;
String on_content;
String off_content;

WiFiServer server(80);
 
void setup() {
  Serial.begin(115200);
  delay(10);

  /*  Start filing subsystem */
  if (SPIFFS.begin()) {
      Serial.println("SPIFFS Active");
      Serial.println();
      spiffsActive = true;
  } else {
      Serial.println("Unable to activate SPIFFS");
  }
 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
 
   /* Connect to WiFi network */
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  /*  Start the server */
  server.begin();
  Serial.println("Server started");
 
  /* Print the IP address */
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}
 
void loop() {

  /* Check if a client has connected*/
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  /* Wait until the client sends some data */
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  /* Read the first line of the request */
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  if (spiffsActive && one_time_only) {
    if (SPIFFS.exists(OFF_FILE) && SPIFFS.exists(ON_FILE) ) {
        Serial.println("Files exists");
    } else {
        Serial.println("Files do not exist");
        response = ERR;
    }
    one_time_only = 0;
  }
  /* Match the request */
  if (request.indexOf("/LED=OFF") != -1)  {
    digitalWrite(ledPin, HIGH);
    value = HIGH;
    response = OFF;
  }
  if (request.indexOf("/LED=ON") != -1)  {
    digitalWrite(ledPin, LOW);
    value = LOW;
    response = ON;
  }
  
  /*  Return the response */

  switch(response){
    case ON:

      on_f = SPIFFS.open(ON_FILE, "r");
      while (on_f.position()<on_f.size())
      {
        on_content = on_f.readStringUntil('\n');
        on_content.trim();
        client.println(on_content);
      } 
      on_f.close();
      break;

    case OFF:
    default:

      off_f = SPIFFS.open(OFF_FILE, "r");
      while (off_f.position()<off_f.size())
      {
        off_content = off_f.readStringUntil('\n');
        off_content.trim();
        client.println(off_content);
      } 
      off_f.close();
      break;

    case ERR:

      client.println("File not found ERR 404");
      break;
  }

  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}
