#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Servo.h>
#include <string>
#include <vector>

using namespace std;

ESP8266WiFiMulti WiFiMulti;
WiFiClient client;

#define MySSID "Fios-DGbZ5"
#define MyWifiPassword "silt769wry62opt"

String thisId = "6018896b3474e51208a11f4a";
String thatId = "601889a22a90dd1295c80127";

Servo main1;

const int pot = A0;
const int servo = D3;
const int servoToggle = D2;

const int redPin = D6;
const int greenPin = D5;
const int bluePin = D7;

int val;
int message[100];
int messageSize = 0;
void setup() {
  // put your setup code here, to run once:
  pinMode(pot, INPUT);
  pinMode(servoToggle, OUTPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  main1.attach(servo);
  digitalWrite(servoToggle, LOW); // enable servo
  Serial.begin(115200);
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);

  // Waiting for Wifi connect
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {

  // check api every 6 seconds
  for (int i = 0; i < 100; i++) {

    val = map(analogRead(pot), 238, 847, 0, 180);
    if (val < 0)
      val = 0;
    else if (val > 180)
      val = 180;

    //180 to the right, 0 to left, with wire going back
    // if dial is not on send, record values, then send when it returns
    if (val < 170 && val > 0) {
      ledOff();
      messageSize = 0;
      while (val < 170 && val > 0) {
        delay(1000); // time between blinks

        val = map(analogRead(pot), 238, 847, 0, 180);
        if (val < 0)
          val = 0;
        else if (val > 180)
          val = 180;

        if (val < 170 && val > 0) {
          message[messageSize] = val;
          blinkLED();
          Serial.println(val);
          messageSize++;
        } else if (val == 0) {
          goto breakall;
        }
      }
      fadeLED();
      postInstructions(message, messageSize);
    }
  breakall:
    Serial.println(val);
    fadeLED();
    delay(60);
  }

  // check for updates
  apiCheck();
}

int red = 255;
int green = 0;
int blue = 0;

int stage = 0;

void fadeLED() {
  switch (stage) {
  case 0:
    if (green < 255) {
      green++;
    } else {
      stage++;
    }
    break;
  case 1:
    if (red > 0) {
      red--;
    } else {
      stage++;
    }
    break;
  case 2:
    if (blue < 255) {
      blue++;
    } else {
      stage++;
    }
    break;
  case 3:
    if (green > 0) {
      green--;
    } else {
      stage++;
    }
    break;
  case 4:
    if (red < 255) {
      red++;
    } else {
      stage++;
    }
    break;
  case 5:
    if (blue > 0) {
      blue--;
    } else {
      stage = 0;
    }
    break;
  }
  /*
  Serial.print(red);
  Serial.print(",");
  Serial.print(green);
  Serial.print(",");
  Serial.println(blue);
  */
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

// blinks the leds
void blinkLED() {
  Serial.println("blink Led");
  red = 0;
  green = 255;
  blue = 255;
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
  delay(200);
  red = 255;
  green = 200;
  blue = 0;
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
  delay(200);
  red = 0;
  green = 255;
  blue = 255;
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
  delay(200);
  red = 255;
  green = 200;
  blue = 0;
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
  delay(200);

  ledOff();
}

// blinks yellow twice
void voicemailBlink() {
  Serial.println("blink Led");
  for (int i = 0; i < 2; i++) {
    red = 255;
    green = 200;
    blue = 0;
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
    delay(200);
    red = 0;
    green = 0;
    blue = 0;
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
    delay(200);
  }
}

void ledOff() {
  Serial.println("Led Off");
  red = 0;
  green = 0;
  blue = 0;
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

void apiCheck() {
  Serial.println("apiCheck");
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; // Object of class HTTPClient
    String url = "http://api.neil-sawhney.com/dials/" + thisId;
    http.begin(client, url);
    int httpCode = http.GET();
    // Check the returning code
    if (httpCode > 0) {
      // parsing
      const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(3) + 60;
      DynamicJsonBuffer jsonBuffer(capacity);

      JsonArray &root = jsonBuffer.parseArray(http.getString());

      JsonObject &root_0 = root[0];
      bool Status = root_0["status"];
      String Data = root_0["data"];
      if (Status) {
        for (int i = 0; i < 3; i++) {

          // parse
          Serial.println("performRequest");
          ledOff();
          int tempdata[100];
          int n = 0;
          String stuffBuff = "";

          for (int i = 1; i <= Data.length() - 2; i++) {
            if (Data[i] == ',') {
              tempdata[n] = stuffBuff.toInt();
              stuffBuff = "";
              n++;
            } else {
              stuffBuff += Data[i];
            }
          }
          tempdata[n] = stuffBuff.toInt();

          vector<int> data;
          auto iter = data.begin();
          for(int i=0; i<=n; i++){
             *iter = tempdata[i];
             advance(iter,1);
          }

          // end of parse   
          //performRequest(data);
          addToVoicemail(data);
          delay(5000);

          val = map(analogRead(pot), 238, 847, 0, 180);
          if (val < 0)
            val = 0;
          else if (val > 180)
            val = 180;

          if (val == 0) {
            break;
          }
        }
      }
    }
    http.end(); // Close connection
  }
}

struct voicemailEntity {
};

void addToVoicemail(vector<int> data){

}

void performRequest(vector<int> data) {

  digitalWrite(servoToggle, HIGH); // enable Servo

  for (int i = 0; i < data.size(); ++i) {
    main1.write(data[i]);
    Serial.println(data[i]);
    delay(500);
    blinkLED();
    delay(1000);
  }
  //180 degrees is the right! If the wire is to the back.
  main1.write(180);
  delay(1500);
  digitalWrite(servoToggle, LOW); // disable Servo

  postCompletion();
}

void postCompletion() {
  Serial.println("postCompletion");
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; // Object of class HTTPClient
    http.begin(client, "http://api.neil-sawhney.com/dials");
    http.addHeader("Content-Type", "application/json");
    int httpCode =
        http.POST("{\"_id\":\"" + thisId + "\",\"status\":\"false\"}");
    http.end(); // Close connection
  }
}

String postInstructions(int arr[], int arraySize) {
  // array to string
  Serial.println("arrayToString");
  String stuffBuff = "[";
  for (int i = 0; i <= arraySize - 1; i++) {
    stuffBuff += arr[i];
    if (i != arraySize - 1) {
      stuffBuff += ",";
    }
  }
  stuffBuff += "]";
  // array to string

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; // Object of class HTTPClient
    http.begin(client, "http://api.neil-sawhney.com/dials");
    http.addHeader("Content-Type", "application/json");
    int httpCode =
        http.POST("{\"_id\":\"" + thatId +
                  "\",\"status\":\"true\",\"data\":" + stuffBuff + "}");
    http.end(); // Close connection
  }
}
