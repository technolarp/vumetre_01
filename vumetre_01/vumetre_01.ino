/*
   ----------------------------------------------------------------------------
   TECHNOLARP - https://technolarp.github.io/
   VU-METRE 01 - https://github.com/technolarp/vumetre_01
   version 1.0 - 01/2022
   ----------------------------------------------------------------------------
*/

/*
   ----------------------------------------------------------------------------
   Pour ce montage, vous avez besoin de 
   1 potentiometre 10Hohms
   3 ou + led neopixel
   ----------------------------------------------------------------------------
*/

/*
   ----------------------------------------------------------------------------
   PINOUT
   D0     NEOPIXEL
   A0     POTENTIOMETRE
   ----------------------------------------------------------------------------
*/

/*
TODO LIST
creer class potentiometre
scintillement dans fastled
scintillement rotatif 


check desactiver seuils dans html
update automatique min max pour chaque seuil

refaire html
cleanup code
*/

#include <Arduino.h>

// WIFI
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// WEBSOCKET
AsyncWebSocket ws("/ws");

char bufferWebsocket[300];
bool flagBufferWebsocket = false;

// CONFIG
#include "config.h"
M_config aConfig;

#define BUFFERSENDSIZE 600
char bufferToSend[BUFFERSENDSIZE];

// FASTLED
#include <technolarp_fastled.h>
M_fastled aFastled;

bool increaseBrightness = true;
uint8_t indexBrightness = 0;

// POTENTIOMETER
#define POTENTIOMETER_PIN A0
uint8_t indexLed = 0;
uint8_t indexLedLocal = 0;
uint8_t indexLedPrevious = 0;
bool uneFois = true;
bool blinkUneFois = true;

// STATUTS DE L OBJET
enum {
  OBJET_ACTIF = 0,
  OBJET_BLINK = 5
};

// HEARTBEAT
uint32_t previousMillisHB;
uint32_t intervalHB;

// TIME
uint32_t previousMillisBrightness;
uint32_t previousMillisRead;
uint32_t intervalRead;
/*
   ----------------------------------------------------------------------------
   SETUP
   ----------------------------------------------------------------------------
*/
void setup()
{
  Serial.begin(115200);

  // VERSION
  delay(500);
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("----------------------------------------------------------------------------"));
  Serial.println(F("TECHNOLARP - https://technolarp.github.io/"));
  Serial.println(F("SERRURE MEDFAN 02 - https://github.com/technolarp/serrure_mefan_02"));
  Serial.println(F("version 1.0 - 01/2022"));
  Serial.println(F("----------------------------------------------------------------------------"));
  
  // I2C RESET
  aConfig.i2cReset();
  
  // CONFIG OBJET
  Serial.println(F(""));
  Serial.println(F(""));
  aConfig.mountFS();
  aConfig.listDir("/");
  aConfig.listDir("/config");
  aConfig.listDir("/www");
  
  aConfig.printJsonFile("/config/objectconfig.txt");
  aConfig.readObjectConfig("/config/objectconfig.txt");

  aConfig.printJsonFile("/config/networkconfig.txt");
  aConfig.readNetworkConfig("/config/networkconfig.txt");

  // POTENTIOMETER
  pinMode(POTENTIOMETER_PIN, INPUT);

  // FASTLED
  aFastled.setNbLed(aConfig.objectConfig.activeLeds);
  // animation led de depart
  aFastled.animationDepart(50, aFastled.getNbLed()*2, CRGB::Blue);

  // WIFI
  WiFi.disconnect(true);

  Serial.println(F(""));
  Serial.println(F("connecting WiFi"));
  
  /*
  // AP MODE
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(aConfig.networkConfig.apIP, aConfig.networkConfig.apIP, aConfig.networkConfig.apNetMsk);
  bool apRC = WiFi.softAP(aConfig.networkConfig.apName, aConfig.networkConfig.apPassword);

  if (apRC)
  {
    Serial.println(F("AP WiFi OK"));
  }
  else
  {
    Serial.println(F("AP WiFi failed"));
  }

  // Print ESP soptAP IP Address
  Serial.print(F("softAPIP: "));
  Serial.println(WiFi.softAPIP());
  
  */
  // CLIENT MODE POUR DEBUG
  const char* ssid = "MYDEBUG";
  const char* password = "ppppppppppp";
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  if (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.println(F("WiFi Failed!"));
  }
  else
  {
    Serial.println(F("WiFi OK"));
  }
  /**/
  
  // Print ESP Local IP Address
  Serial.print(F("localIP: "));
  Serial.println(WiFi.localIP());
  
  // WEB SERVER
  // Route for root / web page
  server.serveStatic("/", LittleFS, "/www/").setDefaultFile("config.html");
  server.serveStatic("/config", LittleFS, "/config/");
  server.onNotFound(notFound);

  // WEBSOCKET
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Start server
  server.begin();

  // HEARTBEAT
  previousMillisHB = millis();
  intervalHB = 10000;

  previousMillisBrightness = millis();
  
  previousMillisRead = millis();
  intervalRead = 10;

  // SERIAL
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("START !!!"));
}
/*
   ----------------------------------------------------------------------------
   FIN DU SETUP
   ----------------------------------------------------------------------------
*/




