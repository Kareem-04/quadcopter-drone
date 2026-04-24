#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC ADDRESS
uint8_t broadcastAddress[] = {0x38, 0x18, 0x2B, 0x8B, 0xC8, 0x48};

// --- Deadzone Settings ---
// Increase these values if the sticks "drift" while centered
const int ROLL_DEADZONE  = 40; 
const int PITCH_DEADZONE = 40;
const int YAW_DEADZONE   = 100; 

typedef struct struct_message {
    int roll; int pitch; int throttle; int yaw;
    int aux1; int aux2;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// Pin Definitions
const int throttle_pin = 34;
const int roll_pin     = 33;
const int pitch_pin    = 32;
const int yaw_pin      = 35;
const int aux1_pin     = 4; // D4
const int aux2_pin     = 5; // D5

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(aux1_pin, INPUT_PULLUP);
  pinMode(aux2_pin, INPUT_PULLUP);
  
  Serial.println("Transmitter Initialized. Deadzones active on R, P, and Y.");
}

void loop() {
  // 1. Read Raw Values
  int rawThrottle = analogRead(throttle_pin);
  int rawRoll     = analogRead(roll_pin);
  int rawPitch    = analogRead(pitch_pin);
  int rawYaw      = analogRead(yaw_pin);

  // 2. Process Throttle (No deadzone usually needed)
  myData.throttle = map(rawThrottle, 0, 4095, 1000, 2000);

  // 3. Process Roll with Deadzone
  int tempRoll = map(rawRoll, 0, 4095, 1000, 2000);
  if (abs(tempRoll - 1500) < ROLL_DEADZONE) {
    myData.roll = 1500;
  } else {
    myData.roll = tempRoll;
  }

  // 4. Process Pitch with Deadzone
  int tempPitch = map(rawPitch, 0, 4095, 1000, 2000);
  if (abs(tempPitch - 1500) < PITCH_DEADZONE) {
    myData.pitch = 1500;
  } else {
    myData.pitch = tempPitch;
  }

  // 5. Process Yaw with Deadzone
  int tempYaw = map(rawYaw, 0, 4095, 1000, 2000);
  if (abs(tempYaw - 1500) < YAW_DEADZONE) {
    myData.yaw = 1500;
  } else {
    myData.yaw = tempYaw;
  }

  // 6. Read Switches
  myData.aux1 = digitalRead(aux1_pin);
  myData.aux2 = digitalRead(aux2_pin);

  // 7. Serial Print for Debugging
  Serial.print("T:");  Serial.print(myData.throttle);
  Serial.print(" | R:");  Serial.print(myData.roll);
  Serial.print(" | P:");  Serial.print(myData.pitch);
  Serial.print(" | Y:");  Serial.print(myData.yaw);
  Serial.print(" | A1:"); Serial.print(myData.aux1);
  Serial.print(" | A2:"); Serial.println(myData.aux2);

  // 8. Send via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  delay(20); 
}
