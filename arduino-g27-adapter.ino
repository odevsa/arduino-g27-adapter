#include <HID.h>

// Comment out the following lines to disable the respective input
#define USE_SHIFTER
#define USE_BUTTON_ARRAY
#define USE_PEDALS
#define USE_HANDBRAKE

// Pin definitions
#define PIN_SHIFTER_CLOCK 0
#define PIN_SHIFTER_DATA 1
#define PIN_SHIFTER_MODE 6
#define PIN_SHIFTER_X 8
#define PIN_SHIFTER_Y 9
#define PIN_PEDALS_ACCELERATOR 18
#define PIN_PEDALS_BRAKE 19
#define PIN_PEDALS_CLUTCH 20
#define PIN_HANDBRAKE 21
#define PIN_BUTTON_ARRAY_COLS \
  (int[4]) { 2, 3, 4, 5 }
#define PIN_BUTTON_ARRAY_ROWS \
  (int[4]) { 10, 14, 15, 16 }

// Shifter tresholds
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

// Joystick Report ID
#define JOYSTICK_REPORT_ID 0x03

#define SIGNAL_SETTLE_DELAY 10

typedef struct JoystickReport
{
#if defined(USE_SHIFTER) || defined(USE_BUTTON_ARRAY)
  uint32_t buttons;
#endif
#ifdef USE_PEDALS
  uint16_t accelerator;
  uint16_t brake;
  uint16_t clutch;
#endif
#ifdef USE_HANDBRAKE
  uint16_t handbrake;
#endif
};

typedef struct Axis
{
  int min = INFINITY;
  int max = -INFINITY;
  int value;
  uint16_t output;

  void setValue(int value)
  {
    min = min(min, value);
    max = max(max, value);
    output = map(value, min, max, 0, 1023);
  }
};

int buttonArrayRowSize = sizeof(PIN_BUTTON_ARRAY_ROWS) / sizeof(PIN_BUTTON_ARRAY_ROWS[0]);
int buttonArrayColSize = sizeof(PIN_BUTTON_ARRAY_COLS) / sizeof(PIN_BUTTON_ARRAY_COLS[0]);

JoystickReport joystickReport = {
#if defined(USE_SHIFTER) || defined(USE_BUTTON_ARRAY)
    0,
#endif
#ifdef USE_PEDALS
    0,
    0,
    0,
#endif
#ifdef USE_HANDBRAKE
    0,
#endif
};

