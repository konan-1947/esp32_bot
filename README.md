# VectorEyes Project - Cơ Chế Cảm Xúc Robot

## Tổng Quan Hệ Thống

VectorEyes là một dự án robot với hệ thống mắt vector có khả năng biểu hiện cảm xúc tự nhiên thông qua các animation mượt mà. Hệ thống được thiết kế theo kiến trúc "Bộ não" (Directors) và "Trái tim" (Animation Engine) để tạo ra hành vi tự nhiên và đa dạng.

## Kiến Trúc Hệ Thống

### 1. Emotion Manager (Quản Lý Cảm Xúc)
- **Vị trí**: `directors/EmotionManager.h`
- **Chức năng**: Định nghĩa cấu trúc dữ liệu cho các cảm xúc
- **Cấu trúc Emotion**:
  ```cpp
  struct Emotion {
      const char* name;           // Tên cảm xúc
      const Point* left_shape;    // Hình dạng mắt trái
      const Point* right_shape;   // Hình dạng mắt phải
  };
  ```

### 2. Các Cảm Xúc Được Hỗ Trợ (có thể vẽ thêm và dùng tool python để tạo file vector_shapes.h)
- **Neutral**: Trạng thái trung tính (mặc định)
- **Happy**: Vui vẻ
- **Angry**: Giận dữ
- **Sad**: Buồn bã
- **Love**: Yêu thương
- **Surprise**: Ngạc nhiên
- **Sus/Sus2**: Nghi ngờ
- **Blink**: Chớp mắt

## Logic Cơ Chế Cảm Xúc

### 1. Emotion Director (Bộ Não Cảm Xúc)

#### Nguyên Tắc Hoạt Động
- **Trạng thái mặc định**: Robot luôn bắt đầu ở trạng thái NEUTRAL
- **Thời gian tồn tại**: Mỗi cảm xúc có thời gian tồn tại ngẫu nhiên
- **Quyết định chuyển đổi**: Dựa trên xác suất và trạng thái hiện tại

#### Logic Quyết Định

**Khi đang ở trạng thái NEUTRAL:**
- 60% cơ hội chuyển sang cảm xúc mới (ngẫu nhiên)
- 40% cơ hội tiếp tục ở trạng thái NEUTRAL
- Thời gian tồn tại: 5-10 giây

**Khi đang có cảm xúc:**
- 80% cơ hội quay về NEUTRAL
- 20% cơ hội tiếp tục giữ cảm xúc hiện tại
- Thời gian tồn tại: 3-7 giây

#### Cấu Hình Thời Gian
```cpp
// Thời gian ở trạng thái NEUTRAL
const unsigned long NEUTRAL_DWELL_MIN = 5000;  // 5 giây
const unsigned long NEUTRAL_DWELL_MAX = 10000; // 10 giây

// Thời gian ở trạng thái có cảm xúc
const unsigned long EMOTION_DWELL_MIN = 3000;  // 3 giây
const unsigned long EMOTION_DWELL_MAX = 7000;  // 7 giây
```

### 2. Blink Director (Bộ Não Chớp Mắt)

#### Hoạt Động Độc Lập
- Chạy song song với Emotion Director
- Không bị ảnh hưởng bởi cảm xúc hiện tại
- Tạo cảm giác tự nhiên cho robot

#### Cấu Hình
```cpp
const unsigned long BLINK_INTERVAL_MIN = 2500; // 2.5 giây
const unsigned long BLINK_INTERVAL_MAX = 7000; // 7 giây
```

### 3. Gaze Director (Bộ Não Hướng Nhìn)

#### Các Trạng Thái
1. **GAZE_IDLE**: Mắt nhìn thẳng, chờ hành động
2. **GAZE_TRANSITION_TO_SIDE**: Đang liếc sang bên
3. **GAZE_DWELLING_AT_SIDE**: Dừng lại ở bên
4. **GAZE_TRANSITION_TO_CENTER**: Quay về trung tâm

#### Logic Hành Động
- 70% cơ hội liếc mắt khi ở trạng thái IDLE
- Thời gian liếc: 0.25 giây
- Thời gian dừng ở bên: 100-400ms
- Thời gian quay về: 0.3 giây
- Khoảng cách liếc: ±10 đơn vị

## Animation Engine (Trái Tim)

### Chức Năng
- Thực hiện các animation chuyển đổi mượt mà
- Sử dụng easing functions để tạo chuyển động tự nhiên
- Quản lý thời gian và tiến trình animation

### Các Loại Animation
1. **Emotion Transition**: Chuyển đổi giữa các cảm xúc
2. **Blink Animation**: Chớp mắt
3. **Gaze Transition**: Liếc mắt

### Easing Functions
- **LINEAR**: Chuyển động đều
- **EASE_IN_OUT_QUAD**: Chuyển động mượt mà với gia tốc và giảm tốc

## Tích Hợp Hệ Thống

### Main Loop
```cpp
void loop() {
    // Cập nhật các bộ não
    blink_director_update();
    emotion_director_update();
    gaze_director_update();
    
    // Cập nhật animation engine
    animation_engine_update();
    
    // Render màn hình
    render_current_state();
}
```

### Ưu Tiên Animation
1. **Blink**: Ưu tiên cao nhất, có thể gián đoạn các animation khác
2. **Emotion Transition**: Ưu tiên trung bình
3. **Gaze Transition**: Ưu tiên thấp nhất, chỉ chạy khi không có animation khác

## Tùy Chỉnh Hành Vi

