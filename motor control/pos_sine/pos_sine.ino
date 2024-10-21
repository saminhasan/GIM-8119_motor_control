#include <mcp_can.h>
#include <motctrl_prot.h>
#include <SPI.h>
#include <math.h>  

MCP_CAN CAN(10);
unsigned char tmp[8];  // Buffer for CAN messages

unsigned long previousMillis = 0;
unsigned long interval = 33;  // Time interval between updates in milliseconds (33 Hz sampling rate)
unsigned long startTime = 0;

float target_position_deg = 0;
float actual_position_deg = 0;
float sine_amplitude = 45;   // Amplitude of the sine wave in degrees
float sine_frequency = 2;  // Frequency of the sine wave in Hz
float ramp_up_duration = 5000.0;  // Ramp-up duration in milliseconds

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
  CAN.sendMsgBuf(0x01, 0, 8, tmp);  // Send the CAN message
  delay(100);  // Small delay to ensure the message is processed

  // offest the start time for sine wave generation, so that there is no big jump at the start
  startTime = millis();
}

void loop()
{
  unsigned long currentMillis = millis();

  // Calculate the elapsed time since the start
  unsigned long elapsedTime = currentMillis - startTime;

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Calculate the target position based on a sine wave with ramp-up
    float time_sec = elapsedTime / 1000.0;
    float ramp_factor = min(1.0, elapsedTime / ramp_up_duration);
    target_position_deg = ramp_factor * sine_amplitude * sin(2 * M_PI * sine_frequency * time_sec);

    uint32_t duration = interval;
    MCReqPositionControl(tmp, target_position_deg * M_PI / 180, duration);  // Convert degrees to radians
    CAN.sendMsgBuf(0x01, 0, 8, tmp);
    delay(1);  

    getMotorFeedback();

    // Print the target and actual positions in CSV format for Serial Plotter
    Serial.print(target_position_deg);  // Target Position (degrees)
    Serial.print(",");
    Serial.println(actual_position_deg);  // Current actual Position from feedback
  }

  // // Check for serial input to update sine wave parameters
  // if (Serial.available() > 0) {
  //   String input = Serial.readStringUntil('\n');
  //   input.trim();  // Remove any whitespace or newline characters
  //   if (input.startsWith("a=")) {
  //     sine_amplitude = input.substring(2).toFloat();  // Update sine wave amplitude
  //     Serial.print("New sine wave amplitude set to: ");
  //     Serial.println(sine_amplitude);
  //   } else if (input.startsWith("f=")) {
  //     sine_frequency = input.substring(2).toFloat();  // Update sine wave frequency
  //     Serial.print("New sine wave frequency set to: ");
  //     Serial.println(sine_frequency);
  //   }
  // }
}

// Function to retrieve  feedback
void getMotorFeedback() {
  int8_t temp;
  float position, speed, torque;  // Use float for position, speed, and torque

  if (CAN_MSGAVAIL == CAN.checkReceive()) {  // Check if a message is available on the CAN bus
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char buf[8];
    
    CAN.readMsgBuf(&rxId, &len, buf);  // Read CAN message
    
    if (MCResPositionControl(buf, &temp, &position, &speed, &torque) == MOTCTRL_RES_SUCCESS) {
      actual_position_deg = position * 180 / M_PI;  // Convert radians to degrees
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
        Serial.println("Serial start command detected. Starting motor position control...");
        break;  // Exit the loop to start the main loop
      }
    }
    delay(100);  // Small delay to prevent busy waiting
  }
}
