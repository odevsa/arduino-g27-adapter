void setup()
{
  pinMode(LED_BUILTIN_RX, OUTPUT);
}

void loop()
{
  digitalWrite(LED_BUILTIN_RX, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN_RX, LOW);
  delay(500);
}