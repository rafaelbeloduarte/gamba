// file name to use for writing
const char filename[] = "dados.csv";
float Tr = 0;
float Tf = 0;
float Ta = 0;
int gas_sensorValue = 0;
float CO2_molar_flow = 0;
float CH4_molar_flow = 0;
int n_flow_readings = 100; // to take average flow readings from n_flow_readings
// MFC calibrated from 0 to 1000 cm³/min at 70 oF and 14.8 psi
// outputs a 0 to 5V signal linear to this scale 


// Data Logger shield

#include<Wire.h>
#include "RTClib.h" // Must download the library from manage library &gt; type RTCLib by Adafruit
#include<SPI.h>
#include<SD.h>
#include <LiquidCrystal.h>

RTC_DS1307 rtc;

int chipSelect = 10;

// A simple data logger for the Arduino analog pins
#define WAIT_TO_START    0 // Wait for serial input in setup()

//LCD pin to Arduino
const int pin_RS = 8;
const int pin_EN = 9;
const int pin_d4 = 4;
const int pin_d5 = 5;
const int pin_d6 = 6;
const int pin_d7 = 7;

// sensor pins
const int pin_Ta = 1;
const int pin_Tr = 2;
const int pin_Tf = 3;

File mySensorData;
unsigned long startMillisSD;
unsigned long currentMillisSD;
const unsigned long periodSD = 5000;
unsigned long startMillisLCD;
unsigned long currentMillisLCD;
const unsigned long periodLCD = 2000;
float t_min = 0;

LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

double readCelsius(uint8_t cs) {
  uint16_t v;

  digitalWrite(cs, LOW);
  v = SPI.transfer(0x00);
  v <<= 8;
  v |= SPI.transfer(0x00);
  digitalWrite(cs, HIGH);

  if (v & 0x4) {
    // uh oh, no thermocouple attached!
    return NAN;
  }

  v >>= 3;

  return v * 0.25;
}

void printMenu(int option) {
  if (option == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Menu");
    delay(500);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("<Voltar");
    lcd.setCursor(8, 0);
    lcd.print(">Suporte");
    lcd.setCursor(0, 1);
    lcd.print("vEnvDado");
    lcd.setCursor(8, 1);
    lcd.print("^DelDado");
  }
  else if (option == 11) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("duarterafaelbelo");
    lcd.setCursor(0, 1);
    lcd.print("@gmail.com");
  }
}

void dumpToSerial() {
  Serial.begin(230400);
  Serial.println();
  Serial.println();
  Serial.print("Enviando dados...");
  Serial.println();
  Serial.println();

  lcd.clear();
  lcd.print("Enviando...");

  delay(5000);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(filename);

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
      //delay(1);
    }
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("erro ao abrir");
  }
  Serial.println();
  Serial.println();
  Serial.end();
}

void deleteData() {
  lcd.clear();
  lcd.print("Tem certeza?");
  lcd.setCursor(0, 1);
  lcd.print("< Nao  > Sim");
  delay(500);
  while (1) {
    int x = analogRead (0);
    delay(1);
    if (x < 60) {
      lcd.clear();
      lcd.print("Pressione RST");
      lcd.setCursor(0, 1);
      lcd.print("p/ cancelar");
      delay(2000);
      lcd.clear();
      lcd.print("Apagando em...");
      lcd.setCursor(0, 1);
      for (int i = 5; i >= 0; i--) {
        lcd.print(i);
        lcd.print(" ");
        delay(1000);
      }
      SD.remove(filename);
      lcd.clear();
      lcd.print(filename);
      lcd.setCursor(0, 1);
      lcd.print("apagado");
      delay(3000);
      lcd.clear();
      break;
    }
    else if (x < 600) {
      delay(100);
      break;
    }
  }
  
}

