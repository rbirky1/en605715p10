#include <Wire.h>
#include <Adafruit_BNO055.h>

int debug = 1;
char buffer[40];
byte data[6];
int I2C_ADDR = 0X7F;
Adafruit_BNO055 bno = Adafruit_BNO055(55);

void setup() {
  Serial.begin(9600);
  
  if(!bno.begin()) {
    while(1);
  }
  delay(1000);
  bno.setExtCrystalUse(true);
  
  Wire.begin(I2C_ADDR);
  Wire.onRequest(requestEvent);
}

void loop() {
  Serial.println("Waiting for an I2C request.");
  
  sensors_event_t event; 
  bno.getEvent(&event);
  int roll = (int) event.orientation.z;
  int pitch = (int) event.orientation.x;
  int yaw = (int) event.orientation.y;

  byte roll_low = (roll >> 8);
  byte roll_high = roll;

  byte pitch_low = (pitch >> 8);
  byte pitch_high = pitch;

  byte yaw_low = (yaw >> 8);
  byte yaw_high = yaw;
  
  data[0] = roll_high;
  data[1] = roll_low;
  data[2] = pitch_high;
  data[3] = pitch_low;
  data[4] = yaw_high;
  data[5] = yaw_low;

  if (debug) {
    // Raw
    sprintf(buffer, "roll:%d, pitch:%d, yaw:%d\n", roll, pitch, yaw);
    Serial.print(buffer);

    // Converted
    int roll_i = ((data[1] << 8) | (data[0]));
    int pitch_i = ((data[3] << 8) | (data[2]));
    int yaw_i = ((data[5] << 8) | (data[4]));
    Serial.print("roll:");
    Serial.print(roll_i);
    Serial.print(", pitch:");
    Serial.print(pitch_i);
    Serial.print(", yaw:");
    Serial.println(yaw_i);
  }
  
  delay(10000);
}

void requestEvent() {
  Serial.println("Received I2C request.");
  Wire.write(data, 6);
}
