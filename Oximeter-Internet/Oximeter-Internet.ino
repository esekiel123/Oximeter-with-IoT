#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <MAX30105.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;

#define MAX_BPM 220
#define MIN_BPM 40

unsigned long lastBeatTime;
float beatsPerMinute;
int beatCount;

void setup()
{
  Serial.begin(9600);
  Serial.println("Pulsometer");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("Could not find a valid MAX30105 sensor, check wiring!");
    while (1);
  }

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.display();
  delay(2000);

  byte ledBrightness = 60;
  byte sampleAverage = 4;
  byte ledMode = 3;
  int sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeIR(0);

  lastBeatTime = 0;
  beatsPerMinute = 0;
  beatCount = 0;
}

void loop()
{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    float bpm = calculateHeartRate();
    if (bpm < MIN_BPM || bpm > MAX_BPM) {
      // Invalid reading, ignore it.
      return;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Heart Rate: ");
    display.print(bpm);
    display.print(" BPM");
    display.display();
  }

  delay(10);
}

bool checkForBeat(long value)
{
  unsigned long time = millis();
  
  if (value > 500)
  {
    if (time - lastBeatTime >= 200)
    {
      beatCount++;
      lastBeatTime = time;
      return true;
    }
  }
  return false;
}

float calculateHeartRate()
{
  float timeInterval = (float)(millis() - lastBeatTime) / 1000.0; // Tiempo desde el último pulso en segundos.
  beatsPerMinute = (float)beatCount / (timeInterval / 60.0); // Cálculo de BPM basado en pulsos y tiempo transcurrido.
  beatCount = 0; // Reiniciar contador de pulsos para el siguiente cálculo.
  
  return beatsPerMinute;
}
