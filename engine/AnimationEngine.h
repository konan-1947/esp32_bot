#pragma once
#include "../directors/EmotionManager.h" // File do Python tạo, chứa danh sách Emotion
#include "AnimationTypes.h" // Include để sử dụng EasingType và AnimationState

// Function declarations
void animation_engine_initialize();
bool animation_engine_is_busy();
void animation_engine_change_emotion(const Emotion* start, const Emotion* target, float duration, float intensity, EasingType easing);
void animation_engine_start_blink();
void animation_engine_update();
void animation_engine_look_left();
void animation_engine_look_right();
void animation_engine_look_center();