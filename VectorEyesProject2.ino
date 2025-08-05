#include "engine/AnimationEngine.h"
#include "directors/Directors.h"
#include <U8g2lib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define LED_PIN 2 // Đèn LED tích hợp trên nhiều bo ESP32

// =================================================================
// BƯỚC 1: VIẾT HÀM CHO TASK MỚI
// =================================================================
void ledBlinkTask(void *pvParameters) {
  // Khởi tạo chân LED
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Task nháy đèn LED đã bắt đầu trên Core 0.");

  // Vòng lặp vô tận của task
  for (;;) {
    digitalWrite(LED_PIN, HIGH); // Bật đèn
    vTaskDelay(pdMS_TO_TICKS(500)); // Chờ 500ms

    digitalWrite(LED_PIN, LOW);  // Tắt đèn
    vTaskDelay(pdMS_TO_TICKS(500)); // Chờ 500ms
  }
}

// Hàm setup() được gọi một lần khi khởi động Arduino
// - Khởi tạo Serial để debug
// - Khởi tạo random seed cho hàm random()
// - Khởi tạo màn hình OLED
// - Khởi tạo engine animation và các "đạo diễn" (quản lý trạng thái cảm xúc, chớp mắt)
// - Tạo các FreeRTOS tasks
void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(A0));
    u8g2.begin();
    
    animation_engine_initialize(); // Khởi tạo engine animation (reset trạng thái animation)
    initialize_directors();       // Khởi tạo các biến trạng thái cho đạo diễn cảm xúc và chớp mắt

    // =================================================================
    // BƯỚC 2: KHỞI TẠO TASK MỚI TRONG SETUP()
    // =================================================================
    // Tạo luồng nháy đèn LED
    xTaskCreatePinnedToCore(
        ledBlinkTask,       // Tên hàm của task
        "LEDBlinkTask",     // Tên mô tả
        1024,               // Stack size là đủ cho task đơn giản này
        NULL,               // Không có tham số
        1,                  // Mức ưu tiên 1
        NULL,               // Không cần handle
        0);                 // Chạy trên Core 0
}

// Hàm loop() chạy lặp lại liên tục
// - Kiểm tra nếu đến lúc chớp mắt thì gọi animation_engine_start_blink()
// - Cập nhật trạng thái cảm xúc (có thể chuyển sang cảm xúc mới)
// - Cập nhật engine animation để vẽ frame mới lên màn hình
// =================================================================
// BƯỚC 3: HÀM LOOP() VẪN CHẠY BÌNH THƯỜNG
// =================================================================
void loop() {
    // 1. Xử lý chớp mắt (Ưu tiên cao nhất)
    if (blink_director_update()) {
        animation_engine_start_blink();
    }
    
    // 2. Cập nhật "Bộ não Cảm xúc"
    emotion_director_update();

    // 3. Cập nhật "Bộ não Hướng nhìn"
    gaze_director_update();

    // 4. Luôn luôn cập nhật Engine
    animation_engine_update();

    // Hàm loop() này không hề có lệnh delay nào,
    // nhưng đèn LED vẫn nháy đều đặn.
    // Điều này chứng tỏ task nháy đèn đang chạy độc lập.
}