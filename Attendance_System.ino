/* ------------------------------------------------------------------------
 * Created by: Hacki Tech
 * Created on: 17 May, 2023
 * Tutorial: * ------------------------------------------------------------------------
 * Download Resources
 * HTTPS Redirect Library:
 * Example Arduino/ESP8266 code to upload data to Google Sheets
 * Follow setup instructions found here:
 * ------------------------------------------------------------------------*/
 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>
#include<Wire.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Enter Google Script Deployment ID:
const char *GScriptId = "AKfycbwU-ZwO8FXLYxIRcpwX5gMwXw509KJ7dCdcGNcTctyK9ahUR6C-Tc_z2gb4z7A9bjLb";
String gate_number = "Main Gate"; //Define Entry Gate

// Enter network credentials:
const char* ssid     = "LAB1";
const char* password = "hacker667";

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup
const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

// Declare variables that will be published to Google Sheets
String student_id;

int blocks[] = {4,5,6,8,9};
#define total_blocks  (sizeof(blocks) / sizeof(blocks[0]))
#define RST_PIN  0   //D3
#define SS_PIN   2   //D4
#define SDA      4   //D2
#define Buzzer   D0  //Buzzer Pin
#define WiFiPin  D8  //WiFiLed Pin
//------------------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;
//------------------------------------------------------------
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  
/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];

void setup() {
  //----------------------------------------------------------
  Serial.begin(9600);          
  delay(10);
  Serial.println('\n');
  pinMode(Buzzer, OUTPUT);  
  pinMode(WiFiPin, OUTPUT);  
  //----------------------------------------------------------
  SPI.begin();
  //----------------------------------------------------------
  //initialize lcd screen
  lcd.init();
  // turn on the backlight
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0,1); //col=0 row=1
  lcd.print("WiFi...");

  // Connect to WiFi
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("WiFi Connected!");

  //Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Device IP:-");
  lcd.setCursor(0,1); //col=0 row=1
  lcd.print(WiFi.localIP());
  delay(2000);
  //----------------------------------------------------------
  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  //----------------------------------------------------------
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("Google ");
  delay(5000);
  //----------------------------------------------------------
  Serial.print("Connecting to ");
  Serial.println(host);
  //----------------------------------------------------------
  // Try to connect for a maximum of 5 times
  bool flag = false;
  for(int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    //*************************************************
    if (retval == 1){
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      digitalWrite(WiFiPin, HIGH);
      digitalWrite(Buzzer, HIGH);
      delay(300);
      digitalWrite(Buzzer, LOW);
      lcd.clear();
      lcd.setCursor(0,0); //col=0 row=0
      lcd.print(msg);
      delay(2000);
      break;
    }
    //*************************************************
    else
      Serial.println("Connection failed. Retrying...");
    //*************************************************
  }
  //----------------------------------------------------------
  if (!flag){
    //____________________________________________
    lcd.clear();
    lcd.setCursor(0,0); //col=0 row=0
    lcd.print("Connection fail");
    //____________________________________________
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    return;
    //____________________________________________
  }
  //----------------------------------------------------------
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
  //----------------------------------------------------------
}

void loop() {
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    //when below if condition is TRUE then it takes more time then usual, It means the device 
    //is disconnected from the google sheet server and it takes time to connect again
    if (!client->connected()){
      int retval = client->connect(host, httpsPort);
      if (retval != 1){
        digitalWrite(WiFiPin, LOW);
        Serial.println("Disconnected. Retrying...");
        lcd.clear();
        lcd.setCursor(0,0); //col=0 row=0
        lcd.print("Disconnected.");
        lcd.setCursor(0,1); //col=0 row=0
        lcd.print("Retrying...");
        return; //Reset the loop
      }
    }
  }
  else{Serial.println("Error creating client object!"); Serial.println("else");}
  //----------------------------------------------------------------
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("  Scan your Tag");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("Owner-Rahul Dudi");
  digitalWrite(WiFiPin, HIGH);
  //Serial.println("[TEST] Scan Your Tag");
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}
  digitalWrite(Buzzer, HIGH);
  delay(200);
  digitalWrite(Buzzer, LOW);
  delay(100);
  digitalWrite(Buzzer, HIGH);
  delay(200);
  digitalWrite(Buzzer, LOW);
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;}
  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));  
  //----------------------------------------------------------------
  String values = "", data;
  for (byte i = 0; i < total_blocks; i++) {
    ReadDataFromBlock(blocks[i], readBlockData); //edited i to i+1
    if(i == 0){
      data = String((char*)readBlockData);
      data.trim();
      student_id = data;
      values = "\"" +  data + ",";
    }
    else{
      data = String((char*)readBlockData);
      data.trim();
      values += data + ",";
    }
  }
  values += gate_number + "\"}";
  payload = payload_base + values;
  //----------------------------------------------------------------
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Publishing Data");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("Please Wait...");
  //----------------------------------------------------------------
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
    Serial.println("[OK] Data published.");
    lcd.clear();
    lcd.setCursor(0,1); //col=0 row=1
    lcd.print("Student ID:"+student_id);
    lcd.setCursor(0,0); //col=0 row=0
    lcd.print("     Welcome");  
  }
  //----------------------------------------------------------------
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
    lcd.clear();
    lcd.setCursor(0,0); //col=0 row=0
    lcd.print("Failed.");
    lcd.setCursor(0,1); //col=0 row=1
    lcd.print("Try Again");
  }
  //----------------------------------------------------------------
  // a delay of several seconds is required before publishing again    
  Serial.println("[TEST] delay(3000)");
  delay(3000);
  // digitalWrite(WiFiPin, LOW);
  }

void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 

  /* Prepare the key for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //----------------------------------------------------------------------------
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  //----------------------------------------------------------------------------s
  if (status != MFRC522::STATUS_OK){
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
  //----------------------------------------------------------------------------
  else {
    Serial.println("Authentication success");
  }
  //----------------------------------------------------------------------------
  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //----------------------------------------------------------------------------
  else {
    readBlockData[16] = ' ';
    readBlockData[17] = ' ';
    Serial.println("Block was read successfully");  
  }
  //----------------------------------------------------------------------------
}
