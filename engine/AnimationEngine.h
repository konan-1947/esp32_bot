#pragma once
#include "../directors/EmotionManager.h" // File do Python tạo, chứa danh sách Emotion

// Enum cho các quy luật chuyển động
enum EasingType {
    LINEAR,
    EASE_IN_OUT_QUAD
};

// Struct quản lý trạng thái của MỘT animation đang chạy
struct AnimationState {
    bool is_playing = false;
    bool is_paused = false;
    
    unsigned long start_time = 0;
    float duration_sec = 0.0;
    
    // Dùng con trỏ đến struct Emotion thay vì enum
    const Emotion* start_state = nullptr;
    const Emotion* end_state = nullptr;
    
    float intensity = 1.0;
    EasingType easing = LINEAR;
    
    float paused_progress = 0.0;
};

// Function declarations
void animation_engine_initialize();
bool animation_engine_is_busy();
void animation_engine_change_emotion(const Emotion* start, const Emotion* target, float duration, float intensity, EasingType easing);
void animation_engine_start_blink();
void animation_engine_update();