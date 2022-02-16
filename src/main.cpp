/*
	==== CONDUCTOR ESP32 ====
*/

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

uint8_t broadcastAddressConductor[]		= {0x02, 0x50, 0xC5, 0xAC, 0xC9, 0xE5};
uint8_t broadcastAddressWaterBrain[]	= {0x12, 0xB4, 0x92, 0x12, 0x27, 0xFA};
// uint8_t broadcastAddressWaterFood[]		= {0x22, 0x61, 0x7C, 0xBD, 0x2D, 0xFD};
// uint8_t broadcastAddressRelay[]			= {0x32, 0x04, 0x6C, 0x26, 0xE3, 0x47};
// uint8_t broadcastAddressSecurity[]		= {0x42, 0x6D, 0xB0, 0x1C, 0xC0, 0xD9};

// Sensors readings
int w_temp;
int level;
int ph;
int ec;
// float w_temp;
// float level;
// float ph;
// float ec;
typedef struct struct_message {
		int w_temp;
		int level;
		int ph;
		int ec;
		// float w_temp;
		// float level;
		// float ph;
		// float ec;
} struct_message;
struct_message incomingMessage;

// Orders
int orderMeasures;
int orderSetup;
typedef struct struct_order {
		int measures;
		int setup;
} struct_order;
struct_order sendingOrders;

String success;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
void sendOrders();

void setup() {
	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	// Change MAC
	esp_wifi_set_mac(WIFI_IF_STA, &broadcastAddressConductor[0]);
	if (esp_now_init() != ESP_OK) {
		Serial.println("[CDT] Error initializing ESP-NOW");
		return;
	}
	// Register Send CB to get the status of Transmitted packet
	esp_now_register_send_cb(OnDataSent);
	// Register peer WaterBrain
	esp_now_peer_info_t peerInfo;
	memcpy(peerInfo.peer_addr, broadcastAddressWaterBrain, 6);
	peerInfo.channel = 0;	
	peerInfo.encrypt = false;
	// Add peer				
	if (esp_now_add_peer(&peerInfo) != ESP_OK){
		Serial.println("[CDT] Failed to add peer");
		return;
	}
	// Register for a callback function to be called when data is received
	esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
	
	sendOrders();

	delay(10000);
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
	Serial.print("\r\n[CDT] Last Packet Send Status: ");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? 
		"Delivery Success" : "Delivery Fail");
	if (status == 0){
		success = "\r\n[CDT] [SND] Delivery Success :)";
	}
	else{
		success = "\r\n[CDT] [SND] Delivery Fail :(";
	}
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
	memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
	Serial.print("\r\n[CDT] [RCV] Bytes received: ");
	Serial.println(len);
	w_temp	= incomingMessage.w_temp;
	level	= incomingMessage.level;
	ph		= incomingMessage.ph;
	ec		= incomingMessage.ec;
	Serial.printf("\r\n[CDT] [RCV] Water temperature: %i", w_temp);
	Serial.printf("\r\n[CDT] [RCV] Level: %i", level);
	Serial.printf("\r\n[CDT] [RCV] pH: %i", ph);
	Serial.printf("\r\n[CDT] [RCV] EC: %i", ec);
}

void sendOrders() {
	
	orderMeasures	= random(0, 2);
	orderSetup		= random(0, 2);

	// Set values to send
	sendingOrders.measures = orderMeasures;
	sendingOrders.setup = orderSetup;
	Serial.printf("\r\n===========================");
	Serial.printf("\r\n[CDT] [SND] Order Measures: %i", sendingOrders.measures);
	Serial.printf("\r\n[CDT] [SND] Order Setup: %i", sendingOrders.setup);

	// Send message via ESP-NOW
	esp_err_t result = esp_now_send(broadcastAddressWaterBrain, (uint8_t *) 
		&sendingOrders, sizeof(sendingOrders));
	if (result == ESP_OK) {
		Serial.println("\r\n[CDT] [SND] Sent with success");
	}
	else {
		Serial.println("\r\n[CDT] [SND] Error sending the data");
	}
}