static const uint8_t _hidReportDescriptor[] PROGMEM = {

    // Joystick
    0x05, 0x01,               // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,               // USAGE (Joystick)
    0xa1, 0x01,               // COLLECTION (Application)
    0x85, JOYSTICK_REPORT_ID, //   REPORT_ID (3)

#if defined(USE_SHIFTER) && defined(USE_BUTTON_ARRAY)
    // 31 Buttons (7 first is for gears, 16 last is for button array, G27 Shifter's D-Pad is disabled because HID Library is limited to only 32 buttons)
    0x05, 0x09, //   USAGE_PAGE (Button)
    0x19, 0x01, //      USAGE_MINIMUM (Button 1)
    0x29, 0x1f, //      USAGE_MAXIMUM (Button 31)
    0x15, 0x00, //      LOGICAL_MINIMUM (0)
    0x25, 0x01, //      LOGICAL_MAXIMUM (1)
    0x75, 0x01, //      REPORT_SIZE (1)
    0x95, 0x1f, //      REPORT_COUNT (31)
    0x55, 0x00, //      UNIT_EXPONENT (0)
    0x65, 0x00, //      UNIT (None)
    0x81, 0x02, //      INPUT (Data,Var,Abs)
    // Padding
    0x95, 0x01, //      REPORT_COUNT (1)
    0x75, 0x01, //      REPORT_SIZE (1)
    0x81, 0x03, //      INPUT (Cnst,Var,Abs)
#elif defined(USE_SHIFTER)
    // 19 Buttons (7 first is for gears)
    0x05, 0x09, //   USAGE_PAGE (Button)
    0x19, 0x01, //      USAGE_MINIMUM (Button 1)
    0x29, 0x13, //      USAGE_MAXIMUM (Button 19)
    0x15, 0x00, //      LOGICAL_MINIMUM (0)
    0x25, 0x01, //      LOGICAL_MAXIMUM (1)
    0x75, 0x01, //      REPORT_SIZE (1)
    0x95, 0x13, //      REPORT_COUNT (19)
    0x55, 0x00, //      UNIT_EXPONENT (0)
    0x65, 0x00, //      UNIT (None)
    0x81, 0x02, //      INPUT (Data,Var,Abs)
    // Padding
    0x95, 0x01, //      REPORT_COUNT (1)
    0x75, 0x0d, //      REPORT_SIZE (13)
    0x81, 0x03, //      INPUT (Cnst,Var,Abs)
#elif defined(USE_BUTTON_ARRAY)
    // 16 Buttons (Button Array)
    0x05, 0x09, //   USAGE_PAGE (Button)
    0x19, 0x01, //      USAGE_MINIMUM (Button 1)
    0x29, 0x10, //      USAGE_MAXIMUM (Button 16)
    0x15, 0x00, //      LOGICAL_MINIMUM (0)
    0x25, 0x01, //      LOGICAL_MAXIMUM (1)
    0x75, 0x01, //      REPORT_SIZE (1)
    0x95, 0x10, //      REPORT_COUNT (16)
    0x55, 0x00, //      UNIT_EXPONENT (0)
    0x65, 0x00, //      UNIT (None)
    0x81, 0x02, //      INPUT (Data,Var,Abs)
    // Padding
    0x95, 0x01, //      REPORT_COUNT (1)
    0x75, 0x10, //      REPORT_SIZE (16)
    0x81, 0x03, //      INPUT (Cnst,Var,Abs)
#endif

#if defined(USE_PEDALS) && defined(USE_HANDBRAKE)
    // 4 Axes (Accelerator, Brake, Clutch and Handbrake)
    0x05, 0x01,       //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,       //      USAGE (Accelerator)
    0x09, 0x31,       //      USAGE (Brake)
    0x09, 0x32,       //      USAGE (Clutch)
    0x09, 0x33,       //      USAGE (Handbrake)
    0x15, 0x00,       //      LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x03, //      LOGICAL_MAXIMUM (1023)
    0x75, 0x10,       //      REPORT_SIZE (16)
    0x95, 0x04,       //      REPORT_COUNT (4)
    0x81, 0x02,       //      INPUT (Data,Var,Abs)
#elif defined(USE_PEDALS)
    // 3 Axes (Accelerator, Brake, Clutch)
    0x05, 0x01,       //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,       //      USAGE (Accelerator)
    0x09, 0x31,       //      USAGE (Brake)
    0x09, 0x32,       //      USAGE (Clutch)
    0x15, 0x00,       //      LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x03, //      LOGICAL_MAXIMUM (1023)
    0x75, 0x10,       //      REPORT_SIZE (16)
    0x95, 0x03,       //      REPORT_COUNT (3)
    0x81, 0x02,       //      INPUT (Data,Var,Abs)
#elif defined(USE_HANDBRAKE)
    // 1 Axis (Handbrake)
    0x05, 0x01,       //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,       //      USAGE (Handbrake)
    0x15, 0x00,       //      LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x03, //      LOGICAL_MAXIMUM (1023)
    0x75, 0x10,       //      REPORT_SIZE (16)
    0x95, 0x01,       //      REPORT_COUNT (1)
    0x81, 0x02,       //      INPUT (Data,Var,Abs)
#endif
    0xc0 // END_COLLECTION
};

#if defined(USE_SHIFTER) || defined(USE_BUTTON_ARRAY)
uint32_t outputButtons = 0;
int buttonCurrentIndex = 0;

void pressButton(uint8_t button)
{
  bitSet(outputButtons, button);
}
void releaseButton(uint8_t button)
{
  bitClear(outputButtons, button);
}
void setButton(uint8_t button, bool pressed)
{
  pressed ? pressButton(button) : releaseButton(button);
}
#endif

#ifdef USE_SHIFTER
int shifterValueData[16];
int shifterValueX;
int shifterValueY;
int shifterCurrentGear;

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
    return (x > TRESHOLD_SHIFTER_X_RIGHT && y < TRESHOLD_SHIFTER_Y_DOWN) ? 7 : 0;

  if (x < TRESHOLD_SHIFTER_X_LEFT)
    return (y > TRESHOLD_SHIFTER_Y_UP) ? 1 : (y < TRESHOLD_SHIFTER_Y_DOWN) ? 2
                                                                           : 0;
  else if (x > TRESHOLD_SHIFTER_X_RIGHT)
    return (y > TRESHOLD_SHIFTER_Y_UP) ? 5 : (y < TRESHOLD_SHIFTER_Y_DOWN) ? 6
                                                                           : 0;
  else
    return (y > TRESHOLD_SHIFTER_Y_UP) ? 3 : (y < TRESHOLD_SHIFTER_Y_DOWN) ? 4
                                                                           : 0;

  return 0;
}

