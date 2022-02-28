#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

  
// Lütfen wifi şifrenizi ve ssid adınızı doldurunuz
const char* ssid     = "******";
const char* password = "******";

// Web sunucusunun portu 80 olarak ayarlandı
WiFiServer server(80);

// HTTP Request başlığı
String header;

// Output durumları
String output5State = "Off";
String output4State = "off";

// Output GPIO çıkış portları 
const int output5 = 5;
const int output4 = 4;

// Şuanki zaman
unsigned long currentTime = millis();

unsigned long previousTime = 0; 

const long timeoutTime = 2000;

// İlk kurulum işlemini başlatıyoruz
void setup() {
  Serial.begin(115200);
  
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // LCD Ekrana ip yazdırma
 
  Wire.begin(2, 0);
  lcd.begin();
  lcd.home();
  lcd.setCursor(0,0);
  lcd.print("IP:" + WiFi.localIP().toString());
  lcd.setCursor(0,1);
  lcd.print("Durum: Off");
  server.begin();
}

// Loop döngüsü başlangıcı
void loop(){
  
  Wire.begin(2, 0);
  lcd.begin();
  lcd.home();
  lcd.setCursor(0,1);
              lcd.print("Durum: " + output5State);
  lcd.setCursor(0,0);
  lcd.print("IP:" + WiFi.localIP().toString());
  WiFiClient client = server.available();   

  if (client) {                             
    Serial.println("New Client.");          
    String currentLine = "";                
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { 
      currentTime = millis();         
      if (client.available()) {            
        char c = client.read();             
        Serial.write(c);                    
        header += c;
        if (c == '\n') {                    
       
          if (currentLine.length() == 0) {
           
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Bu kısım birazcık hazır :)
          
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              lcd.setCursor(0,1);
              lcd.print("Durum: " + output5State);
              output5State = "On";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              lcd.setCursor(0,1);
              lcd.print("Durum: " + output5State);
              output5State = "Off";
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }
            
            // HTML Sayfa kodları
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<title>UTKU'nun Web Sunucusu</title>");
           // CSS Kodları || Geliştirilebilir
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #226a19; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #6a1d19;} .menucolor {     background-color: #345a6c;     float: left;     z-index: -1;     height: 68px;     width: 100%;     position: absolute;     opacity: .3;     filter: alpha(opacity=30); } .menu {     float: left;     width: 100%;     margin: 0;     }</style></head>");
            
            
            client.println("<body><h1>UTKU'nun Web Sunucusu</h1>");
            
            
            client.println("<p>Rolenin durumu - " + output5State + "</p>");
            
            if (output5State=="Off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
         
          
            client.println("</body></html>");
            
            
            client.println();
           
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;     
        }
      }
    }
    
    header = "";
    
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
