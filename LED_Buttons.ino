#define LED LED_BUILTIN
#define BUTTON 0
#define MANUAL_ADV_BTN 35
#define BTN_DEBOUNCE 500
#define RESET_HOLD_TIME 20000 //20 seconds


void taskLedButtons(void * pvParameters);

void initButtons(){
    xTaskCreatePinnedToCore(
    taskLedButtons
    ,  "Task_LED_Button"   // A name just for humans
    ,  2048  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);  
}



void taskLedButtons(void * pvParameters){

  int buttonState = 0;
  bool buttonPressed = false;
  bool toggle = true;
  TickType_t buttonPressedTick = 0;
  uint32_t buttonPressedTime = 0;
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON, INPUT);

  
  for(;;){
    
    buttonState = digitalRead(BUTTON);

    if(buttonState == LOW){
      //Button is depressed
      
      if(!buttonPressed){
        //Button was just pressed
        buttonPressed = true;
        toggle = true;
        buttonPressedTick = xTaskGetTickCount();
      }
      
      //Calc how long the button has been pressed
      // if it has been long enoug start blinking very fast
      buttonPressedTime = xTaskGetTickCount() - buttonPressedTick;
      if(toggle){
        digitalWrite(LED_BUILTIN, HIGH);
        if(buttonPressedTime > RESET_HOLD_TIME){
          toggle = false;  
        }
      } else {
        digitalWrite(LED_BUILTIN, LOW);
        toggle = true;  
      }
      
    } else {
      //Button is not depressed
      if(buttonPressed){
        //Button has been released figure out how long we pushed it and do a thing
        Serial.println("Button Released");
        buttonPressedTime = xTaskGetTickCount() - buttonPressedTick;
        if(buttonPressedTime > RESET_HOLD_TIME){
            //Reset wifi config and reboot
            Serial.println("Resetting Wifi");
            resetWifiConfig();
          
        } else if (buttonPressedTime > BTN_DEBOUNCE){
            // Just make the stepper go
            Serial.println("Sending Stepper command");
            xTaskNotify( taskHandleStepperControl, stepperStepPerFlap, eSetValueWithOverwrite );
        } else {
          Serial.println(buttonPressedTime);
        }

        buttonPressed = false;
        digitalWrite(LED_BUILTIN, LOW);
        toggle = false;  
      }
    }
    vTaskDelay(100); //Wait 100ms between checks for connections
    
  }
}
