/*
	==== CONDUCTOR ESP32 ====
*/

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

uint8_t broadcastAddressConductor[]		= {0x02, 0x50, 0xC5, 0xAC, 0xC9, 0xE5}; // MAC Board 4 / ESP32
uint8_t broadcastAddressWaterBrain[]	= {0x12, 0xB4, 0x92, 0x12, 0x27, 0xFA}; // MAC Board 1 / ESP8266

// Sensors readings
int temperature;
int level;
int ph;
int ec;
// float temperature;
// float level;
// float ph;
// float ec;
typedef struct struct_message {
		int temperature;
		int level;
		int ph;
		int ec;
		// float temperature;
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
	// Init Serial Monitor
	Serial.begin(115200);
 
	// Set device as a Wi-Fi Station
	WiFi.mode(WIFI_STA);

	esp_wifi_set_mac(WIFI_IF_STA, &broadcastAddressConductor[0]);
	Serial.print("[NEW] ESP32 Board MAC Address:	");
	Serial.println(WiFi.macAddress());

	// Init ESP-NOW
	if (esp_now_init() != ESP_OK) {
		Serial.println("Error initializing ESP-NOW");
		return;
	}

	// Once ESPNow is successfully Init, we will register for Send CB to
	// get the status of Trasnmitted packet
	esp_now_register_send_cb(OnDataSent);
	
	// Register peer
	esp_now_peer_info_t peerInfo;
	memcpy(peerInfo.peer_addr, broadcastAddressWaterBrain, 6);
	peerInfo.channel = 0;	
	peerInfo.encrypt = false;
	
	// Add peer				
	if (esp_now_add_peer(&peerInfo) != ESP_OK){
		Serial.println("Failed to add peer");
		return;
	}
	// Register for a callback function that will be called when data is received
	esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
	sendOrders();
 
	// Set values to send
	sendingOrders.measures		= orderMeasures;
	sendingOrders.setup			= orderSetup;
	Serial.printf("[SND-CDT] Measures: %i\n", sendingOrders.measures);
	Serial.printf("[SND-CDT] Setup: %i\n", sendingOrders.setup);

	// Send message via ESP-NOW
	esp_err_t result = esp_now_send(broadcastAddressWaterBrain, (uint8_t *) &sendingOrders, sizeof(sendingOrders));
	if (result == ESP_OK) {
		Serial.println("Sent with success");
	}
	else {
		Serial.println("Error sending the data");
	}
	delay(10000);
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
	Serial.print("\r\nLast Packet Send Status:\t");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
	if (status ==0){
		success = "Delivery Success :)";
	}
	else{
		success = "Delivery Fail :(";
	}
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
	memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
	Serial.print("Bytes received: ");
	Serial.println(len);
	temperature	= incomingMessage.temperature;
	level		= incomingMessage.level;
	ph			= incomingMessage.ph;
	ec			= incomingMessage.ec;
	Serial.printf("[RCV-CDT] Temperature: %i\n", temperature);
	Serial.printf("[RCV-CDT] Level: %i\n", level);
	Serial.printf("[RCV-CDT] pH: %i\n", ph);
	Serial.printf("[RCV-CDT] EC: %i\n", ec);
}

void sendOrders() {
	orderMeasures	= random(0, 2);
	orderSetup		= random(0, 2);
	Serial.printf("orderMeasures: %i\n", orderMeasures);
	Serial.printf("orderSetup: %i\n", orderSetup);
}