#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
String apiKey = "EESKYJFS48N3FCD2";     //  API KEY THINKSPEAK
const char* server = "api.thingspeak.com";
const char *ssid =  "ototronic";     // ssid wifi
const char *pass =  "mentaltempe112_"; // wifi password
#define DHTPIN D4          // GPIO Pin where the dht11 is connected
DHT dht(DHTPIN, DHT11);
WiFiClient client;

// Soil Moisture Pin
const int moisturePin = A0;
// Motor pin
const int motorPin = D0;
unsigned long interval = 10000;
unsigned long previousMillis = 0;
unsigned long interval1 = 1000;
unsigned long previousMillis1 = 0;
float moisturePercentage;              //moisture reading
float h;                  // humidity reading
float t;                  //temperature reading

// I2C 
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Membuat ikon kelelembaban // 
byte suhu[8] =
{
  B00100,
  B01010,
  B01010,
  B01110,
  B11111,
  B11111,
  B01110,
  B00000
};

// Membuat ikon kelelembaban // 
byte kelembaban[8] =
{
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110,
  B00000
};

void setup()
{
  Serial.begin(115200);
  //Start LCD Screen
  lcd.begin();
  lcd.backlight();
  lcd.createChar(1, kelembaban);
  lcd.createChar(2, suhu);
  lcd.setCursor(0,0);
  dht.begin();
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.write(2);
  lcd.print(" Suhu: ");
  lcd.setCursor(0,1);
  lcd.write(1);
  lcd.print(" Lembab: ");
  delay(10);
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW); // keep motor off initally
  dht.begin();
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");              // print ... till not connected
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop()
{
  unsigned long currentMillis = millis(); // grab current time

  h = dht.readHumidity();     // read humiduty
  t = dht.readTemperature();     // read temperature

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  moisturePercentage = ( 100.00 - ( (analogRead(moisturePin) / 1023.00) * 100.00 ) );

  if ((unsigned long)(currentMillis - previousMillis1) >= interval1) {
    Serial.print("Soil Moisture is  = ");
    Serial.print(moisturePercentage);
    Serial.println("%");
    previousMillis1 = millis();
  }

if (moisturePercentage < 50) {
  digitalWrite(motorPin, HIGH);         // tun on motor
}
if (moisturePercentage > 50 && moisturePercentage < 55) {
  digitalWrite(motorPin, HIGH);        //turn on motor pump
}
if (moisturePercentage > 56) {
  digitalWrite(motorPin, LOW);          // turn off mottor
}

  
  lcd.setCursor(8,0);
  lcd.print(t,1);
  lcd.print((char)223);
  lcd.print("C     ");
  lcd.setCursor(10,1);
  lcd.print(moisturePercentage, 0);
  lcd.print("%     ");

if ((unsigned long)(currentMillis - previousMillis) >= interval) {

  sendThingspeak();           //send data to thing speak
  previousMillis = millis();
  client.stop();
}

}

void sendThingspeak() {
  if (client.connect(server, 80))
  {
    String postStr = apiKey;              // add api key in the postStr string
    postStr += "&field1=";
    postStr += String(moisturePercentage);    // add mositure readin
    postStr += "&field2=";
    postStr += String(t);                 // add tempr readin
    postStr += "&field3=";
    postStr += String(h);                  // add humidity readin
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());           //send lenght of the string
    client.print("\n\n");
    client.print(postStr);                      // send complete string
    Serial.print("Moisture Percentage: ");
    Serial.print(moisturePercentage);
    Serial.print("%. Temperature: ");
    Serial.print(t);
    Serial.print(" C, Humidity: ");
    Serial.print(h);
    Serial.println("%. Sent to Thingspeak.");
  }
}