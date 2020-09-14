/* LoraSprinkler Firmware
 * Copyright (C) 2020 by Jeff McDonald (netadept@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
 
#include <SPI.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

// LoraSprinkler Library
#include <Sprinkler.h>

// -------------------------------------
// 2. RADIO
// Radio: Config

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define SERVER_ADDRESS  99
#define CLIENT_ADDRESS  0

#define NEW 0
#define SCHD 1
#define WAITING 2
#define ENGAGED 3

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// 3. NODE
// Node: Config

typedef struct{
  unsigned long int zone_time;
  uint8_t zone;
  char message_type[5];
  char message_data[100];
  uint8_t status;
  uint16_t error_count;
} MESSAGE;

MESSAGE init_message;
MESSAGE data_message;

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

uint8_t status = NEW;
char node_name[64];

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
RHReliableDatagram manager(rf95, CLIENT_ADDRESS);

// Node: Set up
void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  delay(1000);

  Serial.println("Sprinkler Node");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!manager.init()) {
    Serial.println("LoRa radio init failed");
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(13);

}
byte tx_buf[sizeof(init_message)] = {0};

void loop() {
  // CHECK INITIALIZATION
  if (status == NEW){
    initialize();
  } else if (status == SCHD){
	get_schedule();
  } else {
    Serial.println(node_name);
  }
  delay(3000);
}

void initialize(){
  mprint("Initializing with Controller");
  init_message.zone = CLIENT_ADDRESS;
  strcpy(init_message.message_type, "INIT");
  char init_data[19] = "Hello from zone 0 ";
  itoa(CLIENT_ADDRESS, init_data+16,10);
  strcpy(init_message.message_data, init_data); 
  // Send the initialization message 
  init_message.zone_time = millis();
  Serial.println("<======= Sending zone = " + String(init_message.zone));
  Serial.println("<======= Sending time = " + String(init_message.zone_time));
  Serial.println("<======= Sending type = " + String(init_message.message_type));
  Serial.println("<======= Sending data = " + String(init_message.message_data));
  Serial.println("<======= Message Size = " + String(sizeof(init_message)));
  memcpy(tx_buf, &init_message, sizeof(init_message) );
  
  if (manager.sendtoWait((uint8_t *)tx_buf, sizeof(init_message), SERVER_ADDRESS)) {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAckTimeout(buf, &len, 3000, &from)){
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);     
      status = SC; 
      strncpy(node_name,(char*)buf, sizeof(node_name));
    } else {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  } else {
    mprint("Initialization Failed");
  }
}

void get_schedule(){
//  // build the message
  data_message.zone = CLIENT_ADDRESS;
  data_message.status = status;
  data_message.zone_time = millis();
  strcpy(data_message.message_type, "SCHD");
  
  data_message.zone_time = millis();
  Serial.println("SCHEDULE REQUEST");
  Serial.println("<======= Sending zone = " + String(data_message.zone));
  Serial.println("<======= Sending time = " + String(data_message.zone_time));
  Serial.println("<======= Sending type = " + String(data_message.message_type));
  Serial.println("<======= Sending status = " + String(data_message.status));
  Serial.println("<======= Message Size = " + String(sizeof(data_message))); 
  memcpy(tx_buf, &data_message, sizeof(data_message) );  
  if (manager.sendtoWait((uint8_t *)tx_buf, sizeof(data_message), SERVER_ADDRESS)) {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAckTimeout(buf, &len, 3000, &from)){
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);     
      status = WAITING; 
      strncpy(node_name,(char*)buf, sizeof(node_name));
    } else {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  } else {
    mprint("Schedule Request Failed");
  }
/* typedef struct{
  unsigned long int zone_time;
  uint8_t zone;
  char message_type[5];
  char message_data[100];
  uint8_t status;
  uint16_t error_count;
} MESSAGE; 
*/
}
template <typename T>
void mprint( T t ) {
  Serial.print(millis());
  Serial.print(" ");
  Serial.println(t);
}
