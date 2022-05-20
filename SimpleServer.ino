#include <WiFi.h>
//#include <WiFiClient.h>
#include <WiFiAP.h>

#include <String.h>
#include <postParser.h>

WiFiServer server(80);

uint32_t chipId = 0;
String ssidString;

const char * pageHTML = "<!DOCTYPE html>"
      "<html>"
    "<body>"
    "<h2>WiFi Config:</h2>"
    "<form action=\"/UpdateSSID\" method=\"post\">"
    "  SSID:<br/>"
    "   <input type=\"text\" name=\"SSID\" value=\"@SSID\"><br>"
    "    Password:<br/>"
    "    <input type=\"text\" name=\"Password\" value=\"@PASS\"><br/>"
    "    IP address:<br/>"
    "    <input type=\"text\" name=\"IP Address\" value=\"@IP\"><br/>"
    "    Subnet:<br/>"
    "    <input type=\"text\" name=\"Subnet\" value=\"@SUBNET\"><br/>"
    "    Gateway:<br/>"
    "    <input type=\"text\" name=\"Gateway\" value=\"@GATEWAY\"><br/>"
    "    <input type=\"submit\" value=\"Save\">"
    "</form>"
    "<h2>Stepper Config:</h2>"
    "<form action=\"/UpdateStepper\" method=\"post\">"
    " Speed:<br/>"
    "   <input type=\"text\" name=\"Speed\" value=\"@SPEED\"><br>"
    "    Steps Per Flap:<br/>"
    "    <input type=\"text\" name=\"Steps\" value=\"@STEPS\"><br/>"
    "    <input type=\"submit\" value=\"Save\">"
    "</form>"
    ""
    "<h2>Stepper Control:</h2>"
    "<form action=\"/MoveStepper\" method=\"post\">"
    " Steps:<br/>"
    "   <input type=\"text\" name=\"Steps\" value=\"@STEPS\"><br>"
    "    <input type=\"submit\" name=\"Advance\" value=\"Advance\">"
    "</form>"
    ""
    "</body>"
    "</html>";

void taskSimpleServer(void * pvParameters);
void sendResponse(WiFiClient client);

void initServer(){
  xTaskCreatePinnedToCore(
    taskSimpleServer
    ,  "Task_SimpleServer"   // A name just for humans
    ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  
}


void initHotspot(){
  Serial.println("Initializing Hotspot Mode");
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  ssid = String(default_ssid) + String(chipId);
  WiFi.softAP(ssid.c_str(), default_password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Starting Access Point");
  Serial.println(ssid.c_str());
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
}

void initAccesspoint(){
  Serial.println("Connecting to Wifi");
  int IP[4] = {0,0,0,0};
  Serial.println(ssid);
  Serial.println(ssidPassword);
  Serial.println(staticIPaddr);
  Serial.println(subnet);
  Serial.println(gateway);
  if(staticIPaddr != ""){
    Serial.println("Using Static IP");
    getOctets(IP,&staticIPaddr);
    IPAddress local_IP(IP[0], IP[1],IP[2],IP[3]);
    getOctets(IP,&subnet);
    IPAddress subnetIP(IP[0], IP[1],IP[2],IP[3]);
    getOctets(IP,&gateway);
    IPAddress gatewayIP(IP[0], IP[1],IP[2],IP[3]);

    if (!WiFi.config(local_IP, gatewayIP, gatewayIP, subnetIP)) {
      Serial.println("STA Failed to configure");
    }
  }
  WiFi.begin(ssid.c_str(), ssidPassword.c_str());
  while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void getOctets(int * IP, String * str){
  int index = 0;
  int endIndex = 0;
  for(int i = 0; i < 4; i++){
    endIndex = str->indexOf(".",index);
    if(endIndex == -1) endIndex = str->length();
    IP[i] = ((String)str->substring(index,endIndex)).toInt();
    index = endIndex+1;
    //Serial.println(IP[i]);
  }
}


void taskSimpleServer(void * pvParameters){
  Serial.println();
  Serial.println("Configuring Network Connection...");

  //If we have no ssid to connect to start up in hotspot mode
  if(ssid == "" ){
    initHotspot();
  } else {
    initAccesspoint();
  }

  WifiUP = true;

  server.begin();
  Serial.println("Server started");

  for(;;){
    uint32_t numSteps = 0;
    WiFiClient client = server.available();     // listen for incoming clients
    PostParser postParser = PostParser(client); // create our parser
    while (client) {                               // if you get a client,
      Serial.println("New Client.");            // print a message out the serial port
      postParser.read();
      String path = postParser.getPath();
      
      if( postParser.getRequestType() == "POST"){
        if(path == "/UpdateSSID"){
          Serial.println("Updating SSID");
          ssid = postParser.getField("SSID");
          ssidPassword = postParser.getField("Password");
          staticIPaddr = postParser.getField("IP+Address");
          subnet = postParser.getField("Subnet");
          gateway = postParser.getField("Gateway");
          storeWiFiConfig();
        }
        else if(path == "/UpdateStepper"){
          Serial.println("Updating Stepper Config");
          stepperSpeed = ((String)postParser.getField("Speed")).toInt();
          stepperStepPerFlap = ((String)postParser.getField("Steps")).toInt();
          storeStepperConfig();
        }
        else if(path == "/MoveStepper"){
          Serial.println("Moving Stepper");
          numSteps = ((String)postParser.getField("Steps")).toInt();
          xTaskNotify( taskHandleStepperControl, numSteps, eSetValueWithoutOverwrite );
        }
      } else if(path == "/go"){
          xTaskNotify( taskHandleStepperControl, stepperStepPerFlap, eSetValueWithoutOverwrite );
      }

      if(client.connected()){
        sendResponse(client);
        vTaskDelay(100); //Wait 100ms between checks for connections
        client.stop();
      } else {
        Serial.println("Client Disconnected.");
      }
    }
    vTaskDelay(100); //Wait 100ms between checks for connections
  }
}


void sendResponse(WiFiClient client){
  String strResponse = pageHTML;
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();

  // the content of the HTTP response follows the header:
  strResponse.replace("@SSID",ssid);
  strResponse.replace("@PASS",ssidPassword);
  strResponse.replace("@IP",staticIPaddr);
  strResponse.replace("@SUBNET",subnet);
  strResponse.replace("@GATEWAY",gateway);
  strResponse.replace("@SPEED",String(stepperSpeed));
  strResponse.replace("@STEPS",String(stepperStepPerFlap));
  //strResponse.replace("@MOVE",stepperStepPerFlap);
  client.println(strResponse.c_str());

  // The HTTP response ends with another blank line:
  client.println();
  
}
