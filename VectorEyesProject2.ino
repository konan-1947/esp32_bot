#include "engine/AnimationEngine.h"
#include "directors/Directors.h"
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
    
    Serial.begin(115200);
    randomSeed(analogRead(A0));
    u8g2.begin();
    
    animation_engine_initialize();
    initialize_directors();
}

void loop() {
    if (blink_director_update()) {
        animation_engine_start_blink();
    }
    
    emotion_director_update();

    animation_engine_update();
}