### Thay Đổi Tính Cách
- Điều chỉnh các hằng số trong `config.h`
- Thay đổi tỷ lệ xác suất trong Emotion Director
- Tùy chỉnh thời gian animation

### Thêm Cảm Xúc Mới
1. Tạo file PNG cho cảm xúc mới trong `assets/keyframes_png/`
2. Chạy script `tools/create_headers.py` để tạo vector shapes
3. Thêm cảm xúc vào mảng `emotions` trong `EmotionManager.h`

### Luồng Hoạt Động của Emotion Director

Hàm `emotion_director_update()` chịu trách nhiệm quyết định khi nào và cảm xúc nào sẽ được chuyển đổi cho robot. Dưới đây là mô tả chi tiết về luồng hoạt động của hàm này:

1. **Kiểm Tra Điều Kiện Chuyển Đổi**
   - Hàm chỉ thực hiện chuyển đổi cảm xúc khi:
     - Animation engine không bận (không có animation chuyển đổi nào đang chạy).
     - Đã hết thời gian "dwell" (thời gian ở lại) của cảm xúc hiện tại.

2. **Ra Quyết Định Cảm Xúc Tiếp Theo**
   - Nếu robot đang ở trạng thái cảm xúc (không phải "neutral"):
     - 80% khả năng sẽ quay về trạng thái "neutral".
     - 20% còn lại tiếp tục giữ cảm xúc hiện tại.
   - Nếu robot đang ở trạng thái "neutral":
     - 60% khả năng sẽ chuyển sang một cảm xúc mới (không phải "neutral", chọn ngẫu nhiên).
     - 40% còn lại tiếp tục ở trạng thái "neutral".

3. **Thực Hiện Chuyển Đổi (Nếu Có)**
   - Nếu quyết định chuyển sang cảm xúc mới:
     - Tạo các tham số ngẫu nhiên cho animation (ví dụ: thời gian chuyển đổi từ 0.3 đến 0.7 giây).
     - Gọi hàm `animation_engine_change_emotion()` để thực hiện animation chuyển đổi cảm xúc.

### Tham Số Của Hàm `animation_engine_change_emotion()`

Hàm này nhận vào 5 tham số để kiểm soát hoàn toàn quá trình chuyển đổi cảm xúc:

```cpp
void animation_engine_change_emotion(
    const Emotion* start,      // Cảm xúc bắt đầu
    const Emotion* target,     // Cảm xúc đích
    float duration,            // Thời gian chuyển đổi (giây)
    float intensity,           // Cường độ animation (0.0 - 1.0)
    EasingType easing,         // Kiểu chuyển động
    unsigned long dwell_time   // Thời gian tồn tại của cảm xúc (milliseconds)
);
```

#### Chi Tiết Các Tham Số:

1. **`start` (const Emotion*)**: 
   - Con trỏ đến struct Emotion của cảm xúc hiện tại
   - Chứa thông tin về hình dạng mắt trái và phải của trạng thái bắt đầu

2. **`target` (const Emotion*)**:
   - Con trỏ đến struct Emotion của cảm xúc đích
   - Chứa thông tin về hình dạng mắt trái và phải của trạng thái kết thúc

3. **`duration` (float)**:
   - Thời gian chuyển đổi tính bằng giây
   - Ví dụ: `0.5f` = 0.5 giây, `1.0f` = 1 giây
   - Trong Emotion Director: được tạo ngẫu nhiên từ 0.3 đến 0.7 giây

4. **`intensity` (float)**:
   - Cường độ của animation (từ 0.0 đến 1.0)
   - `1.0f` = cường độ đầy đủ
   - `0.5f` = cường độ một nửa
   - Trong Emotion Director: luôn được đặt là `1.0f`

5. **`easing` (EasingType)**:
   - Kiểu chuyển động của animation
   - Các giá trị có thể:
     - `LINEAR`: Chuyển động đều, không có gia tốc
     - `EASE_IN_OUT_QUAD`: Chuyển động mượt mà với gia tốc và giảm tốc
   - Trong Emotion Director: luôn sử dụng `EASE_IN_OUT_QUAD`

6. **`dwell_time` (unsigned long)**:
   - Thời gian tồn tại của cảm xúc tính bằng milliseconds
   - Được tính toán dựa trên loại cảm xúc:
     - Neutral: 5000-10000ms (5-10 giây)
     - Cảm xúc khác: 3000-7000ms (3-7 giây)
   - Được lưu trong AnimationState để Emotion Director sử dụng

#### Ví Dụ Sử Dụng:

```cpp
// Chuyển từ neutral sang happy trong 0.5 giây, tồn tại 4 giây
animation_engine_change_emotion(
    &emotions[NEUTRAL_STATE_INDEX],  // Bắt đầu từ neutral
    &emotions[HAPPY_STATE_INDEX],    // Kết thúc ở happy
    0.5f,                            // Thời gian chuyển đổi 0.5 giây
    1.0f,                            // Cường độ đầy đủ
    EASE_IN_OUT_QUAD,                // Chuyển động mượt mà
    4000                             // Tồn tại 4 giây (4000ms)
);
```

4. **Cập Nhật Trạng Thái Bộ Não**
   - Cập nhật lại "trí nhớ" về cảm xúc hiện tại.
   - Đặt lại thời điểm bắt đầu trạng thái mới.
   - Tính toán lại thời gian "dwell" cho trạng thái tiếp theo (tùy thuộc vào cảm xúc là "neutral" hay không).

