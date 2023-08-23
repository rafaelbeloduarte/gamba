// file name to use for writing
const char filename[] = "dados.csv";
float T1 = 0;
float T2 = 0;
int gas_sensorValue = 0;

 
// Data Logger shield
 
#include<Wire.h>
// #include "RTClib.h" // Must download the library from manage library &gt; type RTCLib by Adafruit
#include<SPI.h>
#include<SD.h>
#include <LiquidCrystal.h>
 
//RTC_DS1307 RTC;
//DateTime now;
 
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
 
File mySensorData;
unsigned long startMillisSD;
unsigned long currentMillisSD;
const unsigned long periodSD = 5000;
unsigned long startMillisLCD;
unsigned long currentMillisLCD;
const unsigned long periodLCD = 1000;
unsigned long t_min = 0;

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

    return v*0.25;
}

void printLabels(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Gas:");
    lcd.setCursor(8,0);
    lcd.print("T1:");
    lcd.setCursor(8,1);
    lcd.print("T2:");
    lcd.setCursor(14,1);
    lcd.print((char) 223);
    lcd.setCursor(15,1);
    lcd.print("C");
    lcd.setCursor(14,0);
    lcd.print((char) 223);
    lcd.setCursor(15,0);
    lcd.print("C");
    lcd.setCursor(4,1);
    lcd.print("min");
  }

void printMenu(int option){
   if (option == 1){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Menu");
      delay(500);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("<Voltar");
      lcd.setCursor(8,0);
      lcd.print(">Suporte");
      lcd.setCursor(0,1);
      lcd.print("vEnvDado");
      lcd.setCursor(8,1);
      lcd.print("^DelDado");
      }
    else if (option == 11){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("duarterafaelbelo");
      lcd.setCursor(0,1);
      lcd.print("@gmail.com");
      }
  }

void dumpToSerial() {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  Serial.println();
  Serial.println();
  Serial.print("v Dados v");
  Serial.println();
  Serial.println();
  
  File dataFile = SD.open(filename);

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  Serial.println();
  Serial.println();
}

void deleteData(){
  lcd.clear();
  lcd.print("Tem certeza?");
  lcd.setCursor(0,1);
  lcd.print("< Nao  > Sim");
  delay(500);
  while (1){
    int x = analogRead (0);
    delay(1);
    if (x < 60) {
        lcd.clear();
        lcd.print("Pressione RST");
        lcd.setCursor(0,1);
        lcd.print("para cancelar");
        delay(2000);
        lcd.clear();
        lcd.print("Apagando em...");
        lcd.setCursor(0,1);
        for (int i = 5; i>=0; i--){
          lcd.print(i);
          lcd.print(" ");
          delay(1000);
          }
        SD.remove(filename);
        lcd.clear();
        lcd.print(filename);
        lcd.setCursor(0,1);
        lcd.print("apagado");
        delay(3000);
        break;
      }
     else if (x < 600){
        delay(100);
        break;
      }
    }
  printLabels();
  }

void setup()
{ 
  // chip select pins for sensors, must be high, otherwise serial bus will be busy
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  Serial.begin(115200);
  Serial.println("Sistema de coleta de dados Gambá");
   
  // 2 - Data Logger shield
   
  pinMode(chipSelect,OUTPUT);
//  RTC.begin();
  Wire.begin();
  SD.begin(chipSelect);

  // for LCD
  lcd.begin(16, 2);

  lcd.setCursor(3,0);
  lcd.print("Sistema de");
  lcd.setCursor(0,1);
  lcd.print("coleta de dados");
  delay(1000);
  
  // If you want to start from an empty file,
  // uncomment the next line:
  // SD.remove(filename);

  // try to open the file for writing
  mySensorData = SD.open(filename, FILE_WRITE);
  if (!mySensorData) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("erro ao abrir ");
    delay(5);
    Serial.print("error opening ");
    Serial.println(filename);
    lcd.setCursor(0,1);
    lcd.print(filename);
    while (1);
  }
  else{
    Serial.print(filename);
    Serial.println(" aberto");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(filename);
    lcd.setCursor(0,1);
    lcd.print("aberto");
    delay(2000);
    }

  // add some new lines to start
  mySensorData.println("millis,t_min,T1,T2,gas");
  mySensorData.close();
  
  startMillisSD = millis();
