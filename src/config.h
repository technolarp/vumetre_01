#include <LittleFS.h>
#include <ArduinoJson.h> // arduino json v7  // https://github.com/bblanchon/ArduinoJson

#define SIZE_ARRAY 33
#define SIZE_PWD 64
#define NB_COULEURS 10
#define WIFI_CLIENTS 5

#include <IPAddress.h>
#include <FastLED.h>

class M_config
{
public:
  // structure stockage
  struct OBJECT_CONFIG_STRUCT
  {
    uint16_t objectId;
    uint16_t groupId;

    char objectName[SIZE_ARRAY];

    uint8_t activeLeds;
    uint8_t brightness;

    uint16_t intervalScintillement;
    uint16_t scintillementOnOff;

    CRGB couleurs[NB_COULEURS];

    uint8_t nbSeuils;
    uint8_t seuils[NB_COULEURS];
    uint8_t nbLedAffiche;

    uint8_t statutActuel;
    uint8_t statutPrecedent;

    uint8_t potentiometreActif;
    uint8_t uneSeuleCouleur;

    uint8_t ledSnakeMatrix;
    uint8_t nbLignes;
    uint8_t nbColonnes;
  };

  // creer une structure
  OBJECT_CONFIG_STRUCT objectConfig;

  struct NETWORK_CONFIG_STRUCT
  {
    uint8_t wifiConnectDelay = 5;

    char ssid[WIFI_CLIENTS][SIZE_ARRAY];
    char password[WIFI_CLIENTS][SIZE_PWD];
    bool active[WIFI_CLIENTS];

    bool disableSsid = false;
    bool rebootEsp = false;
    
    IPAddress apIP;
    IPAddress apNetMsk;
    char apName[SIZE_ARRAY];
    char apPassword[SIZE_PWD];
  };

  // creer une structure
  NETWORK_CONFIG_STRUCT networkConfig;

  M_config()
  {
  }