void loadShifter()
{
  getButtonStates(shifterValueData);
  shifterValueX = analogRead(PIN_SHIFTER_X);
  shifterValueY = analogRead(PIN_SHIFTER_Y);
  shifterCurrentGear = getCurrentGear(shifterValueX, shifterValueY, shifterValueData[SHIFTER_INDEX_REVERSE] == 1);

  if (shifterCurrentGear > 0)
    setButton(shifterCurrentGear - 1, true);

  buttonCurrentIndex = 7;

  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_BLACK_TOP] == HIGH);
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_BLACK_RIGHT] == HIGH);
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_BLACK_BOTTOM] == HIGH);
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_BLACK_LEFT] == HIGH);

#if !defined(USE_BUTTON_ARRAY)
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_DPAD_TOP] == HIGH);
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_DPAD_RIGHT] == HIGH);
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_DPAD_BOTTOM] == HIGH);
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_DPAD_LEFT] == HIGH);
#endif

  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_RED_1] == HIGH);
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_RED_2] == HIGH);
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_RED_3] == HIGH);
  setButton(buttonCurrentIndex++, shifterValueData[SHIFTER_INDEX_RED_4] == HIGH);
}
#endif

#ifdef USE_BUTTON_ARRAY
void loadButtonArray()
{
  for (int row = 0; row < buttonArrayRowSize; row++)
  {
    digitalWrite(PIN_BUTTON_ARRAY_ROWS[row], LOW);

    for (int col = 0; col < buttonArrayColSize; col++)
      setButton(buttonCurrentIndex++, digitalRead(PIN_BUTTON_ARRAY_COLS[col]) == LOW);

    digitalWrite(PIN_BUTTON_ARRAY_ROWS[row], HIGH);
  }
}
#endif

#ifdef USE_PEDALS
Axis pedalsAccelerator;
Axis pedalsBrake;
Axis pedalsClutch;
#endif

#ifdef USE_HANDBRAKE
Axis handbrake;
#endif

void sendState()
{
#if defined(USE_SHIFTER) || defined(USE_BUTTON_ARRAY)
  joystickReport.buttons = outputButtons;
#endif
#ifdef USE_PEDALS
  joystickReport.accelerator = pedalsAccelerator.output;
  joystickReport.brake = pedalsBrake.output;
  joystickReport.clutch = pedalsClutch.output;
#endif
#ifdef USE_HANDBRAKE
  joystickReport.handbrake = handbrake.output;
#endif

  HID().SendReport(JOYSTICK_REPORT_ID, &joystickReport, sizeof(joystickReport));
}

void setup()
{
  Serial.begin(38400);

  pinMode(PIN_SHIFTER_MODE, OUTPUT);
  pinMode(PIN_SHIFTER_CLOCK, OUTPUT);
  pinMode(PIN_SHIFTER_DATA, INPUT);
  pinMode(PIN_SHIFTER_X, INPUT);
  pinMode(PIN_SHIFTER_Y, INPUT);

  for (int i = 0; i < buttonArrayRowSize; i++)
  {
    pinMode(PIN_BUTTON_ARRAY_ROWS[i], OUTPUT);
    digitalWrite(PIN_BUTTON_ARRAY_ROWS[i], HIGH);
  }
  for (int i = 0; i < buttonArrayColSize; i++)
    pinMode(PIN_BUTTON_ARRAY_COLS[i], INPUT_PULLUP);

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

void loop()
{
#if defined(USE_SHIFTER) || defined(USE_BUTTON_ARRAY)
  buttonCurrentIndex = 0;
  outputButtons = 0;
#endif
#ifdef USE_SHIFTER
  loadShifter();
#endif
#ifdef USE_BUTTON_ARRAY
  loadButtonArray();
#endif
#ifdef USE_PEDALS
  pedalsAccelerator.setValue(analogRead(PIN_PEDALS_ACCELERATOR));
  pedalsBrake.setValue(analogRead(PIN_PEDALS_BRAKE));
  pedalsClutch.setValue(analogRead(PIN_PEDALS_CLUTCH));
#endif
#ifdef USE_HANDBRAKE
  handbrake.setValue(analogRead(PIN_HANDBRAKE));
#endif
  sendState();
}