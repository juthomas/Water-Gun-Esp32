#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <Button2.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define COMPRESSOR 25
#define VALVE 26
#define TRIGGER 39

#define TRESHOLD_COMPRESSOR 200

bool is_compressor_active = false;
// SDA => D2
// SCK => D1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int16_t rotary_count[3] = {400, 0, 1};
int8_t current_selection = 0;

Button2 encoder_button = Button2(33);
// Button2 trigger_button = Button2(39);

static IRAM_ATTR void enc_cb(void *arg)
{
  // ESP32Encoder* enc = (ESP32Encoder*) arg;
  static int old_count = 0;
  // int64_t count = ((ESP32Encoder*)arg)->getCount();

  switch (current_selection)
  {
  case 0:
  {
    if ((((ESP32Encoder *)arg)->getCount() - old_count > 0 && rotary_count[current_selection] < 2000) 
    || (((ESP32Encoder *)arg)->getCount() - old_count < 0 && rotary_count[current_selection] > 350)

    )
      rotary_count[current_selection] += (((ESP32Encoder *)arg)->getCount() - old_count) * 10;
  }
  break;

  case 1:
  {
    if ((((ESP32Encoder *)arg)->getCount() - old_count > 0 && rotary_count[current_selection] < 400) || (((ESP32Encoder *)arg)->getCount() - old_count < 0 && rotary_count[current_selection] > 0)

    )
      rotary_count[current_selection] += ((ESP32Encoder *)arg)->getCount() - old_count;
  }
  break;

  case 2:
  {
    if ((((ESP32Encoder *)arg)->getCount() - old_count > 0 && rotary_count[current_selection] < 1)
     || (((ESP32Encoder *)arg)->getCount() - old_count < 0 && rotary_count[current_selection] > 0)

    )
      rotary_count[current_selection] += ((ESP32Encoder *)arg)->getCount() - old_count;
  }
  break;

  default:
    break;
  }

  // rotary_count[current_selection] += old_count - ((ESP32Encoder *)arg)->getCount();

  old_count = ((ESP32Encoder *)arg)->getCount();

  // Serial.printf("enc cb: count: %d\n", enc->getCount());
  // static bool leds = false;
  // digitalWrite(LED_BUILTIN, (int)leds);
  // leds = !leds;
}

ESP32Encoder encoder(true, enc_cb);

void click(Button2 &btn)
{
  current_selection = (current_selection + 1) % 3;
}

void compressor_activation(uint16_t pressure)
{

  if (pressure < rotary_count[0])
  {
    digitalWrite(COMPRESSOR, LOW);
    is_compressor_active = true;
  }
  else if (is_compressor_active && pressure > rotary_count[0]  + TRESHOLD_COMPRESSOR)
  {
    digitalWrite(COMPRESSOR, HIGH);
    is_compressor_active = false;
  }
}

void fire_mods()
{
  if (rotary_count[1] == 0)
  {
    // continious
    if (digitalRead(TRIGGER))
    {
      digitalWrite(VALVE, LOW);
    }
    else
    {
      digitalWrite(VALVE, HIGH);
    }
  }
  else if (rotary_count[1] == 1)
  {

    // one tick
    static int last_time = 0;
    if (digitalRead(TRIGGER))
    {
      if (millis() - last_time < 300)
      {
        digitalWrite(VALVE, LOW);
      }
      else
      {
        digitalWrite(VALVE, HIGH);
      }
    }
    else
    {
      digitalWrite(VALVE, HIGH);
      last_time = millis();
    }
  }
  else if (rotary_count[1] == 2)
  {
    // tree shots
    static int last_time = 0;
    if (digitalRead(TRIGGER))
    {
      if (millis() - last_time < 300 ||
          (millis() - last_time > 400 &&
           millis() - last_time < 700) ||
          (millis() - last_time > 800 &&
           millis() - last_time < 1100)

      )
      {
        digitalWrite(VALVE, LOW);
      }
      else
      {
        digitalWrite(VALVE, HIGH);
      }
    }
    else
    {
      digitalWrite(VALVE, HIGH);
      last_time = millis();
    }
  }
  else
  {
    // number = frequency

    static int last_time = 0;
    if (digitalRead(TRIGGER))
    {
      if ((millis() - last_time) % (500 - rotary_count[1]) < (500 - rotary_count[1]) * 0.5)

      {
        digitalWrite(VALVE, LOW);
      }
      else
      {
        digitalWrite(VALVE, HIGH);
      }
    }
    else
    {
      digitalWrite(VALVE, HIGH);
      last_time = millis();
    }
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  encoder.attachSingleEdge(35, 32);
  encoder.clearCount();
  encoder.setFilter(1023);

  encoder_button.setTapHandler(click);

  pinMode(COMPRESSOR, OUTPUT);
  pinMode(VALVE, OUTPUT);
  pinMode(TRIGGER, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
    digitalWrite(COMPRESSOR, HIGH);

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);
  // Clear the buffer
  display.clearDisplay();

  Serial.println("Hello world");
  display.println("Hello world");
  display.display();
}

void display_data(uint16_t pressure)
{
  display.clearDisplay();
  display.setCursor(0, 0); // Start at top-left corner
  display.printf("Pressure : %d\nEncoder 1 : %d\nEncoder 2 : %d\nEncoder 3 : %d\n", pressure, rotary_count[0], rotary_count[1], rotary_count[2]);
  display.display();
}

void display_hud(uint16_t pressure)
{
  display.clearDisplay();
  display.drawRoundRect(0, 5 + 49, 128, 10, 5, WHITE);

  display.fillRoundRect(4, 8 + 49, 120 * (pressure - 400) / 1700, 4, 4, WHITE);

  // TODO: make blink this bar when focused
  if (current_selection != 0 || millis() % 1000 < 900)
    display.fillRoundRect( 4 + 120 * (rotary_count[0] - 400) / 1700, 45, 5, 10, 4, WHITE);
  // TODO: draw differents fire mods

  // 128 - 30 => 98;

  display.setCursor(50, 0); // Start at top-left corner

  display.print("Mode");
  display.setCursor(40, 35); // Start at top-left corner

  display.print("Pressure");

  if (current_selection != 1 || millis() % 1000 < 900)
  {

    if (rotary_count[1] == 0)
    {
      display.fillRect(5, 20, 118, 3, WHITE);
    }
    else if (rotary_count[1] == 1)
    {
      display.fillRect(5, 20, 20, 3, WHITE);
    }
    else if (rotary_count[1] == 2)
    {
      display.fillRect(5, 20, 20, 3, WHITE);
      display.fillRect(30, 20, 20, 3, WHITE);
      display.fillRect(55, 20, 20, 3, WHITE);
    }
    else
    {
      int16_t step = 500 - rotary_count[1];
      for (int16_t i = 0; i < 118; i++)
      {
        if (i % (step / 15) < (step / 20))
        {
          display.fillRect(5 + i, 20, 1, 3, WHITE);
        }
      }
    }
  }
  display.display();
}

void loop()
{
  uint16_t pressure = analogRead(36);
  Serial.printf("Pressure : %d\r\nEncoder : %d", pressure, rotary_count[current_selection]);

  encoder_button.loop();
  compressor_activation(pressure);
  if (rotary_count[2] == 0)
    display_data(pressure);
  else
    display_hud(pressure);
  fire_mods();
  delay(10);

  // put your main code here, to run repeatedly:
}