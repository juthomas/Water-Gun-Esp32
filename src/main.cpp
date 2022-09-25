#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <Button2.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// SDA => D2
// SCK => D1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


int16_t rotary_count[3] = {0};
int8_t current_selection = 0;


Button2 encoder_button = Button2(33);


static IRAM_ATTR void enc_cb(void* arg) {
  // ESP32Encoder* enc = (ESP32Encoder*) arg;
  static int old_count = 0;
  // int64_t count = ((ESP32Encoder*)arg)->getCount();
  rotary_count[current_selection] += old_count - ((ESP32Encoder*)arg)->getCount();
  old_count = ((ESP32Encoder*)arg)->getCount();
  
  // Serial.printf("enc cb: count: %d\n", enc->getCount());
  // static bool leds = false;
  // digitalWrite(LED_BUILTIN, (int)leds);
  // leds = !leds;
}

ESP32Encoder encoder(true, enc_cb);


void click(Button2& btn) {
    current_selection = (current_selection + 1) % 3;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  encoder.attachSingleEdge(35, 32);
  encoder.clearCount();
  encoder.setFilter(1023);

  encoder_button.setTapHandler(click);



  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
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

void loop() {
  uint16_t pressure = analogRead(36);
  Serial.printf("Pressure : %d\r\nEncoder : %d", pressure, rotary_count[current_selection]);
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner
  display.printf("Pressure : %d\nEncoder 1 : %d\nEncoder 2 : %d\nEncoder 3 : %d\n", pressure, rotary_count[0], rotary_count[1], rotary_count[2]);
  display.display();
  encoder_button.loop();

  delay(10);

  // put your main code here, to run repeatedly:
}