  void readObjectConfig(const char *filename)
  {
    // lire les données depuis le fichier littleFS
    // Open file for reading
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
      Serial.println(F("Failed to open file for reading"));
      return;
    }

    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      Serial.println(F("Failed to deserialize file in read object"));
      Serial.println(error.c_str());
    }
    else
    {
      // Copy values from the JsonObject to the Config
      objectConfig.objectId = doc["objectId"];
      objectConfig.groupId = doc["groupId"];

      objectConfig.activeLeds = doc["activeLeds"];
      objectConfig.brightness = doc["brightness"];
      objectConfig.intervalScintillement = doc["intervalScintillement"];
      objectConfig.scintillementOnOff = doc["scintillementOnOff"];

      objectConfig.statutActuel = doc["statutActuel"];
      objectConfig.statutPrecedent = doc["statutPrecedent"];

      objectConfig.nbSeuils = doc["nbSeuils"];
      objectConfig.nbLedAffiche = doc["nbLedAffiche"];

      objectConfig.potentiometreActif = doc["potentiometreActif"];
      objectConfig.uneSeuleCouleur = doc["uneSeuleCouleur"];

      objectConfig.ledSnakeMatrix = doc["ledSnakeMatrix"];
      objectConfig.nbLignes = doc["nbLignes"];
      objectConfig.nbColonnes = doc["nbColonnes"];

      if (doc["couleurs"].is<JsonVariant>())
      {
        JsonArray couleurArray = doc["couleurs"];

        for (uint8_t i = 0; i < NB_COULEURS; i++)
        {
          JsonArray rgbArray = couleurArray[i];

          objectConfig.couleurs[i].red = rgbArray[0];
          objectConfig.couleurs[i].green = rgbArray[1];
          objectConfig.couleurs[i].blue = rgbArray[2];
        }
      }

      if (doc["seuils"].is<JsonVariant>())
      {
        JsonArray seuilsArray = doc["seuils"];

        for (uint8_t i = 0; i < NB_COULEURS; i++)
        {
          objectConfig.seuils[i] = seuilsArray[i];
        }
      }

      // read object name
      if (doc["objectName"].is<JsonVariant>())
      {
        strlcpy(objectConfig.objectName,
                doc["objectName"],
                SIZE_ARRAY);
      }
    }

    // Close the file (File's destructor doesn't close the file)
    file.close();
  }

  void writeObjectConfig(const char *filename)
  {
    // Delete existing file, otherwise the configuration is appended to the file
    LittleFS.remove(filename);

    // Open file for writing
    File file = LittleFS.open(filename, "w");
    if (!file)
    {
      Serial.println(F("Failed to create file"));
      return;
    }

    // Allocate a temporary JsonDocument
    JsonDocument doc;

    doc["objectName"] = objectConfig.objectName;

    doc["objectId"] = objectConfig.objectId;
    doc["groupId"] = objectConfig.groupId;

    doc["activeLeds"] = objectConfig.activeLeds;
    doc["brightness"] = objectConfig.brightness;
    doc["intervalScintillement"] = objectConfig.intervalScintillement;
    doc["scintillementOnOff"] = objectConfig.scintillementOnOff;

    doc["statutActuel"] = objectConfig.statutActuel;
    doc["statutPrecedent"] = objectConfig.statutPrecedent;

    doc["nbSeuils"] = objectConfig.nbSeuils;
    doc["nbLedAffiche"] = objectConfig.nbLedAffiche;

    doc["potentiometreActif"] = objectConfig.potentiometreActif;
    doc["uneSeuleCouleur"] = objectConfig.uneSeuleCouleur;

    doc["ledSnakeMatrix"] = objectConfig.ledSnakeMatrix;
    doc["nbLignes"] = objectConfig.nbLignes;
    doc["nbColonnes"] = objectConfig.nbColonnes;

    JsonArray seuilsArray = doc["seuils"].to<JsonArray>();

    for (uint8_t i = 0; i < NB_COULEURS; i++)
    {
      seuilsArray.add(objectConfig.seuils[i]);
    }

    JsonArray couleurArray = doc["couleurs"].to<JsonArray>();

    for (uint8_t i = 0; i < NB_COULEURS; i++)
    {
      JsonArray couleur_x = couleurArray.add<JsonArray>();

      couleur_x.add(objectConfig.couleurs[i].red);
      couleur_x.add(objectConfig.couleurs[i].green);
      couleur_x.add(objectConfig.couleurs[i].blue);
    }

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
      Serial.println(F("Failed to write to file"));
    }

    // Close the file (File's destructor doesn't close the file)
    file.close();
  }

  void writeDefaultObjectConfig(const char *filename)
  {
    objectConfig.objectId = 1;
    objectConfig.groupId = 1;

    objectConfig.activeLeds = 8;
    objectConfig.brightness = 80;

    objectConfig.intervalScintillement = 50;
    objectConfig.scintillementOnOff = 0;

    objectConfig.statutActuel = 0;
    objectConfig.statutPrecedent = 0;

    objectConfig.nbSeuils = 3;
    objectConfig.nbLedAffiche = 0;

    objectConfig.potentiometreActif = 1;
    objectConfig.uneSeuleCouleur = 0;

    objectConfig.ledSnakeMatrix = 0;
    objectConfig.nbLignes = 2;
    objectConfig.nbColonnes = 2;

    for (uint8_t i = 0; i < NB_COULEURS; i++)
    {
      objectConfig.seuils[i] = 2 * (i + 1);
    }

    // couleur 0
    objectConfig.couleurs[0].red = 0;
    objectConfig.couleurs[0].green = 255;
    objectConfig.couleurs[0].blue = 0;

    // couleur 1
    objectConfig.couleurs[1].red = 255;
    objectConfig.couleurs[1].green = 128;
    objectConfig.couleurs[1].blue = 0;

    // couleur 2
    objectConfig.couleurs[2].red = 255;
    objectConfig.couleurs[2].green = 0;
    objectConfig.couleurs[2].blue = 0;

    // couleur 3
    objectConfig.couleurs[3].red = 255;
    objectConfig.couleurs[3].green = 255;
    objectConfig.couleurs[3].blue = 0;

    // couleur 4
    objectConfig.couleurs[4].red = 0;
    objectConfig.couleurs[4].green = 255;
    objectConfig.couleurs[4].blue = 255;

    // couleur 5
    objectConfig.couleurs[5].red = 255;
    objectConfig.couleurs[5].green = 0;
    objectConfig.couleurs[5].blue = 255;

    // couleur 6
    objectConfig.couleurs[6].red = 255;
    objectConfig.couleurs[6].green = 255;
    objectConfig.couleurs[6].blue = 0;

    // couleur 7
    objectConfig.couleurs[7].red = 0;
    objectConfig.couleurs[7].green = 255;
    objectConfig.couleurs[7].blue = 255;

    // couleur 8
    objectConfig.couleurs[8].red = 255;
    objectConfig.couleurs[8].green = 0;
    objectConfig.couleurs[8].blue = 255;

    // couleur 9
    objectConfig.couleurs[9].red = 255;
    objectConfig.couleurs[9].green = 255;
    objectConfig.couleurs[9].blue = 0;

    strlcpy(objectConfig.objectName,
            "vu-metre 01",
            SIZE_ARRAY);

    writeObjectConfig(filename);
  }

  void readNetworkConfig(const char *filename)
  {
    // lire les données depuis le fichier littleFS
    // Open file for reading
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
      Serial.println(F("Failed to open file for reading"));
      return;
    }

    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      Serial.println(F("Failed to deserialize file in read network "));
      Serial.println(error.c_str());
    }
    else
    {
       // parse wifi ssid list
      if (doc["wifiClientsConfig"]["wifiClientsList"].is<JsonVariant>())
      {
        JsonArray wifiClientArray = doc["wifiClientsConfig"]["wifiClientsList"];

        for (uint8_t i = 0; i < WIFI_CLIENTS; i++)
        {
          if (wifiClientArray[i]["ssid"].is<JsonVariant>())
          {
            strlcpy(networkConfig.ssid[i], wifiClientArray[i]["ssid"], SIZE_ARRAY);
            strlcpy(networkConfig.password[i], wifiClientArray[i]["password"], SIZE_PWD);
            networkConfig.active[i]=wifiClientArray[i]["active"].as<boolean>();
          }
        }

        wifiClientArray.clear();
      }

      if (doc["wifiClientsConfig"]["wifiConnectDelay"].is<unsigned short>())
      {
        networkConfig.wifiConnectDelay=doc["wifiClientsConfig"]["wifiConnectDelay"];
      }

      if (doc["wifiClientsConfig"]["disableSsid"].is<boolean>())
      {
        networkConfig.disableSsid=doc["wifiClientsConfig"]["disableSsid"].as<boolean>();
      }

      if (doc["wifiClientsConfig"]["rebootEsp"].is<boolean>())
      {
        networkConfig.rebootEsp=doc["wifiClientsConfig"]["rebootEsp"].as<boolean>();
      }

      // parse wifi AP config
      if (doc["wifiAPConfig"]["apIP"].is<JsonVariant>())
      {
        JsonArray apIPArray = doc["wifiAPConfig"]["apIP"];

        for (uint8_t i = 0; i < 4; i++)
        {
          networkConfig.apIP[i] = apIPArray[i];
        }

        apIPArray.clear();
      }

      if (doc["wifiAPConfig"]["apNetMsk"].is<JsonVariant>())
      {
        JsonArray apNetMskArray = doc["wifiAPConfig"]["apNetMsk"];

        for (uint8_t i = 0; i < 4; i++)
        {
          networkConfig.apNetMsk[i] = apNetMskArray[i];
        }

        apNetMskArray.clear();
      }

      if (doc["wifiAPConfig"]["apName"].is<JsonVariant>())
      {
        strlcpy(networkConfig.apName,
                doc["wifiAPConfig"]["apName"],
                SIZE_ARRAY);
      }

      if (doc["wifiAPConfig"]["apPassword"].is<JsonVariant>())
      {
        strlcpy(networkConfig.apPassword,
                doc["wifiAPConfig"]["apPassword"],
                SIZE_PWD);
      }

      // check AP config & create default config 
      if (strlen(networkConfig.apName) == 0)
      {
        Serial.println(F("no apName in config, creating default"));
        
        // default AP config
        networkConfig.apIP[0] = 192;
        networkConfig.apIP[1] = 168;
        networkConfig.apIP[2] = 1;
        networkConfig.apIP[3] = 1;

        networkConfig.apNetMsk[0] = 255;
        networkConfig.apNetMsk[1] = 255;
        networkConfig.apNetMsk[2] = 255;
        networkConfig.apNetMsk[3] = 0;

        snprintf(networkConfig.apName, SIZE_ARRAY, "TECHNOLARP_%04d", (ESP.getChipId() & 0xFFFF));
        strlcpy(networkConfig.apPassword, "", SIZE_PWD);
      }
    }

    doc.clear();
    
    // Close the file (File's destructor doesn't close the file)
    file.close();
  }

  void writeNetworkConfig(const char *filename)
  {
    // Delete existing file, otherwise the configuration is appended to the file
    LittleFS.remove(filename);

    // Open file for writing
    File file = LittleFS.open(filename, "w");
    if (!file)
    {
      Serial.println(F("Failed to create file"));
      return;
    }

    // Allocate a temporary JsonDocument
    JsonDocument doc;

    JsonArray arraySsid = doc["wifiClientsConfig"]["wifiClientsList"].to<JsonArray>();
    for (uint8_t i = 0; i < WIFI_CLIENTS; i++)
    {
      JsonDocument docSsid;
      docSsid["ssid"] = networkConfig.ssid[i];
      docSsid["password"] = networkConfig.password[i];
      docSsid["active"] = networkConfig.active[i];

      arraySsid.add(docSsid);
    }

    doc["wifiClientsConfig"]["wifiConnectDelay"] = networkConfig.wifiConnectDelay;
    doc["wifiClientsConfig"]["disableSsid"] = networkConfig.disableSsid;
    doc["wifiClientsConfig"]["rebootEsp"] = networkConfig.rebootEsp;
    
    doc["wifiAPConfig"]["apName"] = networkConfig.apName;
    doc["wifiAPConfig"]["apPassword"] = networkConfig.apPassword;

    JsonArray arrayIp = doc["wifiAPConfig"]["apIP"].to<JsonArray>();
    for (uint8_t i = 0; i < 4; i++)
    {
      arrayIp.add(networkConfig.apIP[i]);
    }

    JsonArray arrayNetMask = doc["wifiAPConfig"]["apNetMsk"].to<JsonArray>();
    for (uint8_t i = 0; i < 4; i++)
    {
      arrayNetMask.add(networkConfig.apNetMsk[i]);
    }

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
      Serial.println(F("Failed to write to file"));
    }

    // Close the file (File's destructor doesn't close the file)
    file.close();
  }

  void writeDefaultNetworkConfig(const char *filename)
  {
    for (uint8_t i = 0; i < WIFI_CLIENTS; i++)
    {
      char buffer[SIZE_ARRAY];
      snprintf(buffer, SIZE_ARRAY, "SSID_%d", i + 1);
      strlcpy(networkConfig.ssid[i],
              buffer,
              SIZE_ARRAY);

      snprintf(buffer, SIZE_ARRAY, "password%d", i + 1);
      strlcpy(networkConfig.password[i],
              buffer,
              SIZE_ARRAY);
      
      networkConfig.active[i] = false;
    }

    networkConfig.wifiConnectDelay = 15;  
    networkConfig.disableSsid = false;
    networkConfig.rebootEsp = false;
    
    strlcpy(networkConfig.apName,
            "VUMETRE_1",
            SIZE_ARRAY);

    strlcpy(networkConfig.apPassword,
            "",
            SIZE_ARRAY);

    networkConfig.apIP[0] = 192;
    networkConfig.apIP[1] = 168;
    networkConfig.apIP[2] = 1;
    networkConfig.apIP[3] = 1;

    networkConfig.apNetMsk[0] = 255;
    networkConfig.apNetMsk[1] = 255;
    networkConfig.apNetMsk[2] = 255;
    networkConfig.apNetMsk[3] = 0;

    writeNetworkConfig(filename);
  }

  void stringJsonFile(const char *filename, char *target, uint16_t targetReadSize)
  {
    // Open file for reading
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
      Serial.println(F("Failed to open file for reading"));
    }
    else
    {
      uint16_t cptRead = 0;
      while ((file.available()) && (cptRead < targetReadSize))
      {
        target[cptRead] = file.read();
        cptRead++;
      }

      if (cptRead < targetReadSize)
      {
        target[cptRead] = '\0';
      }
      else
      {
        target[targetReadSize] = '\0';
      }
    }

    // Close the file (File's destructor doesn't close the file)
    file.close();
  }

  void mountFS()
  {
    Serial.println(F("Mount LittleFS"));
    if (!LittleFS.begin())
    {
      Serial.println(F("LittleFS mount failed"));
      return;
    }
  }

  void printJsonFile(const char *filename)
  {
    // Open file for reading
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
      Serial.println(F("Failed to open file for reading"));
    }

    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      Serial.println(F("Failed to deserialize file in print object"));
      Serial.println(error.c_str());
    }
    else
    {
      // serializeJsonPretty(doc, Serial);
      Serial.print(measureJson(doc));
      serializeJson(doc, Serial);
      Serial.println();
    }

    // Close the file (File's destructor doesn't close the file)
    file.close();
  }

  void listDir(const char *dirname)
  {
    Serial.printf("Listing directory: %s", dirname);
    Serial.println();

    Dir root = LittleFS.openDir(dirname);

    while (root.next())
    {
      File file = root.openFile("r");
      Serial.print(F("  FILE: "));
      Serial.print(root.fileName());
      Serial.print(F("  SIZE: "));
      Serial.print(file.size());
      Serial.println();
      file.close();
    }
    Serial.println();
  }

  // I2C RESET
  void i2cReset()
  {
    uint8_t rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
    if (rtn != 0)
    {
      Serial.println(F("I2C bus error. Could not clear"));
      if (rtn == 1)
      {
        Serial.println(F("SCL clock line held low"));
      }
      else if (rtn == 2)
      {
        Serial.println(F("SCL clock line held low by slave clock stretch"));
      }
      else if (rtn == 3)
      {
        Serial.println(F("SDA data line held low"));
      }
    }
    else
    {
      // bus clear
      Serial.println(F("I2C bus clear"));
    }
  }

  /**
   * This routine turns off the I2C bus and clears it
   * on return SCA and SCL pins are tri-state inputs.
   * You need to call Wire.begin() after this to re-enable I2C
   * This routine does NOT use the Wire library at all.
   *
   * returns 0 if bus cleared
   *         1 if SCL held low.
   *         2 if SDA held low by slave clock stretch for > 2sec
   *         3 if SDA held low after 20 clocks.
   *
   * from:
   * https://github.com/esp8266/Arduino/issues/1025
   * http://www.forward.com.au/pfod/ArduinoProgramming/I2C_ClearBus/index.html
   */
  uint8_t I2C_ClearBus()
  {
#if defined(TWCR) && defined(TWEN)
    TWCR &= ~(_BV(TWEN)); // Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif
    pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
    pinMode(SCL, INPUT_PULLUP);

    delay(500); // Wait 2.5 secs. This is strictly only necessary on the first power
    // up of the DS3231 module to allow it to initialize properly,
    // but is also assists in reliable programming of FioV3 boards as it gives the
    // IDE a chance to start uploaded the program
    // before existing sketch confuses the IDE by sending Serial data.

    boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
    if (SCL_LOW)
    {           // If it is held low Arduno cannot become the I2C master.
      return 1; // I2C bus error. Could not clear SCL clock line held low
    }

    boolean SDA_LOW = (digitalRead(SDA) == LOW); // vi. Check SDA input.
    uint8_t clockCount = 20;                     // > 2x9 clock

    while (SDA_LOW && (clockCount > 0))
    { //  vii. If SDA is Low,
      clockCount--;
      // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
      pinMode(SCL, INPUT);        // release SCL pullup so that when made output it will be LOW
      pinMode(SCL, OUTPUT);       // then clock SCL Low
      delayMicroseconds(10);      //  for >5uS
      pinMode(SCL, INPUT);        // release SCL LOW
      pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
      // do not force high as slave may be holding it low for clock stretching.
      delayMicroseconds(10); //  for >5uS
      // The >5uS is so that even the slowest I2C devices are handled.
      SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
      uint8_t counter = 20;
      while (SCL_LOW && (counter > 0))
      { //  loop waiting for SCL to become High only wait 2sec.
        counter--;
        delay(100);
        SCL_LOW = (digitalRead(SCL) == LOW);
      }
      if (SCL_LOW)
      {           // still low after 2 sec error
        return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
      }
      SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
    }
    if (SDA_LOW)
    {           // still low
      return 3; // I2C bus error. Could not clear. SDA data line held low
    }

    // else pull SDA line low for Start or Repeated Start
    pinMode(SDA, INPUT);  // remove pullup.
    pinMode(SDA, OUTPUT); // and then make it LOW i.e. send an I2C Start or Repeated start control.
    // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
    /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
    delayMicroseconds(10);      // wait >5uS
    pinMode(SDA, INPUT);        // remove output low
    pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
    delayMicroseconds(10);      // x. wait >5uS
    pinMode(SDA, INPUT);        // and reset pins as tri-state inputs which is the default state on reset
    pinMode(SCL, INPUT);
    return 0; // all ok
  }
};
