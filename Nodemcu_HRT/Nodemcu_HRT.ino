#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

SoftwareSerial nodemcu(D6, D5);

const char* ssid = "KRISHNAN";
const char* password = "KRISHNAN";
const char* readAPIKey = "WI3RYASM3X93T6D1";
const char* apiKey = "SJRJU1XPO5Y1J5WY";
const char* channelID = "2314952";
const char* serverURL = "http://api.thingspeak.com/update";

bool dataSent = false;         // Flag to track whether data has been sent
StaticJsonDocument<1000> doc;  // Specify desired capacity
const char* URL = "http://api.thingspeak.com/channels/";
const char* host = "maker.ifttt.com";
int lastEntryID = 0;
int resultsPerPage = 80;  // Number of results per page
int page = 1;             // Starting page number
int a = 5;
int lastentryFinder = 0;
int normal_min = 60;
int normal_max = 100;
int last7Field1Values[7];
int minValue = 0;
int maxValue = 0;
int variability = 0;
int next_max = 0;
int next_min = 0;
int field1Values[7];  // Array to store last 3 field1Value
int field1Index = 0;
int curr = 0;
int alertcount = 0;
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
int counter = 8;
int first = 0;
int second = 0;
int lcdvalue = 0;
int indication = 0;
int g_100 = 0;
int avgPerWeek;
int sum = 0;

void mailalert() {
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/trigger/BPM/with/key/b-LZutbBfyxC0ZGYAaeyb3";
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  delay(1000);
}

void introtext() {
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.print("   Heart Rate   ");
  lcd.setCursor(0, 1);
  lcd.print(" Monitor System ");
  delay(3000);
}
void instructions() {
  lcd.init();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Be Relax, Steady");
  lcd.setCursor(0, 1);
  lcd.print("  Place Finger  ");
  delay(3000);
  first = 3;
}
void analyzingRecords() {
  lcd.init();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Analyzing ...");
  lcd.setCursor(0, 1);
  lcd.print("Past Records BPM");
  delay(5000);
}

