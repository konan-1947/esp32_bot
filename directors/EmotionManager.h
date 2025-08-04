// FILE NÀY ĐƯỢC TẠO TỰ ĐỘNG. KHÔNG CHỈNH SỬA BẰNG TAY.
#pragma once
#include "../generated/vector_shapes.h"

struct Emotion {
    const char* name;
    const Point* left_shape;
    const Point* right_shape;
};

const int EMOTION_COUNT = 4;
const Emotion emotions[EMOTION_COUNT] = {
    {"angry", angry_left_shape, angry_right_shape},
    {"blink", blink_left_shape, blink_right_shape},
    {"neutral", neutral_left_shape, neutral_right_shape},
    {"sad", sad_left_shape, sad_right_shape}
};

const int NEUTRAL_STATE_INDEX = 2;