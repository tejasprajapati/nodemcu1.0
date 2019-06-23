
#include <ESP8266WiFi.h>
#include "FS.h"

#define OFF_FILE "/off.html"
#define ON_FILE "/on.html"
#define DEBUG 1
#define MAX_VALS  16

#define DRY_AIR_LOW 0
#define DRY_AIR_HIGH  15
#define HANDS_LOW 30
#define HANDS_HIGH  55

#define WATER_CALIBRATED  1
#define AIR_CALIBRATED  0

typedef enum{
	OFF = 0,
	ON,
	ERR,
}response_vals;

typedef enum{
	ONLY_WATER = 0,
	HANDS_ON_SENSOR,
}pump_vals;

const char* ssid = "Tejas";
const char* password = "7041679923";
 
int ledPin = LED_BUILTIN; /*  On GPIO3*/
bool spiffsActive = false, on_display = 0, off_display = 0,one_time_only = 1;
File off_f, on_f;
int value = HIGH; /*  Pin on which LED is there is working in inverted form*/
int response;
String on_content,off_content;
unsigned int base_value, measure_value;
int delta_value;
int sensor_read;
int soilMoisture;
int Sensorval;

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

	Sensorval = Read_adc();
	soilMoisture = map(Sensorval,1024,0,0,100);

	Serial.println("soilMoisture:");
	Serial.println(soilMoisture, DEC);
	delay(1000);
#if 1

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
  /* TODO: Put a value in condition for the range that can decide
   * when turn on or turn off
   */
	switch(response){
		case ON:
			on_f = SPIFFS.open(ON_FILE, "r");
			while (on_f.position()<on_f.size()) {
				on_content = on_f.readStringUntil('\n');
				on_content.trim();
				client.println(on_content);
				if(on_content.substring(0) == "<html>"){
					if(sensor_read == ONLY_WATER){
					client.println("Only water, pump would be turned off\n");
				} else if(sensor_read == HANDS_ON_SENSOR) {
					client.println("Pour some water\n");
				} else {
					client.println("Keep the pump off so no water wastage\n");
				}
			}
		}
			on_f.close();
		break;

		case OFF:
		default:
			off_f = SPIFFS.open(OFF_FILE, "r");
			while (off_f.position()<off_f.size()) {
				off_content = off_f.readStringUntil('\n');
				off_content.trim();
				client.println(off_content);
				if(off_content.substring(0) == "<html>"){
					if(sensor_read == ONLY_WATER){
						client.println("Only water, pump would be turned off\n");
					} else if(sensor_read == HANDS_ON_SENSOR) {
						client.println("Pour some water\n");
					} else {
						client.println("Keep the pump off so no water wastage\n");
					}
				}
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
#endif
}


int Read_adc()
{
#if DEBUG
	Serial.println(analogRead(A0));
#endif  
	return analogRead(A0);
}
