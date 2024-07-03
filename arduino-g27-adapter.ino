#include <HID.h>

// #define DEBUG

#define USE_SHIFTER
#define USE_PEDALS
#define USE_HANDBRAKE

// Pins
#define PIN_SHIFTER_CLOCK 0
#define PIN_SHIFTER_DATA 1
#define PIN_SHIFTER_MODE 4
#define PIN_SHIFTER_X 8
#define PIN_SHIFTER_Y 9
#define PIN_PEDALS_ACCELERATOR 18
#define PIN_PEDALS_BRAKE 19
#define PIN_PEDALS_CLUTCH 20
#define PIN_HANDBRAKE 21

// Shifter thresholds
#define TRESHOLD_SHIFTER_X_LEFT 350
#define TRESHOLD_SHIFTER_X_RIGHT 650
#define TRESHOLD_SHIFTER_Y_UP 700
#define TRESHOLD_SHIFTER_Y_DOWN 200

// Shifter button indexes
#define SHIFTER_INDEX_REVERSE 1
#define SHIFTER_INDEX_RED_1 7
#define SHIFTER_INDEX_RED_2 5
#define SHIFTER_INDEX_RED_3 4
#define SHIFTER_INDEX_RED_4 6
#define SHIFTER_INDEX_BLACK_TOP 8
#define SHIFTER_INDEX_BLACK_RIGHT 9
#define SHIFTER_INDEX_BLACK_BOTTOM 11
#define SHIFTER_INDEX_BLACK_LEFT 10
#define SHIFTER_INDEX_DPAD_TOP 15
#define SHIFTER_INDEX_DPAD_RIGHT 12
#define SHIFTER_INDEX_DPAD_BOTTOM 14
#define SHIFTER_INDEX_DPAD_LEFT 13

#define SIGNAL_SETTLE_DELAY 10

#ifdef USE_SHIFTER
int shifterValueData[16];
int shifterValueX;
int shifterValueY;
int shifterCurrentGear;
uint32_t outputShifter = 0;

long normalizeOutput(int value, int valueMin, int valueMax)
{
  return map(value, valueMin, valueMax, 0, 1023);
}

void getButtonStates(int *buffer)
{
  digitalWrite(PIN_SHIFTER_MODE, LOW);
  delayMicroseconds(SIGNAL_SETTLE_DELAY);
  digitalWrite(PIN_SHIFTER_MODE, HIGH);

  for (int i = 0; i < 16; ++i)
  {
    digitalWrite(PIN_SHIFTER_CLOCK, LOW);
    delayMicroseconds(SIGNAL_SETTLE_DELAY);
    buffer[i] = digitalRead(PIN_SHIFTER_DATA);
    digitalWrite(PIN_SHIFTER_CLOCK, HIGH);
    delayMicroseconds(SIGNAL_SETTLE_DELAY);
  }
}

int getCurrentGear(int x, int y, bool reverse)
{

  if (reverse)
  {
    if (x > TRESHOLD_SHIFTER_X_RIGHT && y < TRESHOLD_SHIFTER_Y_DOWN)
      return 7;
    else
      return 0;
  }

  if (x < TRESHOLD_SHIFTER_X_LEFT)
  {
    if (y > TRESHOLD_SHIFTER_Y_UP)
      return 1;
    if (y < TRESHOLD_SHIFTER_Y_DOWN)
      return 2;
  }
  else if (x > TRESHOLD_SHIFTER_X_RIGHT)
  {
    if (y > TRESHOLD_SHIFTER_Y_UP)
      return 5;
    if (y < TRESHOLD_SHIFTER_Y_DOWN)
    {
      return 6;
    }
  }
  else
  {
    if (y > TRESHOLD_SHIFTER_Y_UP)
      return 3;
    if (y < TRESHOLD_SHIFTER_Y_DOWN)
      return 4;
  }

  return 0;
}

void pressButton(uint8_t button)
{
  bitSet(outputShifter, button);
}
void releaseButton(uint8_t button)
{
  bitClear(outputShifter, button);
}
void setButton(uint8_t button, bool pressed)
{
  pressed ? pressButton(button) : releaseButton(button);
}

