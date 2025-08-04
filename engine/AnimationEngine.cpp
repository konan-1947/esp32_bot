#include "AnimationEngine.h"
#include "../config.h"
#include <U8g2lib.h>
#include <string.h>

// Khai báo biến u8g2 từ file .ino chính
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

// --- BIẾN TRẠNG THÁI CỦA ENGINE ---
AnimationState emotion_anim;
AnimationState blink_anim;

// Mảng RAM tạm thời để chứa tọa độ của frame sẽ được vẽ
Point current_left_eye[EYE_VERTEX_COUNT];
Point current_right_eye[EYE_VERTEX_COUNT];

// --- CÁC HÀM TIỆN ÍCH ---

// Nội suy tuyến tính
float linear_interpolate(float start, float end, float t) {
    return start + t * (end - start);
}

// Áp dụng quy luật chuyển động (easing)
float apply_easing(float t, EasingType easing) {
    if (easing == EASE_IN_OUT_QUAD) {
        if (t < 0.5) return 2 * t * t;
        return -1 + (4 - 2 * t) * t;
    }
    return t; // Mặc định là LINEAR
}

// Hàm nội bộ để tính toán và điền vào mảng current_..._eye
void calculate_current_frame(const AnimationState* anim, float progress) {
    const Point* start_l = anim->start_state->left_shape;
    const Point* start_r = anim->start_state->right_shape;
    const Point* end_l = anim->end_state->left_shape;
    const Point* end_r = anim->end_state->right_shape;

    for (int i = 0; i < EYE_VERTEX_COUNT; i++) {
        current_left_eye[i].x = (int16_t)linear_interpolate(start_l[i].x, end_l[i].x, progress);
        current_left_eye[i].y = (int16_t)linear_interpolate(start_l[i].y, end_l[i].y, progress);
        current_right_eye[i].x = (int16_t)linear_interpolate(start_r[i].x, end_r[i].x, progress);
        current_right_eye[i].y = (int16_t)linear_interpolate(start_r[i].y, end_r[i].y, progress);
    }
}

// --- CÁC HÀM CHÍNH CỦA ENGINE ---

void animation_engine_initialize() {
    // Lấy trạng thái neutral từ mảng emotions do Python tạo ra
    const Emotion* neutral_state = &emotions[NEUTRAL_STATE_INDEX];
    // Sao chép hình dạng ban đầu vào mảng vẽ
    memcpy(current_left_eye, neutral_state->left_shape, EYE_VERTEX_COUNT * sizeof(Point));
    memcpy(current_right_eye, neutral_state->right_shape, EYE_VERTEX_COUNT * sizeof(Point));
    
    // Thiết lập trạng thái cuối cùng của emotion_anim để các lệnh sau biết bắt đầu từ đâu
    emotion_anim.end_state = neutral_state;
}

bool animation_engine_is_busy() {
    return emotion_anim.is_playing || blink_anim.is_playing;
}

void animation_engine_change_emotion(const Emotion* start, const Emotion* target, float duration, float intensity, EasingType easing) {
    if (emotion_anim.is_playing) return; // Chỉ nhận lệnh mới khi rảnh

    emotion_anim.is_playing = true;
    emotion_anim.is_paused = false;
    emotion_anim.start_time = millis();
    emotion_anim.duration_sec = duration;
    emotion_anim.start_state = start;
    emotion_anim.end_state = target;
    emotion_anim.intensity = intensity;
    emotion_anim.easing = easing;
}

