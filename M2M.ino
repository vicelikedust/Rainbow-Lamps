/*  Project : Cristamas Lights
 *  Author  : Mukesh Sankhla
 *  IG      : @mukesh.diy
 *  Twitter : @MukeshSankhla0
 */

#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>


#define touchPin 4                //Touch Sensor Pin
#define pixelPin 2                //NeoPixel pin
#define noOfPixels 16             //No.Of LED's
Adafruit_NeoPixel strip(noOfPixels, pixelPin, NEO_GRB + NEO_KHZ800);

//Change These Values
int deviceNo = 1;                 //Device Number
bool ClearColor = true;           //Do you want to clear the broadcast color?
bool Clear = false;
bool touchState = false;

//Set up non-blocking timer
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

//Universal Mac Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

//Message(Data) send and receive doc
typedef struct messageData
{
  int color = 0;
} messageData;

messageData deviceData;     //Local Data
messageData recData;        //Received Data

// Create peer interface
esp_now_peer_info_t peerInfo;

//Called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//Called when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&recData, incomingData, sizeof(recData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.println(recData.color);
  if(ClearColor){
    Clear = true;
  }
}

//Declare the send result
esp_err_t result;

void setup() {
  Serial.begin(115200);
  strip.begin(); 
  strip.show();
  pinMode(touchPin, INPUT_PULLDOWN);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  recData.color = deviceNo;
}

void loop()
{
  if (digitalRead(touchPin)) {
    if(!touchState){
      touchState = true;
      if(ClearColor){
        Clear = true;
      }
      deviceData.color = recData.color = deviceNo;
      result = esp_now_send(broadcastAddress, (uint8_t *) &deviceData, sizeof(deviceData));
    }
  }
  else{
    touchState = false;
  }
    switch(recData.color) {
        case 1:
          colorWipe(strip.Color(255, 0, 0), 10);    // Red
          break;
        case 2:
          colorWipe(strip.Color(255, 255, 0), 10);    // Yellow
          break;
        case 3:
          colorWipe(strip.Color(0, 255, 0), 10);    // Green
          break;
        case 4:
          colorWipe(strip.Color(255, 0, 255), 10);    // Pink
          break;
        case 5:
          colorWipe(strip.Color(0, 0, 255), 10);    // Blue
          break;
    }
    if(Clear){
     switch(recData.color) {
        case 1:
          colorWipe(strip.Color(255, 0, 0), 10);    // Red
          break;
        case 2:
          colorWipe(strip.Color(255, 255, 0), 10);    // Yellow
          break;
        case 3:
          colorWipe(strip.Color(0, 255, 0), 10);    // Green
          break;
        case 4:
          colorWipe(strip.Color(255, 0, 255), 10);    // Pink
          break;
        case 5:
          colorWipe(strip.Color(0, 0, 255), 10);    // Blue
          break;
    }
    nonBlockingDelay(2000);
    recData.color = deviceNo;
    Clear = false;
    }
}

//no need to call a function to get the pixel count, just access it directly
void colorWipe(uint32_t color, int wait) {
  for(int i = 0; i < noOfPixels; i++) { 
    strip.setPixelColor(i, color);         
    strip.show();
    nonBlockingDelay(wait);
  }
}

//using a non-blocking delay will prevent any issues with the WiFi
//Delay() can prevent the esp32 from sending or receiving data
void nonBlockingDelay(int interval){
  while(currentMillis - previousMillis <= interval){
      //check time passed
      currentMillis = millis();
  }
}