void loadShifter()
{
  getButtonStates(shifterValueData);
  shifterValueX = analogRead(PIN_SHIFTER_X);
  shifterValueY = analogRead(PIN_SHIFTER_Y);
  shifterCurrentGear = getCurrentGear(shifterValueX, shifterValueY, shifterValueData[SHIFTER_INDEX_REVERSE] == 1);

  outputShifter = 0;
  if (shifterCurrentGear > 0)
    setButton(shifterCurrentGear - 1, true);

  setButton(7, shifterValueData[SHIFTER_INDEX_BLACK_TOP] == HIGH);
  setButton(8, shifterValueData[SHIFTER_INDEX_BLACK_RIGHT] == HIGH);
  setButton(9, shifterValueData[SHIFTER_INDEX_BLACK_BOTTOM] == HIGH);
  setButton(10, shifterValueData[SHIFTER_INDEX_BLACK_LEFT] == HIGH);

  setButton(11, shifterValueData[SHIFTER_INDEX_DPAD_TOP] == HIGH);
  setButton(12, shifterValueData[SHIFTER_INDEX_DPAD_RIGHT] == HIGH);
  setButton(13, shifterValueData[SHIFTER_INDEX_DPAD_BOTTOM] == HIGH);
  setButton(14, shifterValueData[SHIFTER_INDEX_DPAD_LEFT] == HIGH);

  setButton(15, shifterValueData[SHIFTER_INDEX_RED_1] == HIGH);
  setButton(16, shifterValueData[SHIFTER_INDEX_RED_2] == HIGH);
  setButton(17, shifterValueData[SHIFTER_INDEX_RED_3] == HIGH);
  setButton(18, shifterValueData[SHIFTER_INDEX_RED_4] == HIGH);
}
#endif

#ifdef USE_PEDALS
int pedalsValueAccelerator;
int pedalsValueAcceleratorMin = INFINITY;
int pedalsValueAcceleratorMax = -INFINITY;
uint16_t outputAccelerator = 0;

int pedalsValueBrake;
int pedalsValueBrakeMin = INFINITY;
int pedalsValueBrakeMax = -INFINITY;
uint16_t outputBrake = 0;

int pedalsValueClutch;
int pedalsValueClutchMin = INFINITY;
int pedalsValueClutchMax = -INFINITY;
uint16_t outputClutch = 0;

void loadPedals()
{
  pedalsValueAccelerator = analogRead(PIN_PEDALS_ACCELERATOR);
  pedalsValueAcceleratorMin = min(pedalsValueAcceleratorMin, pedalsValueAccelerator);
  pedalsValueAcceleratorMax = max(pedalsValueAcceleratorMax, pedalsValueAccelerator);
  outputAccelerator = normalizeOutput(pedalsValueAccelerator, pedalsValueAcceleratorMin, pedalsValueAcceleratorMax);

  pedalsValueBrake = analogRead(PIN_PEDALS_BRAKE);
  pedalsValueBrakeMin = min(pedalsValueBrakeMin, pedalsValueBrake);
  pedalsValueBrakeMax = max(pedalsValueBrakeMax, pedalsValueBrake);
  outputBrake = normalizeOutput(pedalsValueBrake, pedalsValueBrakeMin, pedalsValueBrakeMax);

  pedalsValueClutch = analogRead(PIN_PEDALS_CLUTCH);
  pedalsValueClutchMin = min(pedalsValueClutchMin, pedalsValueClutch);
  pedalsValueClutchMax = max(pedalsValueClutchMax, pedalsValueClutch);
  outputClutch = normalizeOutput(pedalsValueClutch, pedalsValueClutchMin, pedalsValueClutchMax);
}
#endif

#ifdef USE_HANDBRAKE
int handbrakeValue;
int handbrakeValueMin = INFINITY;
int handbrakeValueMax = -INFINITY;
uint16_t outputHandbrake = 0;

void loadHandbrake()
{
  handbrakeValue = analogRead(PIN_HANDBRAKE);
  handbrakeValueMin = min(handbrakeValueMin, handbrakeValue);
  handbrakeValueMax = max(handbrakeValueMax, handbrakeValue);
  outputHandbrake = normalizeOutput(handbrakeValue, handbrakeValueMin, handbrakeValueMax);
}
#endif

#define JOYSTICK_REPORT_ID 0x03

typedef struct
{
  uint32_t buttons;
  uint16_t acceleratorAxis;
  uint16_t brakeAxis;
  uint16_t clutchAxis;
  uint16_t handbrakeAxis;
} JoystickReport;

JoystickReport joystickReport = {0, 0, 0, 0, 0};

