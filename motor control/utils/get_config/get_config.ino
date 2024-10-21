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

  // Retrieve and print all configuration items
  retrieveAndPrintConfiguration();
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
        Serial.println("Serial start command detected. Starting configuration retrieval...");
        break;  // Exit the loop to start configuration retrieval
      }
    }
    delay(100);  // Small delay to prevent busy waiting
  }
}

void retrieveAndPrintConfiguration() {
  // Iterate over all integer configuration items
  for (uint8_t confID = 0x00; confID <= 0x1C; confID++) {
    MCReqRetrieveConfiguration(tmp, MOTCTRL_CONFTYPE_FLOAT, confID);
    CAN.sendMsgBuf(0x01, 0, 8, tmp);  // Send the CAN message to retrieve configuration
    delay(100);  // Small delay to ensure the message is sent

    // Wait for response and print it
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
      long unsigned int rxId;
      unsigned char len = 0;
      unsigned char buf[8];
      
      CAN.readMsgBuf(&rxId, &len, buf);  // Read CAN message
      MOTCTRL_CONFTYPE confType;
      MOTCTRL_CONFID confIDResponse;
      float confData;
      MOTCTRL_RES res = MCResRetrieveConfiguration(buf, &confType, &confIDResponse, &confData);
      if (res == MOTCTRL_RES_SUCCESS) {
        Serial.print("Configuration ID: 0x");
        Serial.print(confIDResponse, HEX);
        Serial.print(", Value: ");
        Serial.println(confData);
      } else {
        Serial.println("Failed to retrieve configuration.");
      }
    }
  }

  // Iterate over all float configuration items
  for (uint8_t confID = 0x00; confID <= 0x05; confID++) {
    MCReqRetrieveConfiguration(tmp, MOTCTRL_CONFTYPE_FLOAT, confID);
    CAN.sendMsgBuf(0x01, 0, 8, tmp);  // Send the CAN message to retrieve configuration
    delay(100);  // Small delay to ensure the message is sent

    // Wait for response and print it
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
      long unsigned int rxId;
      unsigned char len = 0;
      unsigned char buf[8];
      
      CAN.readMsgBuf(&rxId, &len, buf);  // Read CAN message
      MOTCTRL_CONFTYPE confType;
      MOTCTRL_CONFID confIDResponse;
      float confData;
      MOTCTRL_RES res = MCResRetrieveConfiguration(buf, &confType, &confIDResponse, &confData);
      if (res == MOTCTRL_RES_SUCCESS) {
        Serial.print("Configuration ID: 0x");
        Serial.print(confIDResponse, HEX);
        Serial.print(", Value: ");
        Serial.println(confData);
      } else {
        Serial.println("Failed to retrieve configuration.");
      }
    }
  }
}
