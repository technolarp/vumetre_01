/*
   ----------------------------------------------------------------------------
   TECHNOLARP - https://technolarp.github.io/
   VU-METRE 01 - https://github.com/technolarp/vumetre_01
   version 1.2.0 - 05/2025
   ----------------------------------------------------------------------------
*/

/*
   ----------------------------------------------------------------------------
   Pour ce montage, vous avez besoin de
   1 potentiometre 10Kohms
   3 ou + led ws2812b
   ----------------------------------------------------------------------------
*/

/*
   ----------------------------------------------------------------------------
   PINOUT
   D0     LED
   A0     POTENTIOMETRE
   ----------------------------------------------------------------------------
*/

/*
TODO LIST
handle horizontal matrix
https://github.com/FastLED/FastLED/blob/master/examples/XYMatrix/XYMatrix.ino

DONE
handle snake matrix
fix bug FASTLED.show()
fix brightness at startup
html fix " escape bug
html remove index.html
*/

#include <Arduino.h>

// WIFI
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// WEBSOCKET
AsyncWebSocket ws("/ws");

char bufferReceiveWebsocket[500];
bool flagReceiveWebsocket = false;

// CONFIG
#include "config.h"
M_config aConfig;

#define BUFFERSENDSIZE 900
char bufferSendWebsocket[BUFFERSENDSIZE];

// FASTLED
#include <technolarp_fastled.h>
M_fastled aFastled;

// // POTENTIOMETRE
#include <technolarp_potentiometre.h>
M_potentiometre aPotentiometre;

// DIVERS
uint8_t indexLed = 0;
uint8_t indexLedLocal = 0;
uint8_t indexLedPrevious = 0;
bool uneFois = true;

// STATUTS DE L OBJET
enum
{
  OBJET_ACTIF = 0,
  OBJET_BLINK = 5
};

// HEARTBEAT
uint32_t previousMillisHB;
uint32_t intervalHB;

// READ POTENTIOMETER
uint32_t previousMillisRead;
uint32_t intervalRead;

// FUNCTION DECLARATIONS
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void handleWebsocketBuffer();
void notFound(AsyncWebServerRequest *request);

void checkCharacter(char *toCheck, const char *allowed, char replaceChar);
uint16_t checkValeur(uint16_t valeur, uint16_t minValeur, uint16_t maxValeur);
void sendIndexLed();
void sendUptime();
void sendMaxLed();
void sendStatut();
void sendObjectConfig();
void writeObjectConfig();
void sendNetworkConfig();
void writeNetworkConfig();
void convertStrToRGB(const char *source, uint8_t *r, uint8_t *g, uint8_t *b);
void vumetreActif();
void vumetreBlink();
uint8_t XY(uint8_t x, uint8_t y);

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
  Serial.println(F("VU-METRE - https://github.com/technolarp/vumetre_01"));
  Serial.println(F("version 1.2.0 - 05/2025"));
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

  Serial.println(F(""));
  aConfig.printJsonFile("/config/networkconfig.txt");
  aConfig.readNetworkConfig("/config/networkconfig.txt");

  // FASTLED
  aFastled.setNbLed(aConfig.objectConfig.activeLeds);
  aFastled.setControlBrightness(aConfig.objectConfig.scintillementOnOff);
  aFastled.setIntervalControlBrightness(aConfig.objectConfig.intervalScintillement);
  aFastled.setBrightness(aConfig.objectConfig.brightness);

  // animation led de depart
  aFastled.animationDepart(50, aFastled.getNbLed() * 2, CRGB::Blue);

  // WIFI
  WiFi.disconnect(true);

  Serial.println(F(""));
  Serial.println(F("connecting WiFi"));

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

  /*
  // CLIENT MODE POUR DEBUG
  const char *ssid = "SSID";
  const char *password = "PASSWORD";
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

  // Print ESP Local IP Address
  Serial.print(F("localIP: "));
  Serial.println(WiFi.localIP());
  */

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

  previousMillisRead = millis();
  intervalRead = 50;

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
  aFastled.controlBrightness(aConfig.objectConfig.brightness);

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
  if (flagReceiveWebsocket)
  {
    flagReceiveWebsocket = false;
    handleWebsocketBuffer();
  }

  // HEARTBEAT
  if (millis() - previousMillisHB > intervalHB)
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

  case WS_EVT_PING:
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    sprintf(bufferReceiveWebsocket, "%s\n", (char *)data);
    Serial.print(len);
    Serial.print(bufferReceiveWebsocket);
    flagReceiveWebsocket = true;
  }
}

