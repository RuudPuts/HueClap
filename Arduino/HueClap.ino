#include <Ethernet.h>
#include <SPI.h>

int threshold = 0;
int thresholdUpdateDelay = 0;

long maxClapTime = 500;
long sequenceTimeout = 200;
boolean processingClap = false;
long clapStartTime;
long clapEndTime;
long lastClapDuration;
long numberOfClaps;
long pauseBetweenClaps;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 1, 101};
byte server[] = { 192, 168, 1, 100 };

EthernetClient client;

void setup() {
  Ethernet.begin(mac, ip);  
  Serial.begin(9600);
  
  setAllBrightness(0, false);
  Serial.println("Setup done");
}

void loop()
{
  threshold = analogRead(A4);
  int sensorValue = analogRead(A5);
  processSensorValue(sensorValue);
  
  long now = millis();
  if (clapEndTime > 0 && numberOfClaps > 0 && pauseBetweenClaps > 0 && (clapEndTime + pauseBetweenClaps + sequenceTimeout) < now) {
    // Sequence over, send number of claps
    setSensorStatus(numberOfClaps);
    numberOfClaps = 0;
    pauseBetweenClaps = 0;
    clapStartTime = 0;
    clapEndTime = 0;
  }
  
  if (clapEndTime > 0 && pauseBetweenClaps == 0 && (clapEndTime + maxClapTime) < now) {
    Serial.println("Resetting");
    setSensorStatus(numberOfClaps);
    numberOfClaps = 0;
    pauseBetweenClaps = 0;
    clapStartTime = 0;
    clapEndTime = 0;
  }
  
  if (thresholdUpdateDelay == 0) {
    sendThreshold(threshold);
    setAllBrightness(threshold / 4, true);
    thresholdUpdateDelay = 100;
  }
  else {
    thresholdUpdateDelay--;
  }
  
  delay(10);        // delay in between reads for stability
}

void processSensorValue(int sensorValue) {
  if (sensorValue > threshold && !processingClap) {
    processingClap = true;
    clapStartTime = millis();
    if (clapEndTime > 0) {
      pauseBetweenClaps = clapStartTime - clapEndTime;
      Serial.println("Pause: " + String(pauseBetweenClaps));
    }
    Serial.println("Clap started");
  }
  else if (sensorValue < threshold && processingClap) {
    clapEndTime = millis();
    lastClapDuration = clapEndTime - clapStartTime;
    Serial.println("Clap ended - Duration: " + String(lastClapDuration));
    processingClap = false;
    numberOfClaps++;
    Serial.println("Number of claps: " + String(numberOfClaps));
  }
}

//void loop()
//{
//  int sensorValue = analogRead(A0);
//  int brightness = sensorValue / 4;
//  if (brightness > 255) {
//    brightness = 255;
//  }  
//  
//  if (brightness != previousBrightness) {
//    Serial.println("Brightness: " + String(brightness));
//    analogWrite(led, brightness);
//    setAllBrightness(brightness);
//    previousBrightness = brightness;
//  }
//  
//  delay(500);        // delay in between reads for stability
//}

void setAllBrightness(int brightness, boolean onlyBrightness) {
  Serial.println("Sending brightness: " + String(brightness));
  String body = "";
  if (onlyBrightness) {
    body = String("{\"bri\":" + String(brightness) + "}");
  }
  else {
    String onState = brightness == 0 ? "false" : "true";
    body = String("{\"on\": " + onState + ",\"bri\":" + String(brightness) + "}");
  }
  String request = String("PUT /api/aValidUser/groups/0/action HTTP/1.0");
  
  if (client.connect(server, 80)) {
    client.println(request);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println();
    client.println(body);
    client.println();
    client.stop();
    Serial.println("Done");
  } else {
    Serial.println("setBrightness connection failed");
  }
}

void setSensorStatus(int status) {
  Serial.println("Sending sensor status: " + String(status));
  String body = String("{\"state\":{\"status\":" + String(status) + "}}");
  Serial.println(body);
  String request = String("PUT /api/aValidUser/sensors/2 HTTP/1.0");
  
  if (client.connect(server, 80)) {
    client.println(request);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println();
    client.println(body);
    client.println();
    client.stop();
    Serial.println("Done");
  } else {
    Serial.println("setSensorStatus connection failed");
  }
}

void sendThreshold(int threshold) {
  Serial.println("Sending threshold status: " + String(threshold));
  String body = String("{\"state\":{\"status\":" + String(threshold) + "}}");
  Serial.println(body);
  String request = String("PUT /api/aValidUser/sensors/4 HTTP/1.0");
  
  if (client.connect(server, 80)) {
    client.println(request);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println();
    client.println(body);
    client.println();
    client.stop();
    Serial.println("Done");
  } else {
    Serial.println("sendThreshold connection failed");
  }
}