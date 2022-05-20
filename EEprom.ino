#include <Preferences.h>

Preferences preferences;

void loadWiFiConfig(){
  preferences.begin("wificonfig", true);
  ssid = preferences.getString("SSID", "");
  ssidPassword = preferences.getString("pass", "");
  staticIPaddr = preferences.getString("IPAddr", "");
  subnet = preferences.getString("subnet", "255.255.255.0");
  gateway = preferences.getString("gateway", "0.0.0.0");
  preferences.end();
}

void storeWiFiConfig(){
  preferences.begin("wificonfig", false);
  preferences.putString("SSID", ssid);
  preferences.putString("pass", ssidPassword);
  preferences.putString("IPAddr", staticIPaddr);
  preferences.putString("subnet", subnet);
  preferences.putString("gateway", gateway);
  preferences.end();
  delay(10000);
  ESP.restart();
}

void resetWifiConfig(){
  preferences.begin("wificonfig", false);
  preferences.putString("SSID", "");
  preferences.putString("pass", "");
  preferences.putString("IPAddr", "");
  preferences.putString("subnet", "255.255.255.0");
  preferences.putString("gateway", "0.0.0.0");
  preferences.end();
  delay(10000);
  ESP.restart();
}

void loadStepperConfig(){
  preferences.begin("stepconfig", true);
  stepperSpeed = preferences.getUInt("speed",DEFAULT_SPEED);
  stepperStepPerFlap = preferences.getUInt("steps",DEFAULT_STEPS);
  preferences.end();
  
}

void storeStepperConfig(){
  preferences.begin("stepconfig", false);
  preferences.putUInt("speed",stepperSpeed);
  preferences.putUInt("steps",stepperStepPerFlap);
  preferences.end();
  delay(10000);
}