void setup()
{
  // chip select pins for sensors, must be high, otherwise serial bus will be busy
  pinMode(pin_Tr, OUTPUT);
  digitalWrite(pin_Tr, HIGH);
  pinMode(pin_Tf, OUTPUT);
  digitalWrite(pin_Tf, HIGH);
  pinMode(pin_Ta, OUTPUT);
  digitalWrite(pin_Ta, HIGH);
  // Serial.begin(115200);
  // Serial.println("Sistema de coleta de dados Gambá");

  // 2 - Data Logger shield

  pinMode(chipSelect, OUTPUT);
  //  RTC.begin();
  Wire.begin();
  SD.begin(chipSelect);

  // for LCD
  lcd.begin(16, 2);

  //lcd.setCursor(3, 0);
  //lcd.print("Sistema de");
  //lcd.setCursor(0, 1);
  //lcd.print("coleta de dados");
  delay(1000);

  // try to open the file for writing
  mySensorData = SD.open(filename, FILE_WRITE);
  if (!mySensorData) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("erro ao abrir");
    delay(5);
    // Serial.print("error opening ");
    // Serial.println(filename);
    lcd.setCursor(0, 1);
    lcd.print(filename);
    while (1);
  }
  else {
    // Serial.print(filename);
    // Serial.println(" aberto");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(filename);
    lcd.setCursor(0, 1);
    lcd.print("aberto");
    delay(2000);
    lcd.clear();
  }

  // add some new lines to start
  mySensorData.println("date,time,millis,t_min,Tr,Tf,Ta,CO2_µmol_per_s,CH4_µmol_per_s,gas");
  mySensorData.close();

  startMillisSD = millis();

  if (! rtc.begin()) {
    lcd.clear();
    lcd.print("RTC not found");
    while (1) delay(100);
  }

  if (! rtc.isrunning()) {
    lcd.clear();
    lcd.print("RTC NOT running");
    lcd.setCursor(0,1);
    lcd.print("setting time");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

#if WAIT_TO_START
  lcd.setCursor(0, 0);
  lcd.print("Aperte qualquer");
  lcd.setCursor(0, 1);
  lcd.print("tecla p/ iniciar");
  while (analogRead(0) > 800);
#endif //WAIT_TO_START

  

}

void loop()