void handleWebsocketBuffer()
{
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, bufferReceiveWebsocket);
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
    if (doc["new_objectName"].is<const char*>())
    {
      strlcpy(aConfig.objectConfig.objectName,
              doc["new_objectName"],
              sizeof(aConfig.objectConfig.objectName));

      char const * listeCheck = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 _-";
      checkCharacter(aConfig.objectConfig.objectName, listeCheck, '_');

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_objectId"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_objectId"];
      aConfig.objectConfig.objectId = checkValeur(tmpValeur, 1, 1000);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_groupId"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_groupId"];
      aConfig.objectConfig.groupId = checkValeur(tmpValeur, 1, 1000);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_activeLeds"].is<unsigned short>())
    {
      FastLED.clear();

      uint16_t tmpValeur = doc["new_activeLeds"];
      aConfig.objectConfig.activeLeds = checkValeur(tmpValeur, 1, NB_LEDS_MAX);
      aFastled.setNbLed(aConfig.objectConfig.activeLeds);
      uneFois = true;

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_brightness"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_brightness"];
      aConfig.objectConfig.brightness = checkValeur(tmpValeur, 0, 255);
      FastLED.setBrightness(aConfig.objectConfig.brightness);
      uneFois = true;

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_indexLed"].is<unsigned char>())
    {
      uint8_t newIndexLed = doc["new_indexLed"];
      indexLed = newIndexLed;
      uneFois = true;
    }

    if (doc["new_intervalScintillement"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_intervalScintillement"];
      aConfig.objectConfig.intervalScintillement = checkValeur(tmpValeur, 0, 1000);
      aFastled.setIntervalControlBrightness(aConfig.objectConfig.intervalScintillement);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_scintillementOnOff"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_scintillementOnOff"];
      aConfig.objectConfig.scintillementOnOff = checkValeur(tmpValeur, 0, 1);
      aFastled.setControlBrightness(aConfig.objectConfig.scintillementOnOff);

      if (aConfig.objectConfig.scintillementOnOff == 0)
      {
        FastLED.setBrightness(aConfig.objectConfig.brightness);
      }

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_nbSeuils"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_nbSeuils"];
      aConfig.objectConfig.nbSeuils = checkValeur(tmpValeur, 1, 10);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_seuils"].is<JsonVariant>())
    {
      JsonArray newSeuils = doc["new_seuils"];

      for (uint8_t i = 0; i < newSeuils.size(); i++)
      {
        uint16_t tmpValeurX = newSeuils[i][0];
        uint16_t tmpValeurY = newSeuils[i][1];

        uint8_t x = checkValeur(tmpValeurX, 0, 9);
        uint8_t y = checkValeur(tmpValeurY, 1, 49);

        aConfig.objectConfig.seuils[x] = y;
      }

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;

      // update statut
      uneFois = true;
    }

    if (doc["new_couleurs"].is<JsonVariant>())
    {
      JsonArray newCouleur = doc["new_couleurs"];

      uint8_t i = newCouleur[0];
      char newColorStr[8];
      strncpy(newColorStr, newCouleur[1], 8);

      uint8_t r;
      uint8_t g;
      uint8_t b;

      convertStrToRGB(newColorStr, &r, &g, &b);
      aConfig.objectConfig.couleurs[i].red = r;
      aConfig.objectConfig.couleurs[i].green = g;
      aConfig.objectConfig.couleurs[i].blue = b;

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_statutActuel"].is<unsigned short>())
    {
      aConfig.objectConfig.statutPrecedent = aConfig.objectConfig.statutActuel;

      uint16_t tmpValeur = doc["new_statutActuel"];
      aConfig.objectConfig.statutActuel = tmpValeur;

      uneFois = true;

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_nbLedAffiche"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_nbLedAffiche"];
      aConfig.objectConfig.nbLedAffiche = checkValeur(tmpValeur, 0, aConfig.objectConfig.activeLeds);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_potentiometreActif"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_potentiometreActif"];
      aConfig.objectConfig.potentiometreActif = checkValeur(tmpValeur, 0, 1);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_uneSeuleCouleur"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_uneSeuleCouleur"];
      aConfig.objectConfig.uneSeuleCouleur = checkValeur(tmpValeur, 0, 1);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_ledSnakeMatrix"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_ledSnakeMatrix"];
      aConfig.objectConfig.ledSnakeMatrix = checkValeur(tmpValeur, 0, 1);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_nbLignes"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_nbLignes"];
      aConfig.objectConfig.nbLignes = checkValeur(tmpValeur, 1, 10);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    if (doc["new_nbColonnes"].is<unsigned short>())
    {
      uint16_t tmpValeur = doc["new_nbColonnes"];
      aConfig.objectConfig.nbColonnes = checkValeur(tmpValeur, 1, 10);

      writeObjectConfigFlag = true;
      sendObjectConfigFlag = true;
    }

    // modif network config
    if (doc["new_apName"].is<const char*>())
    {
      strlcpy(aConfig.networkConfig.apName,
              doc["new_apName"],
              sizeof(aConfig.networkConfig.apName));

      // check for unsupported char
      char const * listeCheck = "ABCDEFGHIJKLMNOPQRSTUVWYXZ0123456789_-";
      checkCharacter(aConfig.networkConfig.apName, listeCheck, 'A');

      writeNetworkConfigFlag = true;
      sendNetworkConfigFlag = true;
    }

    if (doc["new_apPassword"].is<const char*>())
    {
      strlcpy(aConfig.networkConfig.apPassword,
              doc["new_apPassword"],
              sizeof(aConfig.networkConfig.apPassword));

      writeNetworkConfigFlag = true;
      sendNetworkConfigFlag = true;
    }

    if (doc["new_apIP"].is<const char*>())
    {
      char newIPchar[16] = "";

      strlcpy(newIPchar,
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

    if (doc["new_apNetMsk"].is<const char*>())
    {
      char newNMchar[16] = "";

      strlcpy(newNMchar,
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
    if (doc["new_restart"].is<unsigned char>() && doc["new_restart"] == 1)
    {
      Serial.println(F("RESTART RESTART RESTART"));
      ESP.restart();
    }

    if (doc["new_refresh"].is<unsigned char>() && doc["new_refresh"] == 1)
    {
      Serial.println(F("REFRESH"));

      sendObjectConfigFlag = true;
      sendNetworkConfigFlag = true;
    }

    if (doc["new_defaultObjectConfig"].is<unsigned char>() && doc["new_defaultObjectConfig"] == 1)
    {
      aConfig.writeDefaultObjectConfig("/config/objectconfig.txt");
      Serial.println(F("reset to default object config"));

      aFastled.allLedOff();
      aFastled.setNbLed(aConfig.objectConfig.activeLeds);
      FastLED.setBrightness(aConfig.objectConfig.brightness);
      aFastled.setControlBrightness(aConfig.objectConfig.scintillementOnOff);
      aFastled.setIntervalControlBrightness(aConfig.objectConfig.intervalScintillement);

      sendObjectConfigFlag = true;
      uneFois = true;
    }

    if (doc["new_defaultNetworkConfig"].is<unsigned char>() && doc["new_defaultNetworkConfig"] == 1)
    {
      aConfig.writeDefaultNetworkConfig("/config/networkconfig.txt");
      Serial.println(F("reset to default network config"));

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
      writeNetworkConfig();
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

void checkCharacter(char *toCheck, const char *allowed, char replaceChar)
{
  for (uint8_t i = 0; i < strlen(toCheck); i++)
  {
    if (!strchr(allowed, toCheck[i]))
    {
      toCheck[i] = replaceChar;
    }
    Serial.print(toCheck[i]);
  }
  Serial.println("");
}

uint16_t checkValeur(uint16_t valeur, uint16_t minValeur, uint16_t maxValeur)
{
  return (min(max(valeur, minValeur), maxValeur));
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
  uint16_t hours = (now % 86400) / 3600;
  uint16_t minutes = (now % 3600) / 60;
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
  aConfig.stringJsonFile("/config/objectconfig.txt", bufferSendWebsocket, BUFFERSENDSIZE);
  ws.textAll(bufferSendWebsocket);
}

void writeObjectConfig()
{
  aConfig.writeObjectConfig("/config/objectconfig.txt");
}

void sendNetworkConfig()
{
  aConfig.stringJsonFile("/config/networkconfig.txt", bufferSendWebsocket, BUFFERSENDSIZE);
  ws.textAll(bufferSendWebsocket);
}

void writeNetworkConfig()
{
  aConfig.writeNetworkConfig("/config/networkconfig.txt");
}

void convertStrToRGB(const char *source, uint8_t *r, uint8_t *g, uint8_t *b)
{
  uint32_t number = (uint32_t)strtol(&source[1], NULL, 16);

  // Split them up into r, g, b values
  *r = number >> 16;
  *g = number >> 8 & 0xFF;
  *b = number & 0xFF;
}

void vumetreActif()
{
  // update indexLocal by reading potentiometer
  if ((millis() - previousMillisRead) > intervalRead)
  {
    previousMillisRead = millis();

    if (aConfig.objectConfig.potentiometreActif)
    {
      uint16_t potValue = aPotentiometre.readPotValue();
      indexLedLocal = map(potValue, 15, 1010, 0, aConfig.objectConfig.activeLeds);
      indexLedLocal = checkValeur(indexLedLocal, 0, aConfig.objectConfig.activeLeds);
    }
  }

  // check if potentiometer value change
  if (indexLedPrevious != indexLedLocal)
  {
    indexLed = indexLedLocal;
    indexLedPrevious = indexLedLocal;

    sendIndexLed();

    uneFois = true;
  }

  if (uneFois)
  {
    uneFois = false;

    // update the conversion index=>color array
    for (uint8_t i = 0; i < aConfig.objectConfig.activeLeds; i++)
    {
      aFastled.indextoSeuil[i] = aConfig.objectConfig.nbSeuils - 1;
      for (uint8_t j = aConfig.objectConfig.nbSeuils; j > 0; j--)
      {
        if (i < aConfig.objectConfig.seuils[j - 1])
        {
          aFastled.indextoSeuil[i] = j - 1;
        }
      }
    }

    // find the start led
    uint8_t startLed = 0;
    if (aConfig.objectConfig.nbLedAffiche > 0)
    {
      startLed = max(0, indexLed - aConfig.objectConfig.nbLedAffiche);
    }

    // update led color
    aFastled.allLedOff(false);

    if (aConfig.objectConfig.ledSnakeMatrix == 0)
    {
      // led matrix is linear
      for (uint8_t i = 0; i < aConfig.objectConfig.activeLeds; i++)
      {
        aFastled.indexMatrix[i] = i;
      }
    }
    else
    {
      // led matrix is in snake shape
      for (uint8_t i = 0; i < aConfig.objectConfig.nbLignes; i++)
      {
        for (uint8_t j = 0; j < aConfig.objectConfig.nbColonnes; j++)
        {
          // even line, increase indexMatrix from left to right
          if (i % 2 == 0)
          {
            aFastled.indexMatrix[i * aConfig.objectConfig.nbColonnes + j] = i * aConfig.objectConfig.nbColonnes + j;
          }
          // odd line, increase from indexMatrix right to left
          else
          {
            aFastled.indexMatrix[i * aConfig.objectConfig.nbColonnes + j] = i * aConfig.objectConfig.nbColonnes + aConfig.objectConfig.nbColonnes - 1 - j;
          }
        }
      }
    }

    // update led color using indexMatrix & indexToSeuil
    aFastled.allLedOff(false);
    for (uint8_t i = startLed; i < indexLed; i++)
    {
      if (aConfig.objectConfig.uneSeuleCouleur == 0)
      {
        aFastled.ledOn(aFastled.indexMatrix[i], aConfig.objectConfig.couleurs[aFastled.indextoSeuil[i]], false);
      }
      else
      {
        aFastled.ledOn(aFastled.indexMatrix[i], aConfig.objectConfig.couleurs[aFastled.indextoSeuil[indexLed - 1]], false);
      }
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
  if (!aFastled.isAnimActive())
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