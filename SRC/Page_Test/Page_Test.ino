
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

#define INTERVAL  60000

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
bool spiffsActive = false, client_requested = 0, off_display = 0,one_time_only = 1;
File off_f, on_f;
int value = HIGH; /*  Pin on which LED is there is working in inverted form*/
int response, loop_count;
String on_content,off_content;
unsigned int base_value, measure_value;
int delta_value;
int sensor_read;
int soilMoisture;
int Sensorval;

WiFiServer server(80);

void ICACHE_RAM_ATTR onTimerISR(){
  loop_count++;
}

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

  timer1_attachInterrupt(onTimerISR);
}
 
void loop() {

	Sensorval = Read_adc();
	soilMoisture = map(Sensorval,1024,0,0,100);

	Serial.println("soilMoisture:");
	Serial.println(soilMoisture, DEC);
	delay(1000);

 if (soilMoisture > 50 && !client_requested) {
    digitalWrite(ledPin, HIGH);
    value = HIGH;
    response = OFF;
 } else if (soilMoisture <= 50 && !client_requested) {
    digitalWrite(ledPin, LOW);
    value = LOW;
    response = ON;
 }

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

  Serial.println("loop_count");
  Serial.println(loop_count, DEC);
  Serial.println("client requested is");
  Serial.println(client_requested, DEC);
	/* Match the request */
	if (request.indexOf("/LED=OFF") != -1) {
		digitalWrite(ledPin, HIGH);
		value = HIGH;
		response = OFF;
    timer1_disable();
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
    timer1_write(3125); //10ms
    client_requested = 1;
	}
	if (request.indexOf("/LED=ON") != -1) {
		digitalWrite(ledPin, LOW);
		value = LOW;
		response = ON;
    timer1_disable();
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
    timer1_write(3125); //10ms
    client_requested = 1;
	}

  if (loop_count >= INTERVAL) {
    client_requested = 0;
    timer1_disable();
    loop_count = 0;
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
        client.println("<meta http-equiv=\"refresh\" content=\"1800\" >");
				if(on_content.substring(0) == "<html>"){
					if(soilMoisture <= 50){
            client.println("Plants need some water\n");
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
        client.println("<meta http-equiv=\"refresh\" content=\"1800\" >");
				if(off_content.substring(0) == "<html>"){
					if(soilMoisture > 50){
						client.println("Turn off the tap\n");
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