/*
   ----------------------------------------------------------------------------
   LOOP
   ----------------------------------------------------------------------------
*/
void loop()
{  
  // WEBSOCKET
  ws.cleanupClients();

  // FASTLED
  aFastled.updateAnimation();

  // CONTROL BRIGHTNESS
  controlBrightness();
  
  // gerer le statut de la serrure
  switch (aConfig.objectConfig.statutActuel)
  {
    case OBJET_ACTIF:
      // la serrure est fermee
      vumetreActif();
      break;

    case OBJET_BLINK:
      // blink led pour identification
      vumetreBlink();
      break;
      
    default:
      // nothing
      break;
  }
    
  // traiter le buffer du websocket
  if (flagBufferWebsocket)
  {
    flagBufferWebsocket = false;
    handleWebsocketBuffer();
  }

  // HEARTBEAT
  if(millis() - previousMillisHB > intervalHB)
  {
    previousMillisHB = millis();

    // envoyer l'uptime
    sendUptime();
  }
}
/*
   ----------------------------------------------------------------------------
   FIN DU LOOP
   ----------------------------------------------------------------------------
*/





/*
   ----------------------------------------------------------------------------
   FONCTIONS ADDITIONNELLES
   ----------------------------------------------------------------------------
*/
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) 
{
   switch (type) 
    {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        // send config value to html
        sendObjectConfig();
        sendNetworkConfig();
        
        // send volatile info
        sendIndexLed();
        sendMaxLed();

        sendUptime();
        break;
        
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
        
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
        
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) 
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
  {
    data[len] = 0;
    sprintf(bufferWebsocket,"%s\n", (char*)data);
    Serial.print(len);
    Serial.print(bufferWebsocket);
    flagBufferWebsocket = true;
  }
}
  
