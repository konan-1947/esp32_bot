#include "Directors.h"
#include "EmotionManager.h" // File do Python tạo
#include "../engine/AnimationEngine.h"
#include "../config.h"
#include <Arduino.h>

// --- BIẾN TRẠNG THÁI CỦA CÁC BỘ NÃO ---
unsigned long last_blink_time = 0;
unsigned long next_blink_interval = 0;

// "Trí nhớ" của bộ não bây giờ là một con trỏ đến struct Emotion
const Emotion* current_emotion = &emotions[NEUTRAL_STATE_INDEX];
unsigned long emotion_state_start_time = 0;
unsigned long current_emotion_dwell_time = 0;

void initialize_directors() {
    last_blink_time = millis();
    next_blink_interval = random(BLINK_INTERVAL_MIN, BLINK_INTERVAL_MAX);

    emotion_state_start_time = millis();
    current_emotion_dwell_time = random(NEUTRAL_DWELL_MIN, NEUTRAL_DWELL_MAX);
}

bool blink_director_update() {
    if (millis() - last_blink_time > next_blink_interval) {
        last_blink_time = millis();
        next_blink_interval = random(BLINK_INTERVAL_MIN, BLINK_INTERVAL_MAX);
        return true;
    }
    return false;
}

void emotion_director_update() {
    if (animation_engine_is_busy() || (millis() - emotion_state_start_time < current_emotion_dwell_time)) {
        return;
    }

    const Emotion* next_emotion = nullptr;
    const Emotion* start_emotion = current_emotion;

    if (strcmp(current_emotion->name, "neutral") != 0) {
        // Nếu đang có cảm xúc, 80% cơ hội quay về NEUTRAL
        if (random(10) < 8) {
            next_emotion = &emotions[NEUTRAL_STATE_INDEX];
        }
    } else {
        // Nếu đang NEUTRAL, 60% cơ hội có cảm xúc mới
        if (random(10) < 6) {
            int next_index;
            do {
                next_index = random(EMOTION_COUNT);
            } while (next_index == NEUTRAL_STATE_INDEX); // Đảm bảo không chọn lại neutral
            next_emotion = &emotions[next_index];
        }
    }

    if (next_emotion != nullptr) {
        float duration = random(50, 120) / 100.0f;
        animation_engine_change_emotion(start_emotion, next_emotion, duration, 1.0f, EASE_IN_OUT_QUAD);
        current_emotion = next_emotion;
    }
    
    emotion_state_start_time = millis();
    if (strcmp(current_emotion->name, "neutral") == 0) {
        current_emotion_dwell_time = random(NEUTRAL_DWELL_MIN, NEUTRAL_DWELL_MAX);
    } else {
        current_emotion_dwell_time = random(EMOTION_DWELL_MIN, EMOTION_DWELL_MAX);
    }
}