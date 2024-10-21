#include <mcp_can.h>
#include <motctrl_prot.h>
#include <SPI.h>

MCP_CAN CAN(10);
unsigned char tmp[8];  // Buffer for CAN messages

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

  // Reset the motor configuration
  MCReqResetConfiguration(tmp);
  CAN.sendMsgBuf(0x01, 0, 8, tmp);  // Send the CAN message to reset configuration
  delay(100);  // Small delay to ensure the message is sent

  // Wait for response and print it
  if (CAN_MSGAVAIL == CAN.checkReceive()) {  // Check if a message is available on the CAN bus
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char buf[8];
    
    CAN.readMsgBuf(&rxId, &len, buf);  // Read CAN message
    MOTCTRL_RES res = MCResResetConfiguration(buf);
    if (res == MOTCTRL_RES_SUCCESS) {
      Serial.println("Configuration reset successful.");
    } else {
      Serial.println("Configuration reset failed.");
    }
  }

  // Request motor fault status
  MCReqGetFault(tmp);
  CAN.sendMsgBuf(0x01, 0, 8, tmp);  // Send the CAN message to get faults
  delay(100);  // Small delay to ensure the message is sent

  // Wait for fault response and print it
  if (CAN_MSGAVAIL == CAN.checkReceive()) {  // Check if a message is available on the CAN bus
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char buf[8];
    
    CAN.readMsgBuf(&rxId, &len, buf);  // Read CAN message
    MOTCTRL_FAULTNO faultNo;
    MOTCTRL_RES res = MCResGetFault(buf, &faultNo);
    if (res == MOTCTRL_RES_SUCCESS) {
      Serial.print("Fault Number: ");
      Serial.println(faultNo, HEX);

      // Acknowledge the fault
      MCReqAckFault(tmp);
      CAN.sendMsgBuf(0x01, 0, 8, tmp);  // Send the CAN message to acknowledge the fault
      delay(100);  // Small delay to ensure the message is sent

      // Wait for acknowledge response and print it
      if (CAN_MSGAVAIL == CAN.checkReceive()) {  // Check if a message is available on the CAN bus
        CAN.readMsgBuf(&rxId, &len, buf);  // Read CAN message
        res = MCResAckFault(buf);
        if (res == MOTCTRL_RES_SUCCESS) {
          Serial.println("Fault acknowledged successfully.");
        } else {
          Serial.println("Failed to acknowledge fault.");
        }
      }
    } else {
      Serial.println("Failed to retrieve fault information.");
    }
  }
}

void loop() {
  // Nothing to do here
}

// Function that waits for 's' input from the serial monitor
void waitForSerialStart() {
  while (true) {  // Infinite loop until 's' input is detected
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == 's') {
        Serial.println("Serial start command detected. Starting configuration reset...");
        break;  // Exit the loop to start configuration reset
      }
    }
    delay(100);  // Small delay to prevent busy waiting
  }
}
