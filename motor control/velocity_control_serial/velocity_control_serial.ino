#include <mcp_can.h>
#include <motctrl_prot.h>
#include <SPI.h>
#include <math.h>

MCP_CAN CAN(10);
unsigned char tmp[8];

// Time tracking
unsigned long previousMillis = 0;
unsigned long interval = 500;

float target_velocity_rpm = 0;
float actual_velocity_rpm = 0;

void setup()
{
  Serial.begin(115200);

  // Initialize CAN bus at 500 kbps
  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) 
    Serial.println("CAN init ok!!");
  else 
    Serial.println("CAN init fail!!");

  CAN.setMode(MCP_NORMAL);   // Set CAN to normal mode to allow message transmission
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

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    uint32_t duration = interval;
    MCReqSpeedControl(tmp, target_velocity_rpm * 2 * M_PI * 9 / 60, duration);  // Convert RPM to RAD/s * gear ratio
    CAN.sendMsgBuf(0x01, 0, 8, tmp); 
    delay(1);

    getMotorFeedback();

    Serial.print(target_velocity_rpm);  // Target Velocity (RPM)
    Serial.print(",");
    Serial.println(actual_velocity_rpm);  // Current actual Velocity from feedback
  }

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // Remove any whitespace or newline characters
    if (input.length() > 0) {
      target_velocity_rpm = input.toFloat();
      Serial.print("New target velocity set to: ");
      Serial.println(target_velocity_rpm);
    }
  }
}

void getMotorFeedback() {
  int8_t temp;
  float position, speed, torque;  // Use float for position, speed, and torque

  if (CAN_MSGAVAIL == CAN.checkReceive()) { 
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char buf[8];
    
    CAN.readMsgBuf(&rxId, &len, buf);  // Read CAN message
    
    if (MCResSpeedControl(buf, &temp, &position, &speed, &torque) == MOTCTRL_RES_SUCCESS) {
      actual_velocity_rpm = speed * 60 / (2 * M_PI);  // Convert RAD/s to RPM
    }
  }
}

void stopMotor() {
  MCReqStopMotor(tmp);
  CAN.sendMsgBuf(0x01, 0, 8, tmp); 
}

// Function that waits for 's' input from the serial monitor
void waitForSerialStart() {
  while (true) {  // Infinite loop until 's' input is detected
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == 's') {
        Serial.println("Serial start command detected. Starting motor velocity control...");
        break;  // Exit the loop to start the main loop
      }
    }
    delay(100);  // Small delay to prevent busy waiting
  }
}
