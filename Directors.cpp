#include "directors/Directors.h"
#include "directors/EmotionManager.h" // File do Python tạo, chứa danh sách Emotion
#include "./engine/AnimationEngine.h"
#include "./config.h"
#include <Arduino.h>

// =========================================================================
// BIẾN TRẠNG THÁI NỘI BỘ CỦA CÁC "BỘ NÃO"
// =========================================================================

// --- Biến cho "Bộ não Chớp mắt" (Blink Director) ---
unsigned long last_blink_time = 0;
unsigned long next_blink_interval = 0;

// --- Biến cho "Bộ não Cảm xúc" (Emotion Director) ---
const Emotion* current_emotion = &emotions[NEUTRAL_STATE_INDEX]; // "Trí nhớ" về cảm xúc hiện tại
unsigned long emotion_state_start_time = 0;     // Thời điểm bắt đầu trạng thái hiện tại
unsigned long current_emotion_dwell_time = 0;   // Thời gian cần ở lại trong trạng thái này

// --- Biến cho "Bộ não Hướng nhìn" (Gaze Director) ---
unsigned long last_gaze_change_time = 0;
unsigned long next_gaze_interval = 0;


// =========================================================================
// CÁC HÀM CỦA "BỘ NÃO"
// =========================================================================

void initialize_directors() {
    // Khởi tạo Blink Director
    last_blink_time = millis();
    next_blink_interval = random(BLINK_INTERVAL_MIN, BLINK_INTERVAL_MAX);

    // Khởi tạo Emotion Director
    emotion_state_start_time = millis();
    current_emotion_dwell_time = random(NEUTRAL_DWELL_MIN, NEUTRAL_DWELL_MAX);

    // Khởi tạo Gaze Director
    last_gaze_change_time = millis();
    next_gaze_interval = random(4000, 8000); // Thay đổi hướng nhìn sau 4-8 giây
}

// --- BỘ NÃO CHỚP MẮT ---
// Quyết định KHI NÀO cần chớp mắt.
bool blink_director_update() {
    if (millis() - last_blink_time > next_blink_interval) {
        last_blink_time = millis();
        next_blink_interval = random(BLINK_INTERVAL_MIN, BLINK_INTERVAL_MAX);
        return true; // Trả về TRUE để báo cho main loop biết cần hành động
    }
    return false;
}

// --- BỘ NÃO CẢM XÚC ---
// Quyết định CẢM XÚC tiếp theo là gì.
void emotion_director_update() {
    // Điều kiện tiên quyết: Engine phải rảnh và đã hết thời gian tồn tại của cảm xúc cũ
    if (animation_engine_is_busy() || (millis() - emotion_state_start_time < current_emotion_dwell_time)) {
        return; // Chưa đến lúc, không làm gì cả
    }

    // Đã đến lúc ra quyết định mới
    const Emotion* next_emotion = nullptr;
    const Emotion* start_emotion = current_emotion;

    // Logic quyết định dựa trên trạng thái hiện tại
    if (strcmp(current_emotion->name, "neutral") != 0) {
        // Nếu đang có cảm xúc, 80% cơ hội quay về NEUTRAL
        if (random(10) < 8) {
            next_emotion = &emotions[NEUTRAL_STATE_INDEX];
        }
        // 20% còn lại là không làm gì, tiếp tục giữ cảm xúc cũ (bằng cách để next_emotion là nullptr)
    } else {
        // Nếu đang NEUTRAL, 60% cơ hội chuyển sang cảm xúc mới
        if (random(10) < 6) {
            int next_index;
            do {
                next_index = random(EMOTION_COUNT);
            } while (next_index == NEUTRAL_STATE_INDEX); // Đảm bảo không chọn lại neutral
            next_emotion = &emotions[next_index];
        }
        // 40% còn lại là không làm gì, tiếp tục ở trạng thái NEUTRAL
    }

    // Nếu đã có quyết định thay đổi, hãy ra lệnh cho engine
    if (next_emotion != nullptr) {
        // Tạo các tham số ngẫu nhiên cho animation
        float duration = random(30, 71) / 100.0f; // Thời gian chuyển đổi từ 0.3 đến 0.7 giây
        
        // Ra lệnh
        animation_engine_change_emotion(start_emotion, next_emotion, duration, 1.0f, EASE_IN_OUT_QUAD);
        
        // Cập nhật "trí nhớ" của bộ não
        current_emotion = next_emotion;
    }
    
    // Dù có thay đổi hay không, vẫn phải reset bộ đếm cho trạng thái (hiện tại hoặc mới)
    emotion_state_start_time = millis();
    // Quyết định thời gian tồn tại cho trạng thái tiếp theo
    if (strcmp(current_emotion->name, "neutral") == 0) {
        current_emotion_dwell_time = random(NEUTRAL_DWELL_MIN, NEUTRAL_DWELL_MAX);
    } else {
        current_emotion_dwell_time = random(EMOTION_DWELL_MIN, EMOTION_DWELL_MAX);
    }
}

// --- BỘ NÃO HƯỚNG NHÌN ---
// Quyết định HƯỚNG NHÌN tiếp theo là gì.
void gaze_director_update() {
    // Chỉ thay đổi hướng nhìn khi engine rảnh và đã đến lúc
    if (!animation_engine_is_busy() && (millis() - last_gaze_change_time > next_gaze_interval)) {
        int choice = random(10); // 0-9
        if (choice < 3) { // 30% cơ hội nhìn trái
            animation_engine_look_left();
        } else if (choice < 6) { // 30% cơ hội nhìn phải
            animation_engine_look_right();
        } else { // 40% cơ hội nhìn thẳng
            animation_engine_look_center();
        }

        // Đặt lại bộ đếm
        last_gaze_change_time = millis();
        next_gaze_interval = random(4000, 8000);
    }
}