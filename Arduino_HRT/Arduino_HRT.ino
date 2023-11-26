#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

const int PulseWire = 2;
int Threshold = 550;
SoftwareSerial nodemcu(5,6);
int sum=0;
int maxx=0;
int ans=0;
PulseSensorPlayground pulseSensor;

void setup() { 
Serial.begin(9600);
nodemcu.begin(9600);
pulseSensor.analogInput(PulseWire);
//  pulseSensor.blinkOnPulse(LED);
pulseSensor.setThreshold(Threshold);
if (pulseSensor.begin()) {
Serial.println("We created a pulseSensor Object !"); 
Serial.println("Place your hands gently for 30secs");
delay(17000);
}
}

void loop() {
StaticJsonDocument<1000> doc;
int myBPM = pulseSensor.getBeatsPerMinute();
if(pulseSensor.sawNewSample()){
 if(maxx!=6){
if (pulseSensor.sawStartOfBeat()) {
Serial.println("♥ A HeartBeat Happened ! ");
Serial.print("BPM: "); 
Serial.println(myBPM);
 sum=sum+myBPM;
 maxx++;
if(maxx==6){
 Serial.println("Sum: ");
 Serial.println(sum);
 ans=sum/6;
 Serial.print("Average heart Rate: ");
 Serial.println(ans);
}
}}
}
else{
  Serial.println("Place hand on sensor");
}
doc["bpm"]=ans;
serializeJson(doc, nodemcu);
delay(1000);
}
