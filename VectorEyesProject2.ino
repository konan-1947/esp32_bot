// =================================================================
// CÁC THƯ VIỆN VÀ KHAI BÁO
// =================================================================

// Thư viện cho các chức năng cốt lõi
#include "engine/AnimationEngine.h"
#include "directors/Directors.h"
#include <U8g2lib.h>

// Thư viện cho đa nhiệm và WiFi
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "secrets.h"

// Khởi tạo đối tượng màn hình OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Khởi tạo đối tượng WiFiMulti
WiFiMulti wifiMulti;

// =================================================================
// HÀM HELPER ĐỂ TÌM EMOTION THEO TÊN
// =================================================================
const Emotion* find_emotion_by_name(const char* emotion_name) {
    for (int i = 0; i < EMOTION_COUNT; i++) {
        if (strcmp(emotions[i].name, emotion_name) == 0) {
            return &emotions[i];
        }
    }
    return nullptr; // Không tìm thấy
}


// =================================================================
// TASK CHO CORE 0: XỬ LÝ MẠNG VÀ LOGIC "NÃO"
// =================================================================
void networkAndBrainTask(void *pvParameters) {
  Serial.println("Task Mạng & Não bộ đã bắt đầu trên Core 0.");

  // --- Bước 1: Kết nối WiFi ---
  int num_known_wifis = sizeof(known_wifis) / sizeof(known_wifis[0]);
  for (int i = 0; i < num_known_wifis; i++) {
      wifiMulti.addAP(known_wifis[i][0], known_wifis[i][1]);
  }

  Serial.println("[Core 0] Đang quét và kết nối WiFi...");
  while (wifiMulti.run() != WL_CONNECTED) {
      vTaskDelay(pdMS_TO_TICKS(1000)); // Dùng vTaskDelay thay cho delay()
      Serial.print(".");
  }
  Serial.println("\n[Core 0] KẾT NỐI WIFI THÀNH CÔNG!");
  Serial.print("[Core 0] Đã kết nối tới mạng: ");
  Serial.println(WiFi.SSID());
  Serial.print("[Core 0] Địa chỉ IP của ESP32: ");
  Serial.println(WiFi.localIP());

  // =================================================================
  // --- THÊM MỚI: GỬI REQUEST KIỂM TRA ĐẾN SERVER ---
  // =================================================================
  Serial.println("\n[Core 0] Đang thử gửi request kiểm tra đến server...");
  
  HTTPClient http;
  String serverUrl = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/"; // Gửi đến trang chủ
  
  http.begin(serverUrl);
  int httpCode = http.GET(); // Gửi một request GET đơn giản

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.printf("[Core 0] Server đã phản hồi! Mã: %d\n", httpCode);
    Serial.println("[Core 0] Nội dung phản hồi:");
    Serial.println(payload);
  } else {
    Serial.printf("[Core 0] Lỗi kết nối đến server! Mã lỗi: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  // =================================================================
  // --- KẾT THÚC PHẦN THÊM MỚI ---
  // =================================================================

  // --- Bước 2: Vòng lặp chính của Core 0 ---
  // Vòng lặp này sẽ xử lý logic nặng và giao tiếp với server
  unsigned long lastHeartbeat = 0;
  unsigned long lastEmotionCheck = 0;
  
  for (;;) {
    unsigned long currentTime = millis();
    
    // Kiểm tra và duy trì kết nối WiFi
    if (wifiMulti.run() != WL_CONNECTED) {
        Serial.println("[Core 0] Mất kết nối WiFi! Đang thử kết nối lại...");
        // wifiMulti.run() sẽ tự động xử lý việc kết nối lại
    } else {
        // =================================================================
        // --- GỬI HEARTBEAT ĐỊNH KỲ (MỖI 5 GIÂY) ---
        // =================================================================
        if (currentTime - lastHeartbeat > 5000) {
            HTTPClient http;
            String heartbeatUrl = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/api/heartbeat";
            
            http.begin(heartbeatUrl);
            http.addHeader("Content-Type", "application/json");
            
            // Tạo JSON payload cho heartbeat
            String jsonPayload = "{\"status\":\"alive\",\"free_heap\":" + String(esp_get_free_heap_size()) + "}";
            
            int httpCode = http.POST(jsonPayload);
            if (httpCode > 0) {
                Serial.printf("[Core 0] Heartbeat gửi thành công! Mã: %d\n", httpCode);
            } else {
                Serial.printf("[Core 0] Lỗi gửi heartbeat: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
            
            lastHeartbeat = currentTime;
        }
        
        // =================================================================
        // --- KIỂM TRA LỆNH CẢM XÚC (MỖI 2 GIÂY) ---
        // =================================================================
        if (currentTime - lastEmotionCheck > 2000) {
            HTTPClient http;
            String emotionUrl = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/api/emotion/next";
            
            http.begin(emotionUrl);
            int httpCode = http.GET();
            
            if (httpCode == 200) {
                String payload = http.getString();
                Serial.println("[Core 0] Nhận được lệnh cảm xúc:");
                Serial.println(payload);
                
                // Parse JSON để lấy emotion
                if (payload.indexOf("\"emotion\"") != -1) {
                    // Tìm emotion trong JSON
                    int startPos = payload.indexOf("\"emotion\":\"") + 11;
                    int endPos = payload.indexOf("\"", startPos);
                    if (startPos > 10 && endPos > startPos) {
                        String emotion = payload.substring(startPos, endPos);
                        Serial.printf("[Core 0] Thực hiện cảm xúc: %s\n", emotion.c_str());
                        
                        // Thực hiện thay đổi cảm xúc
                        const Emotion* target_emotion = find_emotion_by_name(emotion.c_str());
                        if (target_emotion != nullptr) {
                            // Lấy emotion hiện tại (neutral làm mặc định)
                            const Emotion* current_emotion = &emotions[NEUTRAL_STATE_INDEX];
                            
                            // Thực hiện animation chuyển đổi cảm xúc
                            animation_engine_change_emotion(
                                current_emotion,    // Từ cảm xúc hiện tại
                                target_emotion,     // Đến cảm xúc mới
                                1.0f,               // Thời gian chuyển đổi: 1 giây
                                1.0f,               // Cường độ: 100%
                                EASE_IN_OUT_QUAD,   // Kiểu easing
                                3000                // Thời gian giữ cảm xúc: 3 giây
                            );
                            
                            Serial.printf("[Core 0] ✅ Đã thực hiện cảm xúc: %s\n", emotion.c_str());
                        } else {
                            Serial.printf("[Core 0] ❌ Không tìm thấy cảm xúc: %s\n", emotion.c_str());
                        }
                    }
                }
            } else if (httpCode == 404) {
                // Không có lệnh cảm xúc mới
                // Serial.println("[Core 0] Không có lệnh cảm xúc mới");
            } else {
                Serial.printf("[Core 0] Lỗi kiểm tra cảm xúc: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
            
            lastEmotionCheck = currentTime;
        }
    }

    // Tạm dừng task trong 100ms để không chiếm hết CPU của Core 0
    vTaskDelay(pdMS_TO_TICKS(100)); 
  }
}


// =================================================================
// HÀM SETUP() - CHẠY MỘT LẦN KHI KHỞI ĐỘNG
// =================================================================
void setup() {
    // Khởi tạo giao tiếp Serial để debug
    Serial.begin(115200);
    
    // Lấy hạt giống ngẫu nhiên từ phần cứng
    randomSeed(esp_random());
    
    // Khởi tạo màn hình OLED
    u8g2.begin();
    
    // Khởi tạo các module của mắt robot
    animation_engine_initialize();
    initialize_directors();
    
    // --- TẠO TASK MỚI CHO CORE 0 ---
    xTaskCreatePinnedToCore(
        networkAndBrainTask,   // Tên hàm của task
        "NetworkTask",         // Tên mô tả
        10000,                 // Kích thước Stack (cần lớn cho WiFi và sau này là HTTP)
        NULL,                  // Không có tham số
        1,                     // Mức ưu tiên 1
        NULL,                  // Không cần handle
        0);                    // Chạy trên Core 0
        
    Serial.println("Hàm setup() đã hoàn tất trên Core 1. Giao diện mắt đang chạy.");
}


// =================================================================
// HÀM LOOP() - CHẠY LẶP LẠI TRÊN CORE 1
// =================================================================
void loop() {
    // Core 1 bây giờ chỉ tập trung vào việc làm cho mắt hoạt động mượt mà.
    // Toàn bộ code về WiFi và micro đã được chuyển sang Core 0.

    // 1. "Bộ não Chớp mắt" quyết định khi nào cần chớp
    if (blink_director_update()) {
        animation_engine_start_blink();
    }
    
    // 2. "Bộ não Cảm xúc" quyết định cảm xúc tiếp theo
    emotion_director_update();

    // 3. "Bộ não Hướng nhìn" quyết định khi nào cần liếc mắt
    gaze_director_update();

    // 4. Engine luôn cập nhật và vẽ lại mắt lên màn hình
    animation_engine_update();

    // Không có delay, không có xử lý nặng.
    // Vòng lặp này chạy cực nhanh để đảm bảo animation không bị giật.
}