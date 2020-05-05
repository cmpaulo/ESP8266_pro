//Project: Temperature meter with ESP8266 :: Temperature and Humidity
//Autor: Claudio Paulo

#include <Wire.h>
#include <SSD1306Wire.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

//Dados da rede wifi
#define STASSID "insert the SSID"
#define STAPSK  "insert the password"
const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

//Pino de ligacao do DHT11
#define DHTPIN 5

//Define o tipo de sensor DHT
#define DHTTYPE DHT11

// Inicializa o display Oled SDA 2, SCL 14
SSD1306Wire  display(0x3c, 2, 14);

int minima = 99;
int hu;
int maxima = 0;
int t;

DHT dht(DHTPIN, DHTTYPE);

//Intervalo de tempo entre leituras
const long intervalo = 5000;

//Armazena o valor (tempo) da ultima leitura
unsigned long previousMillis = 0;

void setup()
{
  Serial.begin(115200);

  //Inicializa o Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("the network conected");
  Serial.println(ssid);
  Serial.print("Adress IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.begin();
  Serial.println("HTTP server started");

  //Inicializa o display Oled
  display.init();
  display.flipScreenVertically();

  //Inicializa o sensor de temperatura
  dht.begin();
}

void handleRoot()
{
  server.send(200, "text/plain", "Hi! the Temperature meter IoT!\n\n The current Temperature" + String(t) + " gradius Celcius\n the Max was " + String(maxima) + " gradius \n the Min" + String(minima) + " gradius\nHumidity relative "+String(hu)+" %");
}

void Atualiza_Temperatura_Display(int temperatura, int hmt)
{
  //Apaga o display
  display.clear();

  //Desenha as molduras
  display.drawRect(0, 0, 128, 16);
  display.drawRect(0, 16, 128, 48);
  display.drawLine(64, 16, 64, 64);
  
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 2, "Termometer IoT ESP8266");
  //Atualiza informacoes da temperatura
  display.setFont(ArialMT_Plain_24);
  display.drawString(32, 26, String(temperatura));
  display.drawCircle(52, 32, 2);

  //Atualiza humidade relativa do ar
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_24);
  display.drawString(73, 26, String(hmt));
  display.drawString(101, 26, "%");


  display.display();
}

void loop()
{
  unsigned long currentMillis = millis();

  //Verifica se o intervalo já foi atingido
  if (currentMillis - previousMillis >= intervalo)
  {
    //Armazena o valor da ultima leitura
    previousMillis = currentMillis;

    //Le a temperatura
    t = dht.readTemperature();
    hu = dht.readHumidity();

    //Mostra a temperatura no Serial Monitor
    Serial.print(F("Temperature: "));
    Serial.print(t);
    Serial.println(F("°C "));
   
    Serial.print(F("Humidity: "));
    Serial.print(hu);
    Serial.println(F("% "));
    //Atualiza as variaveis maxima e minima, se necessario
    if (t >= maxima){maxima = t;}
    if (t <= minima){minima = t;}

    //Envia as informacoes para o display
    Atualiza_Temperatura_Display(t, hu);
  }

  server.handleClient();
  MDNS.update();
}
