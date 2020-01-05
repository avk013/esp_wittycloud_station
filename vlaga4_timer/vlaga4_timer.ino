//witty cloud
#include <ESP8266WiFi.h> //web_client
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <TimeLib.h>
//date-time
#include <Ticker.h>
//timer lib
/*power GPIO2 инверсированный индикатор зарузки
  LDR = A0;//датчик освещенности
  GPIO15 RED = 15;//красный вывод RGB светодиода
  GPIO12 GREEN = 12;//зеленый вывод RGB светодиода
  GPIO13 BLUE = 13;//синий вывод RGB светодиода
  GPIO4 BUTTON = 4;//кнопка инвертирована
*/
int red = 15, blue = 13, green = 12; // blue-WEB red-WIFI errors
// Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
/*Put your SSID & Password*/
const char* ssid = "prostor";  // Enter SSID here
const char* password = "";  //Enter Password here
const String macadr = WiFi.macAddress(); //узнаем свом МАК
char adr_server[] = "3it.pp.ua"; // отправляем на
//char adr_server[] = "192.168.2.1"; // debug
int buff = 0;
TimeLib times(2);// часовой пояс
ESP8266WebServer server(80);
WiFiClient client;
Ticker flipper, flipper2; // таймер1-2
const int interval = 5 * 60; // 5 минут Отправка на сервер
const int interval2 = 10 * 60; // 10 минут Проверка сервисов
int tm = interval, tm2 = interval2;
void flip() {  tm--;}
void flip2() {  tm2--;}
//////////////////////////////////////////////
// DHT Sensor
int DHTPin = 14;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
float Temperature;
float Humidity;
///////////////////////////////////////////////
void setup() {
  Serial.begin(74880);
  pinMode(red, OUTPUT); //WiFi error
  pinMode(blue, OUTPUT);//Web error
  pinMode(green, OUTPUT);
  delay(100);
  Serial.println("Setup");
  pinMode(DHTPin, INPUT);
  dht.begin();
  WiFi.mode(WIFI_STA); // Выбираем только клиента чтобы не "фонить"
  WiFi.begin(ssid, password);
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000); digitalWrite(red, HIGH);
    Serial.print(".");
  }
  Serial.println("WiFi connected..!"); digitalWrite(red, LOW);
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());Serial.println(macadr);
  //////////////////////////////////////////////////////////////
  server.on("/", handle_OnConnect);//включаем сервер
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
  flipper.attach(1, flip);// запускаем таймер
}
void loop() {
  server.handleClient();
  timer1();
  timer2();
}
void handle_OnConnect() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  server.send(200, "text/html", SendHTML(Temperature, Humidity));
}
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}
String SendHTML(float Temperaturestat, float Humiditystat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<meta http-equiv=\"refresh\" content=\"345\">";
  ptr += "<title>ESP8266 Weather Report</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>ESP8266 Report</h1>\n";
  ptr += "<p>Temperature: ";
  ptr += (int)Temperaturestat;
  ptr += " *C</p>";
  ptr += "<p>Humidity: ";
  ptr += (int)Humiditystat;
  ptr += "%</p>";
  ptr += "<a href=\"/?\"\"><button> Update Temperature <BR>& Humidity </button></a><br />";
  ptr += "</p>";
  ptr += times.tmTime();
  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

void get2server(float Temperaturestat, float Humiditystat)
{ Serial.println(adr_server);
  if (client.connect(adr_server, 80))
  { client.print( "GET /esp/add_data.php?");
    Serial.println( "GET2serv");
    client.print("temperature=");
    client.print( Temperaturestat );
    client.print("&");
    client.print("&");
    client.print("humiditystat=");
    client.print( Humiditystat );
    client.print("&");
    client.print("mac=");
    client.print(macadr);
    client.print("&");
    client.print("cr=");
    client.print("136");
    client.println( " HTTP/1.1");
    client.print( "Host: " );
    client.println(adr_server);
    client.println( "Connection: close" );
    client.println();
    client.println();
    delay(200);
    /*      while (client.available())
      {
          char c = client.read();
            if ( c=='1')
            {
            buff=1;
            }
            if ( c=='0')
            {
             buff=0;
            }
          }
    */
    client.stop();
    client.flush();
    delay(100);
  }
  else
  { digitalWrite(blue, HIGH);
    client.stop();
    delay(1000);
    client.connect(adr_server, 80);
    if (client.connect(adr_server, 80)) {
      digitalWrite(blue, LOW);
    } else {
      digitalWrite(blue, HIGH);
    }
    Serial.println( "error connect WEB server??");
  }

  //  if ( buff==1)
  //{
  //digitalWrite (led1, HIGH);
  //}
  //else
  //{
  //          digitalWrite(led1, LOW);
  //}
  //delay(500);
}
void timer1()
{ if (tm < 0) {                     // если таймер отработал
    flipper.detach();                 // выключаем
    tm = interval;                    // сбрасываем переменную таймера
    Temperature = dht.readTemperature(); // Gets the values of the temperature
    Humidity = dht.readHumidity(); // Gets the values of the humidity
    delay(10);
    Serial.println("timerOK");
    get2server(Temperature, Humidity);// отправляем
    flipper.attach(1, flip);          // включаем прерывание по таймеру
  }}
void timer2()
{ if (tm2 < 0) {                     // если таймер отработал
    flipper2.detach();                 // выключаем
    tm2 = interval2;                    // сбрасываем переменную таймера
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000); digitalWrite(red, HIGH); Serial.print(".");
    }
    Serial.println("WiFi connected..!"); digitalWrite(red, LOW);
    delay(10);
    Serial.println("timer2OK");
    flipper2.attach(1, flip2);          // включаем прерывание по таймеру
  }}
