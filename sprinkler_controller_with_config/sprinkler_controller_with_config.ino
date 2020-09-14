// 1. default headers

// LORA RadioHead Library
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
// Serial interface for Radio
#include <SPI.h>
// My Sprinkler Class
#include <Sprinkler.h>

// -------------------------------------
// 2. RADIO
// Radio: Set up

#define RFM95_CS        8
#define RFM95_RST       4
#define RFM95_INT       7
#define SERVER_ADDRESS  99

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// -------------------------------------
// 3. SPRINKLER
// Sprinkler: Set up

// Manual schedule time set-up for now until RTC
#define STARTD      4   // 0=Sun, 1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat
#define STARTH      2   // 0-23 hours
#define STARTM      0  // 0-60 minutes
#define ZONES       3   // Number of Zones to configure

// Sprinkler: Schedule setup
char zone_schedule [ZONES][100][2][6] =
{
  { // North Hill Top - 1
    {"40201", "00002"},
    {"40211", "00002"},
    {"40221", "00002"},
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
struct {
  unsigned long int zone_time;
  uint8_t zone;
  char message_type[5];
  char message_data[100];
} tx_message;

Sprinkler sprinklers[ZONES] = {
  Sprinkler((char *)"North Hill Top", zone_schedule[0], STARTD, STARTH, STARTM),
  Sprinkler((char *)"North Hill Bottom", zone_schedule[1], STARTD, STARTH, STARTM),
  Sprinkler((char *)"Northeast Hill", zone_schedule[2], STARTD, STARTH, STARTM)
};

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
RHReliableDatagram manager(rf95, SERVER_ADDRESS);

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  delay(3000);
  Serial.println("Sprinkler Controller v2");

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

  // initia
}

// Wait for Sprinklers to check in
//char * rx_conf;
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
void loop() {
  if (manager.available()) {
    mprint(" Incoming Message");
    // A message came in

    uint8_t bufLen = sizeof(buf);
    Serial.println(bufLen);
    // Serial.println(sizeof(tx_message));
    //Serial.println(sizeof(test_message));
    uint8_t from;

   // if (manager.recvfromAck((uint8_t *) &tx_message, &bufLen, &from)) {
   if (manager.recvfromAck(buf, &bufLen, &from)) {
      memcpy(&tx_message, buf, sizeof(tx_message)); 

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
      strcpy(buf,sprinklers[tx_message.zone].get_name());
      if (!manager.sendtoWait(buf, bufLen, from))
        mprint("sendtoWait failed");
    } else {
     mprint(" Message Failed");
    }

  }
  delay(300);
}
// ---------------------------------------
// 4. When sprinkler checks in
//   get ID
//   get and record time
//   calculate schedule and send

template <typename T>
void mprint( T t ){
  Serial.print(millis());
  Serial.println(t);
}