//  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  if(! RTC.isrunning())
//  {
//    Serial.println("RTC is not running !");
//  }

    #if WAIT_TO_START
    lcd.setCursor(0,0);
    lcd.print("Aperte qualquer");
    lcd.setCursor(0,1);
    lcd.print("tecla p/ iniciar");
    while (analogRead(0)>800);
    #endif //WAIT_TO_START

  printLabels();
   
}
 
void loop()
 
{
  delay(5);

  // to detect key pressed
  int x;
  x = analogRead (0);
  if (x < 800){
    printMenu(1);
    while(1){
       x = analogRead (0);
       if (x < 60) {
          printMenu(11);
        }
        else if (x < 200) {
          deleteData();
          break;
        }
        else if (x < 400){
          dumpToSerial();
          delay(1000);
          lcd.clear();
          lcd.print("Dados enviados");
          lcd.setCursor(0,1);
          lcd.print("p/ porta serial");
          delay(5000);
          printLabels();
          break;
        }
        else if (x < 600){
          printLabels();
          delay(100);
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
    t_min = millis()/(60000);
    lcd.setCursor(0,1);
    lcd.print(t_min);
    Serial.print("millis = ");
    Serial.print(currentMillisSD);
    Serial.print("   ");
    delay(5);        // delay in between reads for stability
  
    T1 = readCelsius(2);
    Serial.print("T1 = ");
    Serial.print(T1);
    Serial.print("   ");
    lcd.setCursor(11,0);
    lcd.print("   ");
    lcd.setCursor(11,0);
    lcd.print(round(T1));
    delay(5);        // delay in between reads for stability
  
    T2 = readCelsius(3);
    Serial.print("T2 = ");
    Serial.print(T2);
    Serial.print("   ");
    lcd.setCursor(11,1);
    lcd.print("   ");
    lcd.setCursor(11,1);
    lcd.print(round(T2));
    delay(5);        // delay in between reads for stability
    
    gas_sensorValue = analogRead(A1); // MQ2_sensor
  //    Serial.print(gas_sensorValue);
  //    Serial.print(" ");
    lcd.setCursor(4,0);
    lcd.print("    ");
    lcd.setCursor(4,0);
    lcd.print(gas_sensorValue);
    Serial.print("gas = ");
    Serial.println(gas_sensorValue);
    delay(5);        // delay in between reads for stability
    startMillisLCD = currentMillisLCD ;
    }

  currentMillisSD = millis();
  
  if (currentMillisSD - startMillisSD >= periodSD)
    {
    mySensorData=SD.open(filename,FILE_WRITE);
    if(mySensorData)
      {
      //DateTime now = RTC.now();
      //mySensorData.print(now.year(),DEC);
      //mySensorData.print("/");
      //mySensorData.print(now.month(),DEC);
      //mySensorData.print("/");
      //mySensorData.print(now.day(),DEC);
      //mySensorData.print(" ");
      //mySensorData.print(now.hour(),DEC);
      //mySensorData.print(":");
      //mySensorData.print(now.minute(),DEC);
      //mySensorData.print(":");
      //mySensorData.print(now.second(),DEC);
      //mySensorData.print(",");
      mySensorData.print(currentMillisSD);
      mySensorData.print(",");

      mySensorData.print(t_min);
      mySensorData.print(",");
      
      mySensorData.print(T1);
      mySensorData.print(",");

      mySensorData.print(T2);
      mySensorData.print(",");

      mySensorData.println(gas_sensorValue);
      
      mySensorData.close();
      Serial.println("written to SD Card !");
      startMillisSD = currentMillisSD ;
    } 
   }
 
}
