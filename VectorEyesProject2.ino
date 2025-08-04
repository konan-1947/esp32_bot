#include "engine/AnimationEngine.h"
#include "directors/Directors.h"
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Hàm setup() được gọi một lần khi khởi động Arduino
// - Khởi tạo Serial để debug
// - Khởi tạo random seed cho hàm random()
// - Khởi tạo màn hình OLED
// - Khởi tạo engine animation và các "đạo diễn" (quản lý trạng thái cảm xúc, chớp mắt)
void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(A0));
    u8g2.begin();
    
    animation_engine_initialize(); // Khởi tạo engine animation (reset trạng thái animation)
    initialize_directors();       // Khởi tạo các biến trạng thái cho đạo diễn cảm xúc và chớp mắt
}

// Hàm loop() chạy lặp lại liên tục
// - Kiểm tra nếu đến lúc chớp mắt thì gọi animation_engine_start_blink()
// - Cập nhật trạng thái cảm xúc (có thể chuyển sang cảm xúc mới)
// - Cập nhật engine animation để vẽ frame mới lên màn hình
void loop() {
    // 1. Xử lý chớp mắt (Ưu tiên cao nhất)
    if (blink_director_update()) {
        animation_engine_start_blink();
    }
    
    // 2. Cập nhật "Bộ não Cảm xúc"
    emotion_director_update();

    // 3. MỚI: Cập nhật "Bộ não Hướng nhìn"
    gaze_director_update();

    // 4. Luôn luôn cập nhật Engine
    animation_engine_update();
}