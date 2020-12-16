#include <ESP8266HTTPClient.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#define WIFI_SSID "MI8"
#define WIFI_PASSWORD "88888888"
#define FIREBASE_HOST "rn-wicket.firebaseio.com"
#define FIREBASE_AUTH "Bfe2xLJetFuxjMvuuXTzemAZJbbHvulqbXZlnVO1"

Servo myservo;
int pot = 180;
int val;
int numConnect = 0;
int degree = 0;
String productKey = "F80WI7";
String statusPath = "/door/datas/" + productKey + "/status";
String degreePath = "/door/status/" + productKey + "/degree";
String degreePathDatas = "/door/datas/" + productKey + "/degree";
String doorPath = "door/datas/" + productKey + "/arduinoConnection";
String connectionPath = "connections/datas/" + productKey + "/arduinoConnection";

void serRotate(byte rotateValue)
{
  byte currentRotate = myservo.read();
  if (rotateValue > currentRotate)
  {
    // Door being off.(false)
    Serial.println("Door being off.");
    for (int i = currentRotate; i < rotateValue; i++)
    {
      myservo.write(i);
      delay(15);
    }
    delay(300);
    digitalWrite(D7, HIGH);
  }
  else
  {
    // Door being on.(true)
    digitalWrite(D7, LOW);
    delay(300);
    Serial.println("Door being on.");
    for (int i = currentRotate; i > rotateValue; i--)
    {
      myservo.write(i);
      delay(15);
    }
  }

  Firebase.setInt(degreePath, currentRotate);
  delay(500);
  Firebase.setInt(degreePathDatas, currentRotate);
  delay(500);
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // Servo setup
  myservo.attach(D6);
  // myservo.write(180);

  //Relay setup
  pinMode(D7, OUTPUT);
  // digitalWrite(D7, LOW);

  Serial.println(WiFi.localIP());
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    Serial.print(WiFi.status());
    delay(500);
  }

  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.status());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop()
{
  if (Firebase.failed())
  {
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Serial.println(Firebase.error());
    Serial.println("Connection to firebase failed...");
    delay(1000);
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
      delay(500);

      // Set arduinoConnection online.
      Firebase.setBool(doorPath, true);
      delay(500);
      Firebase.setBool(connectionPath, true);
      delay(500);

      // Check status on door.
      if (Firebase.getBool(statusPath) == true)
      {
        serRotate(0);
      }
      // Check status off door.
      if (Firebase.getBool(statusPath) == false)
      {
        serRotate(170);
      }
      numConnect++;
      delay(1000);
    }
  }
}
