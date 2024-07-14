#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BME280I2C.h>

BME280I2C bme;            
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SOIL_PIN A0
#define SOIL_D_PIN 3
#define RELAY_PIN 2
#define TOUCH_PIN 7

bool pumpState = false;
bool soilState = false;
bool soilState2 = false;
int soilMoistureValue = 0;
bool pumpStatus = false;

SemaphoreHandle_t mutex_v; 

BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
BME280::PresUnit presUnit(BME280::PresUnit_Pa);
float temp(NAN), hum(NAN), pres(NAN);

String jsonString;

void setup() {
  Serial.begin(115200);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();

  while(!bme.begin()) {
    Serial.println("Could not find BME280 sensor!");
    delay(10);
  }
  
  switch(bme.chipModel()) {
    case BME280::ChipModel_BME280:
      Serial.println("Found BME280 sensor! Success.");
      break;
    case BME280::ChipModel_BMP280:
      Serial.println("Found BMP280 sensor! No Humidity available.");
      break;
    default:
      Serial.println("Found UNKNOWN sensor! Error!");
  }

  mutex_v = xSemaphoreCreateMutex(); 
  if (mutex_v == NULL) { 
    Serial.println("Mutex can not be created"); 
  } 

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT);
  pinMode(SOIL_D_PIN, INPUT);

  xTaskCreate(lerSensor, "SENSOR", 100, NULL, 1, NULL);
  xTaskCreate(printLCD, "LCD", 200, NULL, 2, NULL);
  xTaskCreate(checkSoilMoistureAndPump, "CHECK_SOIL_PUMP", 70, NULL, 1, NULL);
  
  vTaskStartScheduler();   
}

void loop() {
  if (pumpStatus == 1 || pumpState == 1 || soilState2 == 1 ) {
    digitalWrite(RELAY_PIN, HIGH);  // Turn on pump
  } else {
    digitalWrite(RELAY_PIN, LOW);   // Turn off pump
  }
    
  if (Serial.available() > 0) {
    byte k = Serial.read();
    // Serial.println(k);
    if (k == 49) {
      pumpStatus = 1;
    } else {
      pumpStatus = 0;
    }
  }
}

void lerSensor(void *parametro) {
  while(true) {
    soilState = digitalRead(SOIL_D_PIN) == LOW;
    soilMoistureValue = analogRead(SOIL_PIN);

    if (digitalRead(TOUCH_PIN) == HIGH) {
      pumpState = 0;
    } else {
      vTaskDelay(20 / portTICK_PERIOD_MS);
      if (digitalRead(TOUCH_PIN) == LOW) {
        pumpState = 1;
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void printLCD(void *parametro) {
  while(true) {
    xSemaphoreTake(mutex_v, portMAX_DELAY); 
    bme.read(pres, temp, hum, tempUnit, presUnit);
    lcd.setCursor(0, 0);
    lcd.print(F("H:"));
    lcd.print(round(hum));
    lcd.print(F("%"));
    lcd.setCursor(8, 0);
    lcd.print(F("T:"));
    lcd.print(round(temp));
    lcd.print(F("oC"));
    lcd.setCursor(0, 1);
    lcd.print(F("PUMP:"));
    lcd.print((pumpState));
    lcd.setCursor(7, 1);
    lcd.print(F("Soid:"));
    lcd.print(soilMoistureValue);
    lcd.print(F(" "));
    vTaskDelay(100 / portTICK_PERIOD_MS); 
    xSemaphoreGive(mutex_v);    
    

    Serial.print(F("{\"Humidity\":"));
    Serial.print(round(hum));
    Serial.print(F(",\"Temperature\":"));
    Serial.print(round(temp));
    Serial.print(F(",\"soilMoistureValue\":"));
    Serial.print(soilMoistureValue);
    Serial.print(F(",\"pumpState\":"));
    Serial.print(pumpState);
    Serial.println(F("}"));
  }
}

void checkSoilMoistureAndPump(void *parametro) {
  while(true) {
    if (soilMoistureValue > 900) {
      soilState2 = 1;
      } else {
      soilState2 = 0;
      }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
