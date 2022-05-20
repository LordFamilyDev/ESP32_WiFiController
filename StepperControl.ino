#include <Stepper.h>

#define NUM_STEPS 4096

#define bluePin 32
#define pinkPin 33
#define yellowPin 25
#define orangePin 26


// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper stepper(NUM_STEPS, bluePin,yellowPin,pinkPin,orangePin);

void taskStepperControl(void * pvParameters);


void initSteppers(){
  xTaskCreatePinnedToCore(
    taskStepperControl
    ,  "Task_ModbusServer"   // A name just for humans
    ,  2048  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &taskHandleStepperControl 
    ,  ARDUINO_RUNNING_CORE);
}


void taskStepperControl(void * pvParameters){
  if(stepperSpeed > 0)
    stepper.setSpeed(stepperSpeed);
  else
    stepper.setSpeed(1);

    
  for(;;){
    uint32_t numSteps;
    //Hold here until notified from modbus or simple server
    xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                     ULONG_MAX, /* Reset the notification value to 0 on exit. */
                     &numSteps, /* Notified value pass out in ulNotifiedValue. */
                     portMAX_DELAY );  /* Block indefinitely. */

    Serial.println("Stepping...");
    stepper.step(-1 * numSteps);
    Serial.println("Done Stepping");
    xTaskNotify( taskHandleModbus, 1, eSetValueWithOverwrite );
  }
}
