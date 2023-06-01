/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive data with the MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/

#include <MKRWAN.h>
#include <Arduino_MKRENV.h>

// Instanciación del modulo Murata
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
  Serial.println("Iniciando modulo de radio (US915)...");
  if (!modem.begin(US915)) {
    Serial.println("Fallo al iniciar el modulo");
    while (1) {}
  };
  //Imprime los datos del dispositivo
  radioModuleStarted();
 

  // Autenticacion OTAA
  Serial.println("Iniciando activacion OTAA...");
  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Activacion OTAA fallida, intente de nuevo");
    while (1) {}
  }
  Serial.println("Activacion OTAA satisfactoria");


  // Set poll interval to 60 secs.
  modem.minPollInterval(60);
  // NOTE: independently by this setting the modem will
  // not allow to send more than one message every 2 minutes,
  // this is enforced by firmware and can not be changed.

  //Se inicializa el shield MKR ENV
  Serial.println("Iniciando MKR ENV SHIELD");
  if (!ENV.begin()) {
    Serial.println("Falla al inicializar el MKR ENV shield!");
    while (1);
  }
  Serial.println("ENV Shield iniciado correctamente");
}

void loop() {
 
  // Se leen todos los sensores
  float temperature = ENV.readTemperature();
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure();
  float illuminance = ENV.readIlluminance();
  float uva         = ENV.readUVA();
  float uvb         = ENV.readUVB();
  float uvIndex     = ENV.readUVIndex(); 

  //Se impreme por el monitor sería los datos adquiridos por los sensores y su equivalente HEX
  printSensorsData(temperature, humidity, pressure, illuminance, uva, uvb, uvIndex);

  //Serialización de los datos de los sensores
  //float = 32 bits (4bytes)
  //Resolucion HTS221 temperatura y humedad = 16 bits
  //Resolucion LPS22HB presion = 24 bits
  //Resolucion TEMT6000 = 32 bits
  //Resolucion VEML6075 = 16 bits

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
  
  //byte payload[sizeofTemp+sizeofHumi+sizeofPres+sizeofIllu+sizeofUax+sizeofUbx+sizeofUvi];
  byte payload[(sensor*7)];
  
  //int payloadSize = sizeof(payload);
  //Serial.print("Size of payload: ");
  //Serial.println(payloadSize);

  //memcpy(payload, temp, sizeofTemp);
  //memcpy(payload + sizeofTemp, humi, sizeofHumi);
  //memcpy(payload + sizeofTemp + sizeofHumi, pres, sizeofPres);
  //memcpy(payload + sizeofTemp + sizeofHumi + sizeofPres, illu, sizeofIllu);
  //memcpy(payload + sizeofTemp + sizeofHumi + sizeofPres + sizeofIllu, uax, sizeofUax);
  //memcpy(payload + sizeofTemp + sizeofHumi + sizeofPres + sizeofIllu + sizeofUax, ubx, sizeofUbx);
  //memcpy(payload + sizeofTemp + sizeofHumi + sizeofPres + sizeofIllu + sizeofUax + sizeofUbx, uvi, sizeofUvi);

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
  
  //Se transmite el mensaje el mensaje
  int err;
  modem.beginPacket();
  modem.write(payload, sizeof(payload));
  err = modem.endPacket(true);
  if (err > 0) {
    Serial.println("Mensaje enviado correctamente!");
  } else {
    Serial.println("Error al enviar el mensaje");
  }

  //Se imprimer el contador de los mensajes uplink
  Serial.print("Contador uplink: ");
  Serial.println(fcount);
  fcount++;

  //Se revisa el buffer de recepción
  if (!modem.available()) {
    Serial.println("No se ha recibido un mensaje downlink en estas ventanas de recepción.");
    Serial.println();
    Serial.println("_____________________________________________________________");
    //Se enviarán los datos cada 100 segundos
    delay(50000);
  
    return;
  }

  // Si se recibe un mensaje se lo muestra por el monitor serie
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
  //Se enviarán los datos cada 100 segundos
  delay(50000);

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
  //Serial.print("Array size: ");
  //Serial.println(sizeArray);
  //Serial.print("Array content: ");
  Serial.write(sensor, sizeArray);
  Serial.println();
}
