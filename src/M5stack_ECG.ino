
/*********************************************************************************************************
 *  NOTE:
 *  If you can't compile this code because of flash memory shortage, please change partition scheme
 *  "Tools" tab > "partition scheme" > "Minimal SPIFFS"
 * *******************************************************************************************************/

#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define HORIZONTAL_RESOLUTION 320
#define VERTICAL_RESOLUTION   240
#define POSITION_OFFSET_Y      20
#define SIGNAL_LENGTH HORIZONTAL_RESOLUTION
#define ANALOG_SIGNAL_INPUT  36 //Input pin

uint16_t OldSignal[SIGNAL_LENGTH];
uint16_t AdcBuffer[SIGNAL_LENGTH];
int Old_x, Old_y;
int Old_Sig;
uint32_t Sampling_period_us;
uint32_t Notch_freq;
const uint8_t N = 10;
File file;
uint8_t Flag_save;
const char* Str_onoff[2] = {"off", "on"};

const char url[] = "https://script.google.com/macros/s/*********/exec";
const char ssid[] = "XXXXXXXXXX";
const char pass[] = "YYYYYYYYYY";
const char fname[] = "/m5stack_ecg.csv"; // need "/"
//const char fname[] = "/m5stack_ecg.bin";
//String fname = "/m5stack_ecg.csv";


void setup() {
  Notch_freq = 50;
  Sampling_period_us = 1000000/(N*Notch_freq);
  Flag_save = 0;
  M5.begin();
  M5.Power.begin();
  drawInitScreen();
}

void drawInitScreen(void){
  M5.Lcd.fillScreen( BLACK );
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("ECG_checker");
  M5.Lcd.printf("Notch: %d", Notch_freq);
  M5.Lcd.printf(" Hz, Save: %s\n", Str_onoff[Flag_save]);
}

void loop(){
  int n, m;
  uint32_t nextTime = 0;
  uint16_t adcSum =0;
  for (n = 0; n < SIGNAL_LENGTH; n++)
  {
    adcSum = 0;
    for (m = 0; m < N; m++){ //N-ma_filter (N=10)
      adcSum = adcSum + analogRead(ANALOG_SIGNAL_INPUT);
      while (micros() < nextTime); // wait for next sample
      nextTime = micros() + Sampling_period_us;
    }     
    AdcBuffer[n] = adcSum/N; // average of latest N smpales(N=10)
    drawSignal(n);

    if(Flag_save){
      //1. binary data
      //byte buf[2] = {byte(AdcBuffer[n]>>8), byte(AdcBuffer[n])};
      //file.write(buf, 2);      
      //2. text data
      file.print(AdcBuffer[n]);
      file.print("\n");
    }
 
    M5.update();
    if (M5.BtnA.wasPressed()) { // Switch Notch filter 50Hz/60Hz
      pushBtnA();
      break;
    }
    else if(M5.BtnB.wasPressed()){ // Saving mode on/off
      pushBtnB();  
      break;
    }
    else if(M5.BtnC.wasPressed()){ // Turn off power
      pushBtnC();
    }
  }
}

void drawSignal(int n){
  int x, y;
  x = n;
  y = map(AdcBuffer[n], 0, 4096, VERTICAL_RESOLUTION, POSITION_OFFSET_Y); // M5stack's resolution: 12bits(4096)
  if (n > 0){
    //M5.Lcd.drawLine(Old_x, OldSignal[n-1], x, OldSignal[n], BLACK);  // delete old line element
    M5.Lcd.drawLine(Old_x, Old_Sig, x, OldSignal[n], BLACK);  // delete old line element
    // draw new line element
    if (n < SIGNAL_LENGTH-1){  //do not draw last element because it would generate artifacts
      M5.Lcd.drawLine(Old_x, Old_y, x, y, GREEN);
    }
  }
  Old_x = x;
  Old_y = y;
  Old_Sig = OldSignal[n];
  OldSignal[n] = y;
}

void pushBtnA(void){
  if (Notch_freq==50) Notch_freq=60;
  else Notch_freq=50;
  drawInitScreen();
  Sampling_period_us = 1000000/(N*Notch_freq);
}

void pushBtnB(){
  //Saveing mode: ON
  if (Flag_save){
    file.close();
    delay(100);
    file = SD.open(fname, FILE_READ);
    if(file){
      int i=0;
      char* buf;
      buf = (char*)malloc(file.size());
      M5.Lcd.printf("File size is: %dbytes\n", file.size());
      while(file.available()){
        buf[i] = file.read();
        i++;
      }
      if(connectWifi()){
        upload(buf);
      }
      free(buf);
      WiFi.disconnect(true, true);
      file.close();
      delay(5000);
    } else {
      M5.Lcd.println("Failed to re-open a file!");
      delay(5000);
    }
    Flag_save=0;
    drawInitScreen();
  }
  // Saving mode: OFF
  else{
    file = SD.open(fname, FILE_WRITE); //file = SD.open(fname.c_str(), FILE_WRITE);
    if(file){
      Flag_save=1;
      for(uint8_t i=5; i>0; i--){
        M5.Lcd.printf("Recording starts in .... %d\n", i);
        delay(1000);
      }
      drawInitScreen();
    }
    else{
      drawInitScreen();
      M5.Lcd.println("Failed to open a file!");
    }
  }
}

void pushBtnC(){
  if(file) file.close();
  M5.Power.powerOFF();
  while(1);
}

bool connectWifi(void){
  WiFi.disconnect(); // just in case
  delay(100);
  uint8_t timeout = 5;
  M5.Lcd.println("Connecting to AP ...");
  for(uint8_t i=0; i<timeout; i++){
    WiFi.begin(ssid, pass);
    if(WiFi.waitForConnectResult()!=WL_CONNECTED) delay(300); // "WiFi.status()!=WL_CONNECTED" was NOT stable
    else {
      M5.Lcd.println("Successed to connect!");
      return true;
    }
  }
  M5.Lcd.println("Failed to connect!");
  M5.Lcd.printf("WiFi Status: %d\n", WiFi.status());
  return false;
}

bool upload(char* buff){
  M5.Lcd.println("Uploading ...");
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/octet-stream"); // binary file
  //http.addHeader("Content-Type", "test/csv"); //csv file
  int httpCode = http.POST(buff);
  if(httpCode==200) M5.Lcd.println("Successed to upload!");
  else {
    M5.Lcd.println("Failed to upload!");
    M5.Lcd.printf("HTTPstatus code: %d\n", httpCode);
  }
  delay(5000);
  return true;
}
