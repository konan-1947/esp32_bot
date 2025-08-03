#include <U8g2lib.h>
#include "config.h"
#include "animations.h"
#include "shapes.h"

// Khai báo đối tượng màn hình
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Biến quản lý trạng thái test
enum EyeState { NEUTRAL, ANGRY };
EyeState current_state = NEUTRAL;
unsigned long last_state_change_time = 0;

void setup() {
    // Khởi tạo Serial để gỡ lỗi nếu cần
    Serial.begin(115200);

    // Khởi tạo màn hình
    u8g2.begin();

    // Khởi tạo engine animation (vẽ frame đầu tiên)
    initialize_animation_engine();

    // Lưu thời điểm bắt đầu
    last_state_change_time = millis();
}

void loop() {
    // Cập nhật và vẽ frame hiện tại trong MỌI chu kỳ
    update_animation_engine();

    // Logic chuyển đổi trạng thái chỉ được kiểm tra khi không có transition nào đang chạy
    if (!is_transitioning && (millis() - last_state_change_time > PAUSE_DURATION_MS)) {
        if (current_state == NEUTRAL) {
            // Chuyển từ Neutral sang Angry
            start_transition_to(neutral_left_shape, neutral_right_shape, angry_left_shape, angry_right_shape);
            current_state = ANGRY;
        } else {
            // Chuyển từ Angry sang Neutral
            start_transition_to(angry_left_shape, angry_right_shape, neutral_left_shape, neutral_right_shape);
            current_state = NEUTRAL;
        }
        last_state_change_time = millis();
    }
}