{
  delay(10);

  // to detect key pressed
  int x;
  x = analogRead (0);
  if (x < 800) {
    printMenu(1);
    while (1) {
      x = analogRead (0);
      if (x < 60) {
        printMenu(11);
      }
      else if (x < 200) {
        deleteData();
        break;
      }
      else if (x < 400) {
        mySensorData.close();
        delay(100);
        dumpToSerial();
        delay(1000);
        lcd.clear();
        lcd.print("Dados enviados");
        lcd.setCursor(0, 1);
        lcd.print("p/ porta serial");
        delay(5000);
        lcd.clear();
        
        break;
      }
      else if (x < 600) {
        
        delay(100);
        lcd.clear();
        break;
      }
    }
  }
  //  if (x < 60) {
  //    lcd.print ("Right ");
  //  }
  //  else if (x < 200) {
  //    lcd.print ("Up    ");
  //  }
  //  else if (x < 400){
  //    lcd.print ("Down  ");
  //  }
  //  else if (x < 600){
  //    lcd.print ("Left  ");
  //  }
  //  else if (x < 800){
  //    lcd.print ("Select");
  //  }


  // Data Logger shield

  currentMillisLCD = millis();


  if (currentMillisLCD - startMillisLCD >= periodLCD)
  { 
    // read sensors
    t_min = float(millis()) / (60000);
    //lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("    ");
    lcd.setCursor(0,0);
    lcd.print(t_min, 0);
    // Serial.println(t_min);
    // Serial.print("millis = ");
    // Serial.print(currentMillisSD);
    // Serial.print("   ");
    delay(5);        // delay in between reads for stability

    
    DateTime now = rtc.now();
    lcd.setCursor(0,1);
    lcd.print("    ");
    lcd.setCursor(0,1);
    lcd.print(now.hour(),DEC);
    if (now.minute() < 10){
      lcd.print("0");
      lcd.print(now.minute(),DEC);
      }
    else {
      lcd.print(now.minute(),DEC);
      }

    CO2_molar_flow = 0;
    CH4_molar_flow = 0;
    for (int i = 0; i<n_flow_readings; i++){
      CO2_molar_flow = analogRead(A2) + CO2_molar_flow;
      delay(1);
      CH4_molar_flow = analogRead(A3) + CH4_molar_flow;
      delay(1);
      }
    CO2_molar_flow = 0.64338*CO2_molar_flow/n_flow_readings; // µmol/s
    CH4_molar_flow = 0.64338*CH4_molar_flow/n_flow_readings; // µmol/s
    lcd.setCursor(13,1);
    lcd.print("   ");
    lcd.setCursor(13,1);
    lcd.print(round(CO2_molar_flow));
    lcd.setCursor(13,0);
    lcd.print("    ");
    lcd.setCursor(13,0);
    lcd.print(round(CH4_molar_flow));

    Ta = readCelsius(pin_Ta);
    // Serial.print("Ta = ");
    // Serial.print(Ta);
    // Serial.print("   ");
    lcd.setCursor(10, 0);
    lcd.print("  ");
    lcd.setCursor(10, 0);
    lcd.print(round(Ta));
    delay(5);        // delay in between reads for stability

    Tr = readCelsius(pin_Tr);
    // Serial.print("Tr = ");
    // Serial.print(Tr);
    // Serial.print("   ");
    lcd.setCursor(9, 1);
    lcd.print("   ");
    lcd.setCursor(9, 1);
    lcd.print(round(Tr));
    delay(5);        // delay in between reads for stability

    Tf = readCelsius(pin_Tf);
    // Serial.print("Tf = ");
    // Serial.print(Tf);
    // Serial.print("   ");
    lcd.setCursor(5, 1);
    lcd.print("   ");
    lcd.setCursor(5, 1);
    lcd.print(round(Tf));
    delay(5);        // delay in between reads for stability

    gas_sensorValue = analogRead(A1); // MQ2_sensor
    //    // Serial.print(gas_sensorValue);
    //    // Serial.print(" ");
    lcd.setCursor(5, 0);
    lcd.print("    ");
    lcd.setCursor(5, 0);
    lcd.print(gas_sensorValue);
    // Serial.print("gas = ");
    // Serial.println(gas_sensorValue);
    delay(5);        // delay in between reads for stability

    startMillisLCD = currentMillisLCD ;
  }

  currentMillisSD = millis();

  if (currentMillisSD - startMillisSD >= periodSD)
  {
    mySensorData = SD.open(filename, FILE_WRITE);
    if (mySensorData)
    {
      DateTime now = rtc.now();
      mySensorData.print(now.year(),DEC);
      mySensorData.print("/");
      mySensorData.print(now.month(),DEC);
      mySensorData.print("/");
      mySensorData.print(now.day(),DEC);
      mySensorData.print(",");
      mySensorData.print(now.hour(),DEC);
      mySensorData.print(":");
      mySensorData.print(now.minute(),DEC);
      mySensorData.print(":");
      mySensorData.print(now.second(),DEC);
      mySensorData.print(",");
      mySensorData.print(currentMillisSD);
      mySensorData.print(",");

      mySensorData.print(t_min);
      mySensorData.print(",");

      mySensorData.print(Tr);
      mySensorData.print(",");

      mySensorData.print(Tf);
      mySensorData.print(",");

      mySensorData.print(Ta);
      mySensorData.print(",");

      mySensorData.print(CO2_molar_flow);
      mySensorData.print(",");

      mySensorData.print(CH4_molar_flow);
      mySensorData.print(",");

      mySensorData.println(gas_sensorValue);

      mySensorData.close();
      // Serial.println("written to SD Card !");
      startMillisSD = currentMillisSD ;
    }
    else {
      lcd.clear();
      lcd.print("erro ao abrir");
      lcd.setCursor(0, 1);
      lcd.print(filename);
      delay(1000);
      
    }
  }

}
