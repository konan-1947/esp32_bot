#pragma once
#include "../generated/vector_shapes.h" // Dùng ../ để đi ngược ra thư mục gốc

enum EmotionState {
    NEUTRAL,
    HAPPY,
    ANGRY,
    SAD,
    BLINK, // Thêm trạng thái Blink
    NO_CHANGE
};

enum EasingType {
    LINEAR,
    EASE_IN_OUT_QUAD
};

struct AnimationState {
    bool is_playing = false;
    bool is_paused = false;
    
    unsigned long start_time = 0;
    float duration_sec = 0.0;
    
    EmotionState start_state = NEUTRAL;
    EmotionState end_state = NEUTRAL;
    
    float intensity = 1.0;
    EasingType easing = LINEAR;
    
    float paused_progress = 0.0;
};