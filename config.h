#pragma once

// ==============================================================================
// CÀI ĐẶT THỜI GIAN CHO "BỘ NÃO" (DIRECTORS)
// Các hằng số này quyết định "nhịp điệu" và "tính cách" của robot.
// ==============================================================================

// --- Thời gian TỒN TẠI ở trạng thái TRUNG TÍNH (NEUTRAL) ---
// Sau khi ở trạng thái NEUTRAL trong khoảng thời gian này, "Bộ não" sẽ xem xét
// việc chuyển sang một trạng thái cảm xúc mới.
const unsigned long NEUTRAL_DWELL_MIN = 5000; // Thời gian ở lại tối thiểu (5 giây)
const unsigned long NEUTRAL_DWELL_MAX = 10000; // Thời gian ở lại tối đa (10 giây)

// --- Thời gian TỒN TẠI ở các trạng thái CÓ CẢM XÚC (ANGRY, HAPPY, ...) ---
// Sau khi ở một trạng thái cảm xúc trong khoảng thời gian này, "Bộ não" sẽ
// xem xét việc quay trở về trạng thái NEUTRAL hoặc chuyển sang cảm xúc khác.
const unsigned long EMOTION_DWELL_MIN = 3000; // Thời gian ở lại tối thiểu (3 giây)
const unsigned long EMOTION_DWELL_MAX = 7000; // Thời gian ở lại tối đa (7 giây)

// --- Khoảng thời gian giữa hai lần CHỚP MẮT ---
// "Bộ não Chớp mắt" sẽ kích hoạt một lần chớp mắt ngẫu nhiên trong khoảng này.
// Đây là hành vi vô thức, diễn ra độc lập với các cảm xúc.
const unsigned long BLINK_INTERVAL_MIN = 2500; // Thời gian chờ tối thiểu (2.5 giây)
const unsigned long BLINK_INTERVAL_MAX = 7000; // Thời gian chờ tối đa (7 giây)


// ==============================================================================
// CÀI ĐẶT THỜI GIAN CHO "TRÁI TIM" (ANIMATION ENGINE)
// Các hằng số này quyết định tốc độ của bản thân các animation.
// ==============================================================================

// --- Thời gian CHUYỂN ĐỔI mặc định ---
// Được sử dụng khi "Bộ não" không chỉ định một thời gian cụ thể.
// Ví dụ: tốc độ của một phản xạ nhanh.
const float DEFAULT_TRANSITION_DURATION = 0.7f; // Tốc độ chuyển đổi mặc định (0.7 giây)

// --- Thời gian để quay về trạng thái TRUNG TÍNH ---
// Thường được thiết lập dài hơn để tạo cảm giác "bình tĩnh lại" một cách từ từ.
const float DEFAULT_RETURN_TO_NEUTRAL_DURATION = 1.5f; // Tốc độ quay về NEUTRAL (1.5 giây)