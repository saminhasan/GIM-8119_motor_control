#include <mcp_can.h>
#include <motctrl_prot.h>
#include <SPI.h>
#include <math.h>  

MCP_CAN CAN(10);
unsigned char tmp[8];  //  CAN messages Buffer

unsigned long previousMillis = 0;
unsigned long interval = 500;  // Time interval between loops in milliseconds

float target_position_deg = 0;
float actual_position_deg = 0;

void setup()
{
  Serial.begin(115200);

  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) 
    Serial.println("CAN init ok!!");
  else 
    Serial.println("CAN init fail!!");

  CAN.setMode(MCP_NORMAL);
  Serial.println("Ready to start");
  waitForSerialStart();

  // Start the motor
  MCReqStartMotor(tmp);
  CAN.sendMsgBuf(0x01, 0, 8, tmp);  
  delay(100);
}

void loop()
{
  unsigned long currentMillis = millis();

  // Update motor position at fixed intervals
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Send position control command to motor
    uint32_t duration = interval;  
    MCReqPositionControl(tmp, target_position_deg* M_PI/ 180 , duration);
    CAN.sendMsgBuf(0x01, 0, 8, tmp);  
    delay(1);

    getMotorFeedback();

    Serial.print(target_position_deg);
    Serial.print(",");
    Serial.println(actual_position_deg);
  }

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // Remove any whitespace or newline characters
    if (input.length() > 0) {
      target_position_deg = input.toFloat();
      Serial.print("New target position set to: ");
      Serial.println(target_position_deg);
    }
  }
}

// Function to retrieve actual position feedback
void getMotorFeedback() {
  int8_t temp;
  float position, speed, torque;  

  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char buf[8];
    
    CAN.readMsgBuf(&rxId, &len, buf); 
    
    if (MCResPositionControl(buf, &temp, &position, &speed, &torque) == MOTCTRL_RES_SUCCESS) {
      actual_position_deg = position * 180 / M_PI;  // Convert RAD to degrees
    }
  }
}

void stopMotor() {
  MCReqStopMotor(tmp);
  CAN.sendMsgBuf(0x01, 0, 8, tmp);
}

// Function that waits for 's' input from the serial monitor
void waitForSerialStart() {
  while (true) {
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == 's') {
        Serial.println("Serial start command detected. Starting motor position control...");
        break;  // Exit the loop to start the main loop
      }
    }
    delay(100);
  }
}
