#pragma once
#include "shapes.h" // Cần include shapes.h để biết struct Point là gì

// Khai báo các hàm sẽ được định nghĩa trong animations.cpp
void initialize_animation_engine();
void update_animation_engine();
void start_transition_to(const Point start_l[], const Point start_r[], const Point end_l[], const Point end_r[]);

// Khai báo biến cờ is_transitioning để file .ino có thể truy cập
extern bool is_transitioning;