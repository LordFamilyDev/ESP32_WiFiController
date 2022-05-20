#include <WiFi.h>
#include <ArduinoModbus.h>

ModbusTCPServer modbusTCPServer;
WiFiServer wifiModbusServer(502);
WiFiClient modbusClient = WiFiClient(255);


void taskModbusServer(void * pvParameters);

void initModbus(){
  xTaskCreatePinnedToCore(
    taskModbusServer
    ,  "Task_ModbusServer"   // A name just for humans
    ,  2048  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &taskHandleModbus 
    ,  ARDUINO_RUNNING_CORE);
      
}


void taskModbusServer(void * pvParameters){

  while(!WifiUP){
    vTaskDelay(1000);
  }
  // start the Modbus TCP server
  if (!modbusTCPServer.begin()) {
    Serial.println("Failed to start Modbus TCP Server!");
  }
  wifiModbusServer.begin();
  modbusTCPServer.configureCoils(0x00, 1);
  modbusTCPServer.coilWrite(0x00,0);
  Serial.println("Modbus Server Started");

  for(;;){
    //Check Server port for client connections
    modbusClient = wifiModbusServer.available();
    if(modbusClient){
      // a new client connected
      Serial.println("new modbus client");
      // let the Modbus TCP accept the connection 
      modbusTCPServer.accept(modbusClient);

      while(modbusClient.connected()){
        // continuously poll for Modbus TCP requests, while client connected
        modbusTCPServer.poll();
        // update the modbus coils accordingly
        updateCoils();
        vTaskDelay(100); //Wait 100ms between processing commands
      }
      
    }
    vTaskDelay(100); //Wait 100ms between checks for connections
  }
}



void updateCoils() {
  static int lastCoilValue = 0;
  // read the current value of the coil
  //NOTE: Coil #1 is at address 0x00 cause non programers be dumb
  int coilValue = modbusTCPServer.coilRead(0x00);
  
  if (coilValue == 1 && lastCoilValue == 0) {
    Serial.println("Coil Set...");
    xTaskNotify( taskHandleStepperControl, stepperStepPerFlap, eSetValueWithOverwrite );
    lastCoilValue = 1;
  } 
  
  if(xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                      ULONG_MAX, /* Reset the notification value to 0 on exit. */
                      NULL, /* Notified value pass out in ulNotifiedValue. */
                      0)  /* Do not block. */
                      ){
    clearCoils();
    lastCoilValue = 0;
  }
  
}

void clearCoils() {
  modbusTCPServer.coilWrite(0x00,0);
}