void handleWebsocketBuffer()
{
    DynamicJsonDocument doc(JSONBUFFERSIZE);
    
    DeserializationError error = deserializeJson(doc, bufferWebsocket);
    if (error)
    {
      Serial.println(F("Failed to deserialize buffer"));
    }
    else
    {
        // write config or not
        bool writeObjectConfigFlag = false;
        bool sendObjectConfigFlag = false;
        bool writeNetworkConfigFlag = false;
        bool sendNetworkConfigFlag = false;
        
        // modif object config
        if (doc.containsKey("new_objectName"))
        {
          strlcpy(  aConfig.objectConfig.objectName,
                    doc["new_objectName"],
                    sizeof(aConfig.objectConfig.objectName));
  
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }
  
        if (doc.containsKey("new_objectId")) 
        {
          uint16_t tmpValeur = doc["new_objectId"];
          aConfig.objectConfig.objectId = checkValeur(tmpValeur,1,1000);
  
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }
  
        if (doc.containsKey("new_groupId")) 
        {
          uint16_t tmpValeur = doc["new_groupId"];
          aConfig.objectConfig.groupId = checkValeur(tmpValeur,1,1000);
          
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }
  
        if (doc.containsKey("new_activeLeds")) 
        {
          FastLED.clear(); 
          
          uint16_t tmpValeur = doc["new_activeLeds"];
          aConfig.objectConfig.activeLeds = checkValeur(tmpValeur,1,NB_LEDS_MAX);
          aFastled.setNbLed(aConfig.objectConfig.activeLeds);
          uneFois=true;
          
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }
        
        if (doc.containsKey("new_brightness"))
        {
          uint16_t tmpValeur = doc["new_brightness"];
          aConfig.objectConfig.brightness = checkValeur(tmpValeur,0,255);
          FastLED.setBrightness(aConfig.objectConfig.brightness);
          uneFois=true;
          
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }

        if (doc.containsKey("new_indexLed"))
        {
          uint8_t newIndexLed = doc["new_indexLed"];
          indexLed=newIndexLed;
          uneFois=true;
        }
      
        if (doc.containsKey("new_intervalScintillement"))
        {
          uint16_t tmpValeur = doc["new_intervalScintillement"];
          aConfig.objectConfig.intervalScintillement = checkValeur(tmpValeur,0,1000);
          
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }

        if (doc.containsKey("new_scintillementOnOff"))
        {
          uint16_t tmpValeur = doc["new_scintillementOnOff"];
          aConfig.objectConfig.scintillementOnOff = checkValeur(tmpValeur,0,1);

          if (aConfig.objectConfig.scintillementOnOff == 0)
          {
            FastLED.setBrightness(aConfig.objectConfig.brightness);
          }
          
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }

        if (doc.containsKey("new_nbSeuils"))
        {
          uint16_t tmpValeur = doc["new_nbSeuils"];
          aConfig.objectConfig.nbSeuils = checkValeur(tmpValeur,1,10);
          
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }

        if (doc.containsKey("new_seuils")) 
        {
          JsonArray newSeuils = doc["new_seuils"];
            
          for (uint8_t i=0;i<newSeuils.size();i++)
          {
            uint16_t tmpValeurX = newSeuils[i][0];
            uint16_t tmpValeurY = newSeuils[i][1];
            
            uint8_t x = checkValeur(tmpValeurX,0,9);
            uint8_t y = checkValeur(tmpValeurY,1,49);
  
            aConfig.objectConfig.seuils[x] = y;
          }
          
            
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;

          // update statut
          uneFois = true;
        }

        if (doc.containsKey("new_couleurs")) 
        {
          JsonArray newCouleur = doc["new_couleurs"];
  
          uint8_t i = newCouleur[0];
          char newColorStr[8];
          strncpy(newColorStr, newCouleur[1], 8);
            
          uint8_t r;
          uint8_t g;
          uint8_t b;
            
          convertStrToRGB(newColorStr, &r, &g, &b);
          aConfig.objectConfig.couleurs[i].red=r;
          aConfig.objectConfig.couleurs[i].green=g;
          aConfig.objectConfig.couleurs[i].blue=b;
            
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }
  
        if (doc.containsKey("new_statutActuel"))
        {
          aConfig.objectConfig.statutPrecedent=aConfig.objectConfig.statutActuel;
          
          uint16_t tmpValeur = doc["new_statutActuel"];
          aConfig.objectConfig.statutActuel=tmpValeur;

          uneFois=true;
          
          writeObjectConfigFlag = true;
          sendObjectConfigFlag = true;
        }
        
        // modif network config
        if (doc.containsKey("new_apName")) 
        {
          strlcpy(  aConfig.networkConfig.apName,
                    doc["new_apName"],
                    sizeof(aConfig.networkConfig.apName));
  
          // check for unsupported char
          checkCharacter(aConfig.networkConfig.apName, "ABCDEFGHIJKLMNOPQRSTUVWYZ", 'A');
          
          writeNetworkConfigFlag = true;
          sendNetworkConfigFlag = true;
        }
  
        if (doc.containsKey("new_apPassword")) 
        {
          strlcpy(  aConfig.networkConfig.apPassword,
                    doc["new_apPassword"],
                    sizeof(aConfig.networkConfig.apPassword));
  
          writeNetworkConfigFlag = true;
        }
  
        if (doc.containsKey("new_apIP")) 
        {
          char newIPchar[16] = "";
  
          strlcpy(  newIPchar,
                    doc["new_apIP"],
                    sizeof(newIPchar));
  
          IPAddress newIP;
          if (newIP.fromString(newIPchar))
          {
            Serial.println("valid IP");
            aConfig.networkConfig.apIP = newIP;
  
            writeNetworkConfigFlag = true;
          }
          
          sendNetworkConfigFlag = true;
        }
  
        if (doc.containsKey("new_apNetMsk")) 
        {
          char newNMchar[16] = "";
  
          strlcpy(  newNMchar,
                    doc["new_apNetMsk"],
                    sizeof(newNMchar));
  
          IPAddress newNM;
          if (newNM.fromString(newNMchar)) 
          {
            Serial.println("valid netmask");
            aConfig.networkConfig.apNetMsk = newNM;
  
            writeNetworkConfigFlag = true;
          }
  
          sendNetworkConfigFlag = true;
        }
        
        // actions sur le esp8266
        if ( doc.containsKey("new_restart") && doc["new_restart"]==1 )
        {
          Serial.println(F("RESTART RESTART RESTART"));
          ESP.restart();
        }
  
        if ( doc.containsKey("new_refresh") && doc["new_refresh"]==1 )
        {
          Serial.println(F("REFRESH"));

          sendObjectConfigFlag = true;
          sendNetworkConfigFlag = true;
        }

        if ( doc.containsKey("new_defaultObjectConfig") && doc["new_defaultObjectConfig"]==1 )
        {
          Serial.println(F("reset to default object config"));
          aConfig.writeDefaultObjectConfig("/config/objectconfig.txt");
          
          sendObjectConfigFlag = true;
          uneFois = true;
        }

        if ( doc.containsKey("new_defaultNetworkConfig") && doc["new_defaultNetworkConfig"]==1 )
        {
          Serial.println(F("reset to default network config"));
          aConfig.writeDefaultNetworkConfig("/config/networkconfig.txt");
          
          sendNetworkConfigFlag = true;
        }

        // modif config
        // write object config
        if (writeObjectConfigFlag)
        {
          writeObjectConfig();
        
          // update statut
          uneFois = true;
        }
  
        // resend object config
        if (sendObjectConfigFlag)
        {
          sendObjectConfig();
        }
  
        // write network config
        if (writeNetworkConfigFlag)
        {
          writeObjectConfig();
        }
  
        // resend network config
        if (sendNetworkConfigFlag)
        {
          sendNetworkConfig();
        }
    }
 
    // clear json buffer
    doc.clear();
}

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void checkCharacter(char* toCheck, char* allowed, char replaceChar)
{
  for (uint8_t i = 0; i < strlen(toCheck); i++)
  {
    if (!strchr(allowed, toCheck[i]))
    {
      toCheck[i]=replaceChar;
    }
    Serial.print(toCheck[i]);
  }
  Serial.println("");
}

