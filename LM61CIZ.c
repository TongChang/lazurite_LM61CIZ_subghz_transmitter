#include "LM61CIZ_ide.h"		// Additional Header
#include <stdio.h>

#define TEMPERATURE_PIN A0
#define LED_PIN 0
#define LED_ON LOW
#define LED_OFF HIGH

#define SUBGHZ_CH		   36
#define SUBGHZ_PANID	   0x0001
#define PI_GATEWAY_ADDRESS 0x73da
#define SLEEP_INTERVAL     30000

void setup() {
	SUBGHZ_PARAM param;

	Serial.begin(115200);

	pinMode(LED_PIN, OUTPUT);

	// ACK回答を待たない
	// http://www.lapis-semi.com/lazurite-jp/lazuriteide/8654.html
	// http://www.lapis-semi.com/lazurite-jp/contents/reference/subghz_setSendMode.html
	SubGHz.init();
	SubGHz.getSendMode(&param);
//	param.addrType = 4;
	SubGHz.setSendMode(&param);
}

void loop() {
	sendData(getTemperatureFromSensor(TEMPERATURE_PIN));
    sleep(SLEEP_INTERVAL) ;
}

void brinkLed() {
	digitalWrite(LED_PIN, LED_ON);
	sleep(100);
	digitalWrite(LED_PIN, LED_OFF);
}

// http://www.geocities.jp/zattouka/GarageHouse/micon/Arduino/Temp/Temp.htm
int getTemperatureFromSensor(uint8_t pin) {
	int temperature, tv;
	int ans;

	// read from sensor
    ans = analogRead(pin);
    // to den-atsu
    tv  = map(ans,0,1023,0,3300);
    // to tempelature
    temperature = map(tv,300,1600,-30,100);

    return temperature;
}

void sendData(int temperature) {
	SUBGHZ_MSG msg;
	long myAddress;
	// XXX 固定長だと受信側で制御できないので、動的変数に直す必要がある
	char send_data[100];

	myAddress = SubGHz.getMyAddress();
	
	// preparing data
	sprintf(send_data, "{ \"temperature\" : %3d, \"address\" : \"%04x\" }", temperature, myAddress);

	// Initializing
	SubGHz.begin(SUBGHZ_CH, SUBGHZ_PANID,  SUBGHZ_100KBPS, SUBGHZ_PWR_20MW);

	Serial.println("before send.");
	// send data
	// http://www.lapis-semi.com/lazurite-jp/lazuriteide/8733.html
	while(1) {
		digitalWrite(LED_PIN, LED_OFF);
		msg = SubGHz.send(SUBGHZ_PANID, PI_GATEWAY_ADDRESS, &send_data, sizeof(send_data), NULL);
		if (msg == SUBGHZ_TX_CCA_FAIL) {
			// brink
			digitalWrite(LED_PIN, LED_ON);
			Serial.println("send error.");
			// retry
		} else {
			Serial.println("send ok.");
			break;
		}
	}
	
	SubGHz.msgOut(msg);
	Serial.println(send_data);

	// close
	SubGHz.close();

	Serial.println("end send data.");
}
