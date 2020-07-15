#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <Adafruit_NeoPixel.h>

#include <ESP8266WebServer.h>
#include <DNSServer.h>                    //For WiFiManager to work
#include <WiFiManager.h>

WiFiManager wm;

const char* ssid     = "Taylors Wi-Fi";
const char* password = "KarRuss0725";

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

char packet_buf[1024];

#define PIN 5
#define LEDS 75
#define PACKET_SZ ( (LEDS * 3) + 3 )

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);

int value = 0;

void setup(){
  Serial.begin(115200);
  WiFi.hostname("LED_STRIP");                 // Name to show on network
  delay(1000);
  WiFi.mode(WIFI_STA);                        //explicitly set mode, esp defaults to STA+AP
  Serial.println();
  Serial.println("Starting wifi connection");
  connectToWifi();
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Udp.begin(1234);
  strip.begin();
}



void connectToWifi() {
  if (WiFi.status() != WL_CONNECTED){
    const char* menu[] = {"wifi","info","param","sep","restart","exit"}; 
    wm.setMenu(menu,1);
    wm.setClass("invert");                                                  // dark theme web setup page
    Serial.println("Starting AP mode since connection failed or its not setup");
  if (!wm.autoConnect("RGB Light Controler")) {
    Serial.print("FATAL WI-FI CONNECT ERROER!!!");                          //If unable to setup, flash error
    delay(1000);
    ESP.restart();                                                          //reboot and try again
    delay(5000);
    }
  }
}

void reConnectWifi(){
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 }


void loop(){
  int noBytes = Udp.parsePacket();
  
  if (WiFi.status() != WL_CONNECTED){  //----Ensure Wi-Fi connection---
    reConnectWifi();
  }
  
  if( noBytes )
  {
    Serial.print("Received ");
    Serial.print(noBytes);
    Serial.print(" bytes\r\n");
    Udp.read(packet_buf, noBytes);

    if( noBytes == PACKET_SZ && packet_buf[0] == 0xAA )
      {
      unsigned short sum = 0;
      int checksum_0 = PACKET_SZ - 2;
      int checksum_1 = PACKET_SZ - 1;

      for( int i = 0; i < checksum_0; i++ )
        {
          sum += packet_buf[i];
        }
        
      //Test if valid write packet
      if( ( ( (unsigned short)packet_buf[checksum_0] << 8 ) | packet_buf[checksum_1] ) == sum )
      {
        for( int i = 0; i < LEDS; i++ )
        {
          int idx = 1 + ( 3 * i );
          
          strip.setPixelColor(i, strip.Color(packet_buf[idx], packet_buf[idx+1], packet_buf[idx+2]));
        }
        strip.show();
      }
    }
  }
}