void smsalert() {
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/trigger/SMS/with/key/b-LZutbBfyxC0ZGYAaeyb3";
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  nodemcu.begin(9600);
  lcd.init();
  lcd.backlight();
  introtext();
  // if (first < 3) {
  //   lcd.init();
  //   lcd.clear();
  //   lcd.setCursor(0, 0);
  //   lcd.print("Be Relax, Steady");
  //   // lcd.print("Stay calm,Stable");
  //   lcd.setCursor(0, 1);
  //   lcd.print("  Place Finger  ");
  //   // lcd.print("Place ur finger");
  //   delay(3000);
  //   first = 3;
  // }
  instructions();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting....");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    if (first == 3) {
      lcd.init();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wait for");
      // Serial.println("Printed");
      lcd.setCursor(0, 1);
      lcd.print(counter);
      counter = counter - 1;
      if (counter == 0) {
        first = 25;
      }
      delay(1000);
    }
    HTTPClient http;
    WiFiClient client;
    if (a == 5) {
      for (int i = 1; i <= 10; i++) {
        DeserializationError error = deserializeJson(doc, nodemcu);

        if (!error) {
          int bpm = doc["bpm"];
          lcdvalue = bpm;
          if (bpm > 100) {
            g_100 = 1;
          }
          String data = "field1=" + String(bpm);

          if (bpm > 0 && !dataSent && bpm < 101) {
            http.begin(client, serverURL);
            http.addHeader("X-THINGSPEAKAPIKEY", apiKey);
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");

            int httpCode = http.POST(data);
            if (httpCode == HTTP_CODE_OK) {
              Serial.println("\nData sent to ThingSpeak");
              dataSent = true;
              a = 2;
              // lcdvalue = bpm;
            } else {
              Serial.printf("[HTTP] POST failed, error: %d\n", httpCode);
            }

            http.end();
          }
          if (g_100 == 0) {
            Serial.print("BPM From Arduino UNO:\t");
            Serial.println(bpm);
            break;
            delay(15000);
          }
        }
      }
    }
    if (a == 2) {

      String url = String(URL) + channelID + "/feeds.json?api_key=" + readAPIKey + "&results=50" + "&page=" + String(page);

      HTTPClient http;
      WiFiClient client;

      http.begin(client, url);
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        DynamicJsonDocument doc(9216);
        DeserializationError error = deserializeJson(doc, payload);

        JsonArray feeds = doc["feeds"];
        int feedCount = feeds.size();

        // Serial.println(feedCount);
        if (feedCount > 0) {
          JsonVariant lastFeed = feeds[feedCount - 2];
          lastEntryID = lastFeed["entry_id"];

          JsonVariant lastBeforeFeed = feeds[feedCount - 1];
          curr = lastBeforeFeed["field1"];

          Serial.print("last before entryid: ");
          Serial.println(lastEntryID);
          Serial.print("current heart Rate: ");
          Serial.println(curr);
        }

        if (lastEntryID > 6) {
          int fcount = feeds.size();
          int maxcount = 2;
          int ind = 0;
          float value = 0;
          while (maxcount < 9) {
            JsonVariant val = feeds[fcount - maxcount];
            value = val["field1"];
            last7Field1Values[ind] = value;
            ind++;
            maxcount++;
          }
          for (int ji = 0; ji < 7; ji++) {
            sum = sum + last7Field1Values[ji];
            Serial.println(last7Field1Values[ji]);
          }
        }
        avgPerWeek = sum / 7;
        maxValue = last7Field1Values[0];
        minValue = last7Field1Values[0];

        for (int ij = 1; ij < 7; ij++) {
          if (last7Field1Values[ij] > maxValue) {
            maxValue = last7Field1Values[ij];
          }
          if (last7Field1Values[ij] < minValue) {
            minValue = last7Field1Values[ij];
          }
        }
        Serial.print("Average BPM per week: ");
        Serial.println(avgPerWeek);
        Serial.print("Max Value: ");
        Serial.println(maxValue);
        Serial.print("Min Value: ");
        Serial.println(minValue);
        variability = maxValue - minValue;
        next_max = maxValue + variability;
        next_min = minValue - variability;

        if (curr >= normal_min && curr > next_min && curr <= normal_max && curr < next_max) {
          Serial.println("normal");
        } else if ((curr <= normal_min && curr <= next_min) || (curr >= normal_max && curr >= next_max)) {
          Serial.println("Alert");
          alertcount += 1;
        } else if (curr > next_min && curr <= normal_min) {
          Serial.println("Alert...Your heart rate is below the normal human heart rate level");
          alertcount += 1;
        } else if (curr >= normal_min && curr <= next_min) {
          Serial.println("Abnormal Readings...there is a big change in your heart rate, it is better to consult a doctor");
          alertcount += 1;
        } else if (curr <= next_min && curr <= normal_min) {
          Serial.println("Alert...you should consult a doctor");
          alertcount += 1;
        } else if (curr <= normal_max && curr >= next_max) {
          Serial.println("Abnormal Readings....there is a big change in your heart rate, it is better to consult a doctor");
          alertcount += 1;
        } else if (curr >= normal_max && curr < next_max) {
          Serial.println("Alert...Your heart rate is above the normal human heart rate level");
          alertcount += 1;
        } else {
          Serial.println("Alert...consult doctor");
          alertcount += 1;
        }

        a = 3;
        http.end();
        page++;
      }
    }
  }

  if (alertcount > 0) {
    mailalert();
    smsalert();
    indication = 1;
    alertcount = 0;
  }

  if (first == 25 && g_100 == 0) {
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Current BPM: ");
    lcd.setCursor(0, 1);
    lcd.print(lcdvalue);
    delay(3000);
    if (!avgPerWeek) {
      analyzingRecords();
    }
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Avg HR(per Week)");
    lcd.setCursor(0, 1);
    lcd.print(avgPerWeek);
    delay(3000);
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Min: ");
    lcd.print(minValue);
    lcd.print(" Max: ");
    lcd.print(maxValue);
    lcd.setCursor(0, 1);
    lcd.print("Variability:");
    lcd.print(variability);
    delay(3000);
    if (indication == 1) {
      lcd.init();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Status: ");
      lcd.print("Alert");
      lcd.setCursor(0, 1);
      lcd.print("Consult Doctor.");
    } else {
      lcd.init();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Status: ");
      lcd.print("Normal");
      lcd.setCursor(0, 1);
      lcd.print("Good, Healthy.");
    }
    if (g_100 == 1) {
      lcd.init();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BPM Exceeds 100");
      lcd.setCursor(0, 1);
      lcd.print("Try again...");
    }
    first = 29;
  }


  // if (first == 25 && g_100 == 0) {

  //   if (indication == 1) {
  //     lcd.init();
  //     lcd.clear();
  //     lcd.setCursor(0, 0);
  //     lcd.print("HeartRate: ");
  //     lcd.print(lcdvalue);
  //     lcd.setCursor(0, 1);
  //     lcd.print("Alert");

  //   } else {
  //     lcd.init();
  //     lcd.clear();
  //     lcd.setCursor(0, 0);
  //     lcd.print("HeartRate: ");
  //     lcd.print(lcdvalue);
  //     lcd.setCursor(0, 1);
  //     lcd.print("Normal");
  //   }
  // }
  // if (g_100 == 1) {
  //   lcd.init();
  //   lcd.clear();
  //   lcd.setCursor(0, 0);
  //   lcd.print("BPM Exceeds 100");
  //   lcd.setCursor(0, 1);
  //   lcd.print("Try again...");
  // }
}
