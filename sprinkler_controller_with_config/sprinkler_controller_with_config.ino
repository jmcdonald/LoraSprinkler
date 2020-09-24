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

/* sprinkler_controller_with_config.ino
 * Description: Arduino code for IOT Controller, developed for use with Adafruit Feather LORA M0 and remote nodes/zones
 */
 
// 1. default headers-
// LORA RadioHead Library
#include <RHReliableDatagram.h>
// Adafruit Feather LORA M0 uses an RF95 (915.0Mhz) radio
#include <RH_RF95.h>
// Serial interface for Radio
#include <SPI.h>
// LoraSprinkler Library Sprinkler Class
#include <Sprinkler.h>

// -------------------------------------
// 2. RADIO
// Radio: Set up
// Pins specific to Adafruit Feather LORA M0
#define RFM95_CS        8
#define RFM95_RST       4
#define RFM95_INT       7
#define SERVER_ADDRESS  99

// Set to 915.0 for US
#define RF95_FREQ 915.0

// -------------------------------------
// 3. SPRINKLER
// Sprinkler: Set up

// Manual schedule time set-up for now until RTC module is implemented
#define STARTD      4   // 0=Sun, 1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat
#define STARTH      2   // 0-23 hours
#define STARTM      0  // 0-60 minutes
#define ZONES       3   // Number of Zones to configure

// Sprinkler: Schedule setup

// zone_schedule: 
//	[ZONES]: [Sprinkler Zone]
//	[100]: [Event]
//	[2]: ["Start Time", "Duration"]
// 	[6]: Start Time: "DAYOFWEEK(0-6)HOUR(00-24)MINUTE(00-60)"
//  	 Duration:   "00000-99999" number of minutes (shouldn't practically exceed 60 for sprinklers)
char zone_schedule [ZONES][100][2][6] =
{
  { // North Hill Top - 1
    {"40201", "00002"}, // day 4 (THU) - 2:01am | 2 minutes duration
    {"40211", "00002"}, // day 4 (THU) - 2:11am | 2 minutes duration
    {"40221", "00002"}, // day 4 (THU) = 2:21am | 2 minutes duration
  },
  { // North Hill Bottom - 2
    {"40203", "00002"},
    {"40213", "00002"},
    {"40223", "00002"},
  },
  { // Northeast Hill - 3
    {"40205", "00002"},
    {"40215", "00002"},
    {"40225", "00002"},
  }
};

// Sprinkler: Messages
// tx_message struct holds the message type and data for all messages
struct {
  unsigned long int zone_time;
  uint8_t zone;
  char message_type[5];
  char message_data[100];
} tx_message;

// sprinklers[number of zones] is an array of the sprinkler objects 
// initialized with the compile time (until RTC is implemented)
Sprinkler sprinklers[ZONES] = {
  Sprinkler((char *)"North Hill Top", zone_schedule[0], STARTD, STARTH, STARTM),
  Sprinkler((char *)"North Hill Bottom", zone_schedule[1], STARTD, STARTH, STARTM),
  Sprinkler((char *)"Northeast Hill", zone_schedule[2], STARTD, STARTH, STARTM)
};

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
RHReliableDatagram manager(rf95, SERVER_ADDRESS);

void setup() {
  // reset the radio per reference code
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // Set up Serial monitor
  Serial.begin(115200);
  
  // arbitrary delay to let the Serial monitor initialize
  delay(3000);
  Serial.println("Sprinkler Controller v2");

  // initialize the RHReliableDatagram manager
  if (!manager.init()) {
    Serial.println("LoRa radio init failed");
  }
  Serial.println("LoRa radio init OK!");

  // set the frequency of the radio directly (not using manager) JMM: not sure why, should doublecheck
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // The default transmitter power is 13dBm, using PA_BOOST. JMM: confirm PA_BOOST settings
  rf95.setTxPower(13);
}

// establish buffer for message communication
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop() {
	
  // wait for nodes/zones to initialize themselves and fetch their names and offsets, or wait for node/zone to refresh after each activity
  if (manager.available()) {
    mprint(" Incoming Message");

    uint8_t bufLen = sizeof(buf);
	// from: ID of node/zone
    uint8_t from;

   if (manager.recvfromAck(buf, &bufLen, &from)) {
      memcpy(&tx_message, buf, sizeof(tx_message)); 

	  // Serial Debugging Info
      mprint("<======= Received zone = " + String(tx_message.zone));
      mprint("<======= Received time = " + String(tx_message.zone_time));
      mprint("<======= Received type = " + String(tx_message.message_type));
      mprint("<======= Received data = " + String(tx_message.message_data));
      mprint("<======= Received from = " + String(from));   
      Serial.print("got request from : 0x");
      Serial.print(from, HEX);
      Serial.println(":");
      Serial.println(sprinklers[tx_message.zone].get_name());
      Serial.println(tx_message.zone);
      
	  // Handle the message and include in the ACK
	  // if INIT, then pass the Zone name back as the ACK and send
	  if (strcmp("INIT",tx_message.message_type) == 0){
		 strcpy(buf,sprinklers[tx_message.zone].get_name());
		 if (!manager.sendtoWait(buf, bufLen, from))
			mprint("sendtoWait failed");
	  // if SCHD (schedule event) send back the number of schedule entries to expect 
	  } else if (strcmp("SCHD",tx_message.message_type) == 0){
		 // send back the number of schedule entries to expect 
		 if (!manager.sendtoWait(sprinklers[tx_message.zone].num_entries, sizeof(sprinklers[tx_message.zone].num_entries), from))
		   mprint("sendtoWait failed"); 
	     // now send each schedule entry as its own message until finished 
		 // JMM:To be implemented still, still crashing on the above ACK for some reason
	  }
    } else {
      mprint(" Message Failed");
    }
  }
  delay(300);
}


template <typename T>
void mprint( T t ){
  Serial.print(millis());
  Serial.println(t);
}
