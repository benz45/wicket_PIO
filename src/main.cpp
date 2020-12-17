#include <ESP8266HTTPClient.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#define WIFI_SSID "xxx"
#define WIFI_PASSWORD "xxx"
#define FIREBASE_HOST "xxx"
#define FIREBASE_AUTH "xxx"

int numConnect = 0;
Servo myservo;
String productKey = "F80WI7";
String statusPath = "/door/datas/" + productKey + "/status";
String degreePath = "/door/status/" + productKey + "/degree";
String degreePathDatas = "/door/datas/" + productKey + "/degree";
String doorPath = "door/datas/" + productKey + "/arduinoConnection";
String connectionPath = "connections/datas/" + productKey + "/arduinoConnection";

void setup()
{
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  // Servo setup
  myservo.attach(D6);

  //Relay setup
  pinMode(D7, OUTPUT);

  Serial.println(WiFi.localIP());
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    Serial.print(WiFi.status());
    delay(500);
    int currentRotate = myservo.read();
    Serial.println("Door being off.");
    Serial.println(currentRotate);
    for (int i = currentRotate; i < 170; i++)
    {
      myservo.write(i);
      delay(15);
    }
    delay(300);
    digitalWrite(D7, HIGH);
  }

  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.status());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop()
{
  int currentRotate = myservo.read();
  bool lastStatus = Firebase.getBool(statusPath);

  if (Firebase.failed())
  {
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Serial.println(Firebase.error());
    Serial.println("Connection to firebase failed...");
  }
  else
  {
    Serial.println("Connection firebase successful.");

    // Check connection from app.
    if (Firebase.getBool("/connections/datas/" + productKey + "/appConnection") != true)
    {
      Serial.println("Disconnection wicket app.");
    }
    else
    {
      // Set number loop connection.
      Firebase.setInt("connections/states/" + productKey + "/state", numConnect);

      // Set arduinoConnection online.
      Firebase.setBool(doorPath, true);
      Firebase.setBool(connectionPath, true);

      // // Check status on door.
      if (lastStatus == true && currentRotate > 1)
      {
        digitalWrite(D7, LOW);
        delay(300);
        for (int i = currentRotate; i > 0; i--)
        {
          myservo.write(i);
          delay(15);
        }
        Firebase.setInt(degreePath, currentRotate);
        Firebase.setInt(degreePathDatas, currentRotate);
      }
      // Check status off door.
      if (lastStatus == false && currentRotate < 169)
      {
        for (int i = currentRotate; i < 170; i++)
        {
          myservo.write(i);
          delay(15);
        }
        delay(300);
        digitalWrite(D7, HIGH);
        Firebase.setInt(degreePath, currentRotate);
        Firebase.setInt(degreePathDatas, currentRotate);
      }
      numConnect++;
      delay(500);
    }
  }
}