static const uint8_t _hidReportDescriptor[] PROGMEM = {

    // Joystick
    0x05, 0x01,               // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,               // USAGE (Joystick)
    0xa1, 0x01,               // COLLECTION (Application)
    0x85, JOYSTICK_REPORT_ID, //   REPORT_ID (3)

    // 19 Buttons (7 first is for gears)
    0x05, 0x09,       //   USAGE_PAGE (Button)
    0x19, 0x01,       //      USAGE_MINIMUM (Button 1)
    0x29, 0x13,       //      USAGE_MAXIMUM (Button 19)
    0x15, 0x00,       //      LOGICAL_MINIMUM (0)
    0x25, 0x01,       //      LOGICAL_MAXIMUM (1)
    0x75, 0x01,       //      REPORT_SIZE (1)
    0x95, 0x13,       //      REPORT_COUNT (19)
    0x55, 0x00,       //      UNIT_EXPONENT (0)
    0x65, 0x00,       //      UNIT (None)
    0x81, 0x02,       //      INPUT (Data,Var,Abs)
    0x95, 0x01,       //      REPORT_COUNT (1) - para alinhar os bits restantes
    0x75, 0x0d,       //      REPORT_SIZE (13)
    0x81, 0x03,       //      INPUT (Cnst,Var,Abs)
    0x05, 0x01,       //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,       //      USAGE (Accelerator)
    0x09, 0x31,       //      USAGE (Brake)
    0x09, 0x32,       //      USAGE (Clutch)
    0x09, 0x35,       //      USAGE (Handbrake)
    0x15, 0x00,       //      LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x03, //      LOGICAL_MAXIMUM (1023)
    0x75, 0x10,       //      REPORT_SIZE (16)
    0x95, 0x04,       //      REPORT_COUNT (4)
    0x81, 0x02,       //      INPUT (Data,Var,Abs)
    0xc0              // END_COLLECTION
};

void sendState()
{
  joystickReport.buttons = outputShifter;
  joystickReport.acceleratorAxis = outputAccelerator;
  joystickReport.brakeAxis = outputBrake;
  joystickReport.clutchAxis = outputClutch;
  joystickReport.handbrakeAxis = outputHandbrake;

  HID().SendReport(JOYSTICK_REPORT_ID, &joystickReport, sizeof(joystickReport));

#ifdef DEBUG
  delay(200);
  debug();
#endif
}

void setup()
{
  Serial.begin(38400);

  pinMode(PIN_SHIFTER_MODE, OUTPUT);
  pinMode(PIN_SHIFTER_CLOCK, OUTPUT);
  pinMode(PIN_SHIFTER_DATA, INPUT);
  pinMode(PIN_SHIFTER_X, INPUT);
  pinMode(PIN_SHIFTER_Y, INPUT);

  pinMode(PIN_PEDALS_ACCELERATOR, INPUT);
  pinMode(PIN_PEDALS_BRAKE, INPUT);
  pinMode(PIN_PEDALS_CLUTCH, INPUT);

  pinMode(PIN_HANDBRAKE, INPUT);

#ifdef USE_SHIFTER
  digitalWrite(PIN_SHIFTER_MODE, HIGH);
  digitalWrite(PIN_SHIFTER_CLOCK, HIGH);
#endif

  static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
  HID().AppendDescriptor(&node);
}

void debug()
{
  Serial.println("=============== DEBUG ===============");
#ifdef USE_SHIFTER
  Serial.print("Shifter{ ");
  Serial.print("X: ");
  Serial.print(shifterValueX);
  Serial.print(" Y: ");
  Serial.print(shifterValueY);
  Serial.print(" Gear: ");
  Serial.print(shifterCurrentGear);
  Serial.println(" }");
  Serial.print("Buttons{ ");
  for (int i = 0; i < 16; ++i)
  {
    Serial.print(i);
    Serial.print(":");
    Serial.print(shifterValueData[i] == 1 ? "[X]" : "[ ]");
    Serial.print(" ");
  }
  Serial.println(" } ");
#endif

#ifdef USE_PEDALS
  Serial.print("Pedals{ ");
  Serial.print("Accelerator: ");
  Serial.print(outputAccelerator);
  Serial.print(" (");
  Serial.print(pedalsValueAccelerator);
  Serial.print(") ");
  Serial.print(" Brake: ");
  Serial.print(outputBrake);
  Serial.print(" (");
  Serial.print(pedalsValueBrake);
  Serial.print(") ");
  Serial.print(" Clutch: ");
  Serial.print(outputClutch);
  Serial.print(" (");
  Serial.print(pedalsValueClutch);
  Serial.print(") ");
  Serial.println(" }");
#endif

#ifdef USE_HANDBRAKE
  Serial.print("Handbrake{ ");
  Serial.print(outputHandbrake);
  Serial.print(" (");
  Serial.print(handbrakeValue);
  Serial.print(") ");
  Serial.println(" }");
#endif
}

void loop()
{
#ifdef USE_SHIFTER
  loadShifter();
#endif
#ifdef USE_PEDALS
  loadPedals();
#endif
#ifdef USE_HANDBRAKE
  loadHandbrake();
#endif
  sendState();
}