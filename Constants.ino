#define LED_BUILTIN 2   // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED
#define DEFAULT_SPEED 8
#define DEFAULT_STEPS 256

const char *default_ssid = "PropThing_";
const char *default_password = "ParamountProps";

bool WifiUP = false;

String ssid = "";
String ssidPassword = "";
String staticIPaddr = "";
String subnet = "";
String gateway = "";

uint32_t stepperSpeed = DEFAULT_SPEED;
uint32_t stepperStepPerFlap = DEFAULT_STEPS;

TaskHandle_t taskHandleStepperControl = NULL;
TaskHandle_t taskHandleModbus = NULL;