void animation_engine_start_blink() {
    if (blink_anim.is_playing) return; // Tránh chớp mắt chồng chéo

    // Tìm trạng thái BLINK trong mảng emotions
    const Emotion* blink_state = nullptr;
    for(int i = 0; i < EMOTION_COUNT; i++) {
        if (strcmp(emotions[i].name, "blink") == 0) {
            blink_state = &emotions[i];
            break;
        }
    }
    if (blink_state == nullptr) return; // Không tìm thấy hình dạng blink, không làm gì cả

    // Tạm dừng animation cảm xúc hiện tại
    if (emotion_anim.is_playing) {
        emotion_anim.is_paused = true;
        unsigned long elapsed = millis() - emotion_anim.start_time;
        emotion_anim.paused_progress = (float)elapsed / (emotion_anim.duration_sec * 1000.0f);
    }

    // Bắt đầu animation chớp mắt
    blink_anim.is_playing = true;
    blink_anim.is_paused = false;
    blink_anim.start_time = millis();
    blink_anim.duration_sec = 0.25f; // Chớp mắt nhanh trong 0.25 giây
    blink_anim.start_state = emotion_anim.end_state; // Bắt đầu từ trạng thái mắt hiện tại
    blink_anim.end_state = blink_state;
    blink_anim.easing = EASE_IN_OUT_QUAD;
    blink_anim.intensity = 1.0;
}

void animation_engine_update() {
    // Ưu tiên xử lý chớp mắt
    if (blink_anim.is_playing) {
        unsigned long elapsed = millis() - blink_anim.start_time;
        float raw_progress = (float)elapsed / (blink_anim.duration_sec * 1000.0f);

        // Logic chớp mắt: nửa đầu đi xuống (nhắm mắt), nửa sau đi lên (mở mắt)
        if (raw_progress < 0.5) {
            // Đi từ trạng thái hiện tại -> BLINK
            float progress = raw_progress * 2.0f; // Chuyển progress từ 0->0.5 thành 0->1.0
            float eased_progress = apply_easing(progress, blink_anim.easing);
            calculate_current_frame(&blink_anim, eased_progress);
        } else {
            // Đi từ BLINK -> trạng thái hiện tại
            float progress = (raw_progress - 0.5f) * 2.0f; // Chuyển progress từ 0.5->1.0 thành 0->1.0
            float eased_progress = apply_easing(progress, blink_anim.easing);
            // Đảo ngược start và end để mở mắt
            const Emotion* temp_start = blink_anim.end_state;
            const Emotion* temp_end = blink_anim.start_state;
            // Tạo một anim tạm để tính toán
            AnimationState temp_anim = blink_anim;
            temp_anim.start_state = temp_start;
            temp_anim.end_state = temp_end;
            calculate_current_frame(&temp_anim, eased_progress);
        }

        if (raw_progress >= 1.0) {
            blink_anim.is_playing = false;
            // Tiếp tục animation cảm xúc nếu nó đã bị tạm dừng
            if (emotion_anim.is_paused) {
                emotion_anim.is_paused = false;
                // Tính lại start_time để tiếp tục từ điểm đã dừng
                unsigned long new_offset = emotion_anim.duration_sec * 1000.0f * emotion_anim.paused_progress;
                emotion_anim.start_time = millis() - new_offset;
            }
        }
    } 
    // Nếu không chớp mắt, xử lý animation cảm xúc
    else if (emotion_anim.is_playing && !emotion_anim.is_paused) {
        unsigned long elapsed = millis() - emotion_anim.start_time;
        float raw_progress = (float)elapsed / (emotion_anim.duration_sec * 1000.0f);

        if (raw_progress >= 1.0) {
            raw_progress = 1.0;
            emotion_anim.is_playing = false;
        }
        
        float eased_progress = apply_easing(raw_progress, emotion_anim.easing);
        float final_progress = eased_progress * emotion_anim.intensity;
        calculate_current_frame(&emotion_anim, final_progress);
    }

    // Luôn vẽ frame hiện tại lên màn hình
    u8g2.clearBuffer();
    for (int i = 1; i < EYE_VERTEX_COUNT - 1; i++) {
        u8g2.drawTriangle(current_left_eye[0].x, current_left_eye[0].y, current_left_eye[i].x, current_left_eye[i].y, current_left_eye[i+1].x, current_left_eye[i+1].y);
        u8g2.drawTriangle(current_right_eye[0].x, current_right_eye[0].y, current_right_eye[i].x, current_right_eye[i].y, current_right_eye[i+1].x, current_right_eye[i+1].y);
    }
    u8g2.sendBuffer();
}