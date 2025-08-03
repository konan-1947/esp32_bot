#include "animations.h"
#include "config.h"
#include <U8g2lib.h>
#include <string.h>

// Báo cho trình biên dịch biết rằng biến u8g2 được định nghĩa ở một file khác
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

// --- BIẾN TRẠNG THÁI CỦA ENGINE ---
bool is_transitioning = false;
unsigned long transition_start_time = 0;

const Point* start_shape_left;
const Point* start_shape_right;
const Point* end_shape_left;
const Point* end_shape_right;

Point current_left_eye[EYE_VERTEX_COUNT];
Point current_right_eye[EYE_VERTEX_COUNT];

// --- CÁC HÀM ---
int16_t lerp(int16_t start, int16_t end, float t) {
    return start + (int16_t)(t * (end - start));
}

void initialize_animation_engine() {
    // Copy dữ liệu vào RAM
    for (int i = 0; i < EYE_VERTEX_COUNT; i++) {
        current_left_eye[i].x = neutral_left_shape[i].x;
        current_left_eye[i].y = neutral_left_shape[i].y;
        current_right_eye[i].x = neutral_right_shape[i].x;
        current_right_eye[i].y = neutral_right_shape[i].y;
    }
}

void start_transition_to(const Point start_l[], const Point start_r[], const Point end_l[], const Point end_r[]) {
    if (is_transitioning) return;

    start_shape_left = start_l;
    start_shape_right = start_r;
    end_shape_left = end_l;
    end_shape_right = end_r;

    is_transitioning = true;
    transition_start_time = millis();
}

void update_animation_engine() {
    float progress = 1.0;

    if (is_transitioning) {
        unsigned long elapsed = millis() - transition_start_time;
        progress = (float)elapsed / TRANSITION_DURATION_MS;

        if (progress >= 1.0) {
            progress = 1.0;
            is_transitioning = false;
        }

        for (int i = 0; i < EYE_VERTEX_COUNT; i++) {
            // Đọc dữ liệu trực tiếp
            int16_t start_lx = start_shape_left[i].x;
            int16_t start_ly = start_shape_left[i].y;
            int16_t start_rx = start_shape_right[i].x;
            int16_t start_ry = start_shape_right[i].y;
            int16_t end_lx = end_shape_left[i].x;
            int16_t end_ly = end_shape_left[i].y;
            int16_t end_rx = end_shape_right[i].x;
            int16_t end_ry = end_shape_right[i].y;
            
            // Tính toán interpolation
            current_left_eye[i].x = lerp(start_lx, end_lx, progress);
            current_left_eye[i].y = lerp(start_ly, end_ly, progress);
            current_right_eye[i].x = lerp(start_rx, end_rx, progress);
            current_right_eye[i].y = lerp(start_ry, end_ry, progress);
        }
    }
    
    u8g2.clearBuffer();
    
    // Vẽ mắt trái
    for (int i = 0; i < EYE_VERTEX_COUNT; i++) {
        int next_i = (i + 1) % EYE_VERTEX_COUNT;
        u8g2.drawLine(current_left_eye[i].x, current_left_eye[i].y, 
                     current_left_eye[next_i].x, current_left_eye[next_i].y);
    }
    
    // Vẽ mắt phải
    for (int i = 0; i < EYE_VERTEX_COUNT; i++) {
        int next_i = (i + 1) % EYE_VERTEX_COUNT;
        u8g2.drawLine(current_right_eye[i].x, current_right_eye[i].y, 
                     current_right_eye[next_i].x, current_right_eye[next_i].y);
    }
    
    u8g2.sendBuffer();
}