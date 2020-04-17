#include <Scheduler.h>
#include <TridentTD_LineNotify.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

//DHT11
#define DHTPIN D6
#define DHTTYPE DHT11
#define SW D4
#define Fan_1 13
#define Fan_2 14


//Connect WiFi
//SSID
#define SSID "Tingsdt"
//Password
#define PASSWORD "123456789"

//LINE TOKEN
#define LINE_TOKEN "EoEsFlWHogGKL0gXo86NTSGHiOpUywVaDZx8H4SoRNH"

//Thingspeak API key, 
String apiKey = "S6SOCGTMTJQ0PM40";
const char* ssid = "Tingsdt";
const char* password = "123456789"; 
const char* server = "api.thingspeak.com";

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient client;

int analogPin = A0;
int analog_val = 0;
/*int Fan_1 = 13;
int Fan_2 = 14;*/
//For DHT11
class FirstTask : public Task {
protected:
    void setup() {
      Serial.begin(9600);
      Serial.println("DHT test");
      dht.begin();
      pinMode(Fan_1, OUTPUT);
      digitalWrite(Fan_1, LOW);
      pinMode(Fan_2, OUTPUT);
      digitalWrite(Fan_2, LOW);
      

    }

    void loop() {
    
        String dht11_val = "";
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        
    if (isnan(t) || isnan(h)) 
    {
    Serial.println("Failed to read from DHT");
    } 
    else 
    {
   
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println("*C");
   }
  


        if (isnan(h) || isnan(t)) {
            Serial.println("Failed to read from DHT sensor!");
        }

        if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com
            String postStr = apiKey;
            postStr +="&field1=";
            postStr += String(t);
            postStr +="&field2=";
            postStr += String(h);
            postStr += "\r\n\r\n";

            client.print("POST /update HTTP/1.1\n"); 
            client.print("Host: api.thingspeak.com\n"); 
            client.print("Connection: close\n"); 
            client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n"); 
            client.print("Content-Type: application/x-www-form-urlencoded\n"); 
            client.print("Content-Length: "); 
            client.print(postStr.length()); 
            client.print("\n\n"); 
            client.print(postStr);

            Serial.print("Temperature: ");
            Serial.print(t);
            Serial.print(" degrees Celcius Humidity: "); 
            Serial.print(h);
            Serial.println("% send to Thingspeak");    
        }
        
        client.stop();

        if (t > 30) 
          {
            digitalWrite(Fan_1, HIGH);
          }
         else
         {
            digitalWrite(Fan_1, LOW);
          }

        if (t < 27)
          {
             digitalWrite(Fan_2, HIGH);
          }
           else
           {
            digitalWrite(Fan_2, LOW);
          
          }
            
        dht11_val = dht11_val + "อุณหภูมิ "+t+"°C"+ " ความชื้น "+h+"%";
        Serial.println(dht11_val);
        LINE.notify(dht11_val);

        delay(2000);
    }

private:
    uint8_t state;
} first_task;

//For RainDrop
class SecondTask : public Task {
protected:
    void setup() {
       pinMode(SW,INPUT);
       Serial.begin(9600); 
    }

    void loop() {
        analog_val = analogRead(analogPin);
        Serial.print("analog val = ");
        Serial.println(analog_val); 

        if (digitalRead(SW) == HIGH){
          while(digitalRead(SW) == HIGH) delay(10);
          LINE.notify("ตอนนี้มีน้ำหกหรืองูปัสสาวะนะจ๊ะ");
        }
        delay(10);
  }
    
private:
    uint8_t state;
} second_task;

//For LCD

class ThirdTask : public Task {
protected:
    void setup() {
        
    }

    void loop() {
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (isnan(h) || isnan(t)) {
            Serial.println(F("Failed to read from DHT sensor!"));
            return;
        }
        lcd.setCursor(0, 0);
        lcd.print("Temp:     ");
        lcd.setCursor(4, 0);
        lcd.print(t);
        lcd.setCursor(9, 0);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Hum:     ");
        lcd.setCursor(4, 1);
        lcd.print(h);
        lcd.setCursor(9, 1);
        lcd.print("%");
        delay(2000);
    }
    
private:
    uint8_t state;
} third_task;

void setup() {
    Serial.begin(9600);

    dht.begin();
    lcd.begin();

    Serial.println();
    Serial.println(LINE.getVersion());
    WiFi.begin(SSID, PASSWORD);
    Serial.printf("WiFi connecting to %s\n", SSID);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(2000);
    }

    Serial.printf("\nWiFi connected\nIP : ");
    Serial.println(WiFi.localIP());
    LINE.setToken(LINE_TOKEN);
    LINE.notify("เซนเซอร์วัดอุณหภูมิ ความชื้นและเซนเซอร์ตรวจจับน้ำหกหรืองูปัสสาวะ เริ่มทำงานแล้ว");

    Scheduler.start(&first_task);
    Scheduler.start(&second_task);
    Scheduler.start(&third_task);
    Scheduler.begin();
}

void loop() {

}