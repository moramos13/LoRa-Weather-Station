/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive data with the MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/

#include <MKRWAN.h>
#include <Arduino_MKRENV.h>

// Murata radio module initialization
LoRaModem modem(Serial1);

#include "arduino_secrets.h"
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;
int fcount = 0;

void setup() {

  Serial.begin(115200);
  while (!Serial);

  // Regional band (eg. US915, AS923, ...)
  Serial.println("Initializating radio module (US915)...");
  if (!modem.begin(US915)) {
    Serial.println("Failure, try again");
    while (1) {}
  };
  //Print the device EUI
  radioModuleStarted();
 

  //OTAA Authentication
  Serial.println("Initializing OTAA activation.");
  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Failure, try again");
    while (1) {}
  }
  Serial.println("OTAA activation sucessfull");


  // Set poll interval to 60 secs.
  modem.minPollInterval(60);
  // NOTE: independently by this setting the modem will
  // not allow to send more than one message every 2 minutes,
  // this is enforced by firmware and can not be changed.

  //Initialize MKR ENV module
  Serial.println("Initializing MKR ENV SHIELD");
  if (!ENV.begin()) {
    Serial.println("Failure, try again");
    while (1);
  }
  Serial.println("MKR ENV initialized successfully");
}

void loop() {
 
  //Read all MKR ENV sensor values
  float temperature = ENV.readTemperature();
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure();
  float illuminance = ENV.readIlluminance();
  float uva         = ENV.readUVA();
  float uvb         = ENV.readUVB();
  float uvIndex     = ENV.readUVIndex(); 

  printSensorsData(temperature, humidity, pressure, illuminance, uva, uvb, uvIndex);

  //Sensor data serialization
  //float = 32 bits (4bytes)
  //Resolution HTS221 temperature and humidity = 16 bits
  //Resolution LPS22HB presion = 24 bits
  //Resolution TEMT6000 = 32 bits
  //Resolution VEML6075 = 16 bits

  byte temp[4];
  float2Bytes(temp,temperature); 
  byte humi[4];
  float2Bytes(humi,humidity); 
  byte pres[4];
  float2Bytes(pres,pressure); 
  byte illu[4];
  float2Bytes(illu,illuminance); 
  byte uax[4];
  float2Bytes(uax,uva); 
  byte ubx[4];
  float2Bytes(ubx,uvb); 
  byte uvi[4];
  float2Bytes(uvi,uvIndex); 

  //int sizeofTemp = sizeof(temp);
  //int sizeofHumi = sizeof(humi);
  //int sizeofPres = sizeof(pres);
  //int sizeofIllu = sizeof(illu);
  //int sizeofUax = sizeof(uax);
  //int sizeofUbx = sizeof(ubx);
  //int sizeofUvi = sizeof(uvi);

  int sensor = 4;

  Serial.print("Temperature Bytes: ");
  printBytes(temp);
  Serial.print("Humidity Bytes: ");
  printBytes(humi);
  Serial.print("Pressure Bytes: ");
  printBytes(pres);
  Serial.print("Illuminance Bytes: ");
  printBytes(illu);
  Serial.print("UVA Bytes: ");
  printBytes(uax);
  Serial.print("UVB Bytes: ");
  printBytes(ubx);
  Serial.print("UV Index Bytes: ");
  printBytes(uvi);
  
  byte payload[(sensor*7)];

  memcpy(payload, temp, sensor);
  memcpy(payload+(sensor*1), humi, sensor);
  memcpy(payload+(sensor*2), pres, sensor);
  memcpy(payload+(sensor*3), illu, sensor);
  memcpy(payload+(sensor*4), uax, sensor);
  memcpy(payload+(sensor*5), ubx, sensor);
  memcpy(payload+(sensor*6), uvi, sensor);

  
  Serial.println("........................");
  Serial.print("Payload Bytes: ");
  Serial.write(payload, sizeof(payload));
  Serial.println();
  
  //Message transmission
  int err;
  modem.beginPacket();
  modem.write(payload, sizeof(payload));
  err = modem.endPacket(true);
  if (err > 0) {
    Serial.println("Message transmited succesfully!");
  } else {
    Serial.println("Error sending the message");
  }

  Serial.print("Uplink message counter: ");
  Serial.println(fcount);
  fcount++;

  //Check buffer for downlink message received
  if (!modem.available()) {
    Serial.println("Not dowlink message received.");
    Serial.println();
    Serial.println("_____________________________________________________________");  
    return;
  }

  //If a message is received it is shown on the serial monitor
  char rcv[64];
  int i = 0;
  while (modem.available()) {
    rcv[i++] = (char)modem.read();
  }
  Serial.print("Received: ");
  for (unsigned int j = 0; j < i; j++) {
    Serial.print(rcv[j] >> 4, HEX);
    Serial.print(rcv[j] & 0xF, HEX);
    Serial.print(" ");
  }
  
  Serial.println();
  Serial.println("_____________________________________________________________");
  
  //We will gonna send uplink messages every minute as a test but check each country radio frecuency regulations to avoid troubles.
  delay(60000);

}

void radioModuleStarted(){
  Serial.println("-----------------------------");
  Serial.println("MODULO INICIADO CORRECTAMENTE");
  Serial.print("Versión: ");
  Serial.println(modem.version());
  Serial.print("EUI del dispositivo: ");
  Serial.println(modem.deviceEUI());
  Serial.println("-----------------------------");
}

void printSensorsData(float temperature, float humidity, float pressure, float illuminance, float uva, float uvb, float uvIndex){

  Serial.println();
  Serial.println("++++++++++++++++++++++++++++++++++++++++++");
  
  String msg = "";
  
  char tmp[6];
  sprintf(tmp, "%3.2f", temperature);
  msg += tmp;
  msg += ",";

  char hmd[5];
  sprintf(hmd, "%3.2f", humidity);
  msg += hmd;
  msg += ",";

  char prs[5];
  sprintf(prs, "%3.2f", pressure);
  msg += prs;
  msg += ",";

  char ill[8];
  sprintf(ill, "%6.2f", illuminance);
  msg += ill;
  msg += ",";

  char ua[7];
  sprintf(ua, "%4.2f", uva);
  msg += ua;
  msg += ",";

  char ub[7];
  sprintf(ub, "%4.2f", uvb);
  msg += ub;
  msg += ",";

  char ui[5];
  sprintf(ui, "%2.2f", uvIndex);
  msg += ui;

  // Imprime por el monitor serie el mensaje y su equivalente HEX
  Serial.print("Enviando: " + msg + " END");
  Serial.println();
  for (unsigned int i = 0; i < msg.length(); i++) {
    Serial.print(msg[i] >> 4, HEX);
    Serial.print(msg[i] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("++++++++++++++++++++++++++++++++++++++++++");
}

void float2Bytes(byte bytes_sensor[4],float float_variable){ 
  union {
    float a;
    unsigned char bytes[4];
  } thing;
  thing.a = float_variable;
  memcpy(bytes_sensor, thing.bytes, 4);
}

void printBytes(byte sensor[4]){
  int sizeArray = sizeof(sensor);
  Serial.write(sensor, sizeArray);
  Serial.println();
}
