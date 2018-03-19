#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include "DHT.h"
#include "UbidotsMicroESP8266.h"

#define TOKEN  "A1E-KmHyh9twToqbuFnyKTowfdA6PYkExZ"  // Put here your Ubidots TOKEN

// Set these to run example.
#define FIREBASE_HOST "smart-irrigation-system1.firebaseio.com"  //should not have http:// and the end /
#define FIREBASE_AUTH "KhO0JLTJsrUMO6b2pDkInDoYyMasnLpXTBFM0pnf"  // go to settings in firebase,select project settings and service account, secrets auth key
#define WIFI_SSID "JioFi3_5D0BBB"
#define WIFI_PASSWORD "fk379vfu3e"

//ubidots ids
#define ID_IRRIGATION_TIME "5aaacf29c03f977241092678"
#define ID_HUMIDITY "5aaacdabc03f977177c89aae"
#define ID_TEMPERATURE "5aaacd9ec03f97715a35cf85"
#define ID_SOIL_MOISTURE "5aaacd91c03f9770593eafb8"
#define ID_MIN_MOISTURE "5aaac546c03f9766451d919b"
#define ID_CROP_TYPE "5aaac2e6c03f9762f07f05e3"
#define ID_IRRIGATION_TYPE "5aaac2aac03f9762f1ab5884"
#define ID_SOIL_TYPE "5aaac268c03f9761eef95327"

  
  
// Defining the pins
#define SOIL_PIN A0
#define DHTPIN D2
#define RELAY_PIN D1
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
Ubidots client(TOKEN);

int soil_sensor;
int min_value = -50;
int rain_pred = 0;
int soil_type = 1, irrigation_type = 1, crop_type = 1;
int irrigation_time;
void setup() {  
//connecting serial monitor  
  Serial.begin(9600);

//Connecting wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  client.wifiConnection(WIFI_SSID, WIFI_PASSWORD);
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
//Initialization of firebase  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.set("Soil Sensor",1);
  Firebase.set("DHT sensor",1);
  Firebase.set("Irrigation Time",1);
  Firebase.set("Crop Type",1);
  Firebase.set("Irrigation Type",1);
  Firebase.set("Soil Type",1);
  
  
//initialization of pin
//  pinMode(SOIL_PIN,INPUT);
  //pinMode(DHT_PIN,INPUT);
  pinMode(RELAY_PIN,OUTPUT);
  digitalWrite(RELAY_PIN,HIGH);
 //initialization of DHT sensor
  dht.begin();
}

void loop() {
  //reading value of soil moisture
  soil_sensor= analogRead(SOIL_PIN);
  soil_sensor = map(soil_sensor,550,0,0,100);
  delay(3000);
  //reading value of DHT sensor
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    Serial.print(h);
    Serial.print(t);
    Serial.print(f);
    return;
  }

  if(h > 50.0){
    rain_pred = 1;
    Serial.print("Possibility of rain");
  }
  else
    rain_pred = 0;
  min_value = client.getValue(ID_MIN_MOISTURE);
  crop_type = client.getValue(ID_CROP_TYPE);
  irrigation_type = client.getValue(ID_IRRIGATION_TYPE);
  soil_type = client.getValue(ID_SOIL_TYPE);
    
  if(soil_sensor < min_value && rain_pred == 0){
    irrigation_time = (soil_type*1000 + irrigation_type*100 + crop_type*10)+10000;
    if(irrigation_time > 20000){
      irrigation_time = 20000;
    }
    Serial.print("Irrigation Time is : ");
    Serial.println(irrigation_time);
    digitalWrite(RELAY_PIN,LOW);
    delay(irrigation_time);
    digitalWrite(RELAY_PIN,HIGH);
   
  } 
  
  

//Uploading values on ubidots
  client.add("IRRIGATION TIME",irrigation_time);
  client.add("HUMIDITY",h);
  client.add("TEMPERATURE",t);
  client.add("SOIL MOISTURE",soil_sensor);
  client.sendAll(true);

//  delay(1000);
//Printing values
  Serial.print("Moisture : ");
  Serial.println(soil_sensor);
  Serial.print("Temperature : ");
  Serial.println(t);
  Serial.print("Humidity : ");
  Serial.println(h);
  Serial.print("Min Value : ");
  Serial.println(min_value);
  Serial.print("Soil Type : ");
  Serial.println(soil_type);
  digitalWrite(RELAY_PIN,HIGH);
}

