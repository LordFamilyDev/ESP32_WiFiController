#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

void setup() {
  Serial.begin(115200);
  Serial.println("Tick Delay:");
  Serial.println(portTICK_PERIOD_MS);

  loadWiFiConfig();
  loadStepperConfig();
  
  initServer();
  initModbus();
  initSteppers();
  initButtons();
}

void loop() {
  //Using RTOS Tasks
}