uint16_t checkValeur(uint16_t valeur, uint16_t minValeur, uint16_t maxValeur)
{
  return(min(max(valeur,minValeur), maxValeur));
}

void sendIndexLed()
{
  char toSend[100];
  snprintf(toSend, 100, "{\"indexLed\":%i}", indexLedLocal);

  ws.textAll(toSend);
}

void sendUptime()
{
  uint32_t now = millis() / 1000;
  uint16_t days = now / 86400;
  uint16_t hours = (now%86400) / 3600;
  uint16_t minutes = (now%3600) / 60;
  uint16_t seconds = now % 60;
    
  char toSend[100];
  snprintf(toSend, 100, "{\"uptime\":\"%id %ih %im %is\"}", days, hours, minutes, seconds);

  ws.textAll(toSend);
}

void sendMaxLed()
{
  char toSend[20];
  snprintf(toSend, 20, "{\"maxLed\":%i}", NB_LEDS_MAX);
  
  ws.textAll(toSend);
}

void sendStatut()
{
  char toSend[100];
  snprintf(toSend, 100, "{\"statutActuel\":%i}", aConfig.objectConfig.statutActuel); 

  ws.textAll(toSend);
}

void sendObjectConfig()
{
  aConfig.stringJsonFile("/config/objectconfig.txt", bufferToSend, BUFFERSENDSIZE);
  ws.textAll(bufferToSend);
}

void writeObjectConfig()
{
  aConfig.writeObjectConfig("/config/objectconfig.txt");
}

void sendNetworkConfig()
{
  aConfig.stringJsonFile("/config/networkconfig.txt", bufferToSend, BUFFERSENDSIZE);
  ws.textAll(bufferToSend);
}

void writeNetworkConfig()
{
  aConfig.writeNetworkConfig("/config/networkconfig.txt");
}

void convertStrToRGB(const char * source, uint8_t* r, uint8_t* g, uint8_t* b)
{ 
  uint32_t  number = (uint32_t) strtol( &source[1], NULL, 16);
  
  // Split them up into r, g, b values
  *r = number >> 16;
  *g = number >> 8 & 0xFF;
  *b = number & 0xFF;
}

void controlBrightness()
{
  if (aConfig.objectConfig.scintillementOnOff)
  {
    if(millis() - previousMillisBrightness > aConfig.objectConfig.intervalScintillement)
    {
      previousMillisBrightness = millis();
  
      if (increaseBrightness)
      {
        indexBrightness+=1;
        if (indexBrightness>=250)
        {
          increaseBrightness=false;
        }
      }
      else
      {
        indexBrightness-=1;
        if (indexBrightness<5)
        {
          increaseBrightness=true;
        }
      }
      aFastled.setBrightness(map(indexBrightness,5,250,5,aConfig.objectConfig.brightness));
      aFastled.ledShow();
    }
  }
}

void vumetreActif()
{
  // read potentiometer
  if(millis() - previousMillisRead > intervalRead)
  {
    previousMillisRead = millis();

    uint32_t readValue = 0;
    uint8_t readCount = 5;
  
    for (uint16_t i=0;i<readCount;i++)
    {
      readValue += analogRead(POTENTIOMETER_PIN);
    }
    readValue /= readCount;
  
    indexLedLocal = map(readValue, 0, 1024, 0, aConfig.objectConfig.activeLeds);
  }

  // check modif de la valeur lue
  if (indexLedPrevious!=indexLedLocal)
  {
    indexLed=indexLedLocal;
    indexLedPrevious=indexLedLocal;

    sendIndexLed();

    uneFois=true;
  }

  if (uneFois)
  {
    uneFois=false;

    aFastled.allLedOff(false);
    for (uint8_t i=0;i<indexLed;i++)
    {
        CRGB couleurTmp = aConfig.objectConfig.couleurs[aConfig.objectConfig.nbSeuils-1];
        for (uint8_t j=aConfig.objectConfig.nbSeuils;j>0;j--)
        {
          if (i<aConfig.objectConfig.seuils[j-1])
          {
            couleurTmp = aConfig.objectConfig.couleurs[j-1]; 
          }
        }        
        aFastled.ledOn(i, couleurTmp, false);
    }
    aFastled.ledShow();
  }
}

void vumetreBlink()
{
  if (uneFois)
  {
    uneFois = false;
    Serial.println(F("VUMETRE BLINK"));

    sendStatut();

    aFastled.animationBlink02Start(100, 3000, CRGB::Blue, CRGB::Black);
  }

  // fin de l'animation blink
  if(!aFastled.isAnimActive()) 
  {
    uneFois = true;

    aConfig.objectConfig.statutActuel = aConfig.objectConfig.statutPrecedent;

    writeObjectConfig();
    sendObjectConfig();

    Serial.println(F("END VUMETRE BLINK "));
  }
}
/*
   ----------------------------------------------------------------------------
   FIN DES FONCTIONS ADDITIONNELLES
   ----------------------------------------------------------------------------
*/
