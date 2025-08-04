import cv2
import numpy as np
import os
import argparse

# ==============================================================================
# PHẦN 1: CÁC HÀM TIỆN ÍCH
# ==============================================================================

def extract_eye_contours(png_path):
    image = cv2.imread(png_path)
    if image is None:
        print(f"  - Lỗi: Không thể đọc file ảnh '{png_path}'")
        return []
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    _, thresh = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY_INV)
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
    if len(contours) < 2:
        print(f"  - Cảnh báo: Tìm thấy ít hơn 2 hình dạng trong '{os.path.basename(png_path)}'. Bỏ qua file này.")
        return []
    # Chuyển contours thành list để có thể sort
    contours_list = list(contours)
    contours_list.sort(key=cv2.contourArea, reverse=True)
    eye_contours = contours_list[:2]
    eye_contours.sort(key=lambda c: cv2.boundingRect(c)[0])
    return [c.reshape(-1, 2) for c in eye_contours]

def add_points_to_shape(shape, num_points_to_add):
    new_shape = list(shape)
    for _ in range(num_points_to_add):
        longest_segment_index = -1; max_dist_sq = -1
        for i in range(len(new_shape)):
            p1, p2 = new_shape[i], new_shape[(i + 1) % len(new_shape)]
            dist_sq = np.sum((p1 - p2)**2)
            if dist_sq > max_dist_sq: max_dist_sq, longest_segment_index = dist_sq, i
        p1, p2 = new_shape[longest_segment_index], new_shape[(longest_segment_index + 1) % len(new_shape)]
        mid_point = (p1 + p2) // 2
        new_shape.insert(longest_segment_index + 1, mid_point)
    return np.array(new_shape, dtype=int)

# ==============================================================================
# PHẦN 2: CÁC HÀM TẠO FILE HEADER
# ==============================================================================

def generate_vector_shapes_header(input_dir, output_file):
    all_shapes_data = {}
    max_points = 0
    print("--- BƯỚC 1: Đọc và Vector hóa các file PNG từ thư mục eyes ---")
    if not os.path.isdir(input_dir):
        print(f"Lỗi: Thư mục nguồn '{input_dir}' không tồn tại."); return None
    for filename in sorted(os.listdir(input_dir)):
        if filename.lower().endswith(".png"):
            shape_name = os.path.splitext(filename)[0]
            png_path = os.path.join(input_dir, filename)
            eye_contours = extract_eye_contours(png_path)
            if len(eye_contours) == 2:
                all_shapes_data[shape_name] = {'left': eye_contours[0], 'right': eye_contours[1]}
                max_points = max(max_points, len(eye_contours[0]), len(eye_contours[1]))
                print(f"  - Đã xử lý '{filename}': Mắt trái ({len(eye_contours[0])} điểm), Mắt phải ({len(eye_contours[1])} điểm)")
    if not all_shapes_data:
        print("Không tìm thấy ảnh hợp lệ nào để xử lý."); return None
    print(f"\n>>> Số điểm tối đa tìm thấy (MAX_POINTS): {max_points}\n")
    print("--- BƯỚC 2: Chuẩn hóa số điểm cho tất cả các hình dạng ---")
    normalized_shapes = {}
    for name, data in all_shapes_data.items():
        left_shape, right_shape = data['left'], data['right']
        points_to_add_left = max_points - len(left_shape)
        if points_to_add_left > 0:
            left_shape = add_points_to_shape(left_shape, points_to_add_left)
            print(f"  - Đã thêm {points_to_add_left} điểm vào mắt trái của '{name}'")
        points_to_add_right = max_points - len(right_shape)
        if points_to_add_right > 0:
            right_shape = add_points_to_shape(right_shape, points_to_add_right)
            print(f"  - Đã thêm {points_to_add_right} điểm vào mắt phải của '{name}'")
        normalized_shapes[name] = {'left': left_shape, 'right': right_shape}
    print("\n--- BƯỚC 3: Tạo nội dung file vector_shapes.h ---")
    header_lines = ["// FILE NÀY ĐƯỢC TẠO TỰ ĐỘNG. KHÔNG CHỈNH SỬA BẰNG TAY.", "#pragma once", "#include <stdint.h>", "",
                    "struct Point {", "    int16_t x;", "    int16_t y;", "};\n", f"const int EYE_VERTEX_COUNT = {max_points};\n"]
    for name, data in sorted(normalized_shapes.items()):
        for side in ['left', 'right']:
            header_lines.append(f"// Trạng thái: {name.upper()} - Mắt {side.capitalize()}")
            header_lines.append(f"const Point {name}_{side}_shape[EYE_VERTEX_COUNT] = {{")
            points_str = ",\n".join([f"    {{{p[0]}, {p[1]}}}" for p in data[side]])
            header_lines.append(points_str); header_lines.append("};\n")
    output_dir = os.path.dirname(output_file)
    if not os.path.exists(output_dir): os.makedirs(output_dir)
    try:
        with open(output_file, "w", encoding="utf-8") as f: f.write("\n".join(header_lines))
        print(f">>> ĐÃ TẠO THÀNH CÔNG FILE '{output_file}'!")
        return list(sorted(normalized_shapes.keys()))
    except IOError as e:
        print(f"Lỗi: Không thể ghi file '{output_file}'. {e}"); return None

def generate_emotion_manager_header(all_shape_names, output_file):
    print("\n--- BƯỚC 4: Tạo file quản lý cảm xúc EmotionManager.h ---")
    header_lines = ["// FILE NÀY ĐƯỢC TẠO TỰ ĐỘNG. KHÔNG CHỈNH SỬA BẰNG TAY.", "#pragma once", "#include \"../generated/vector_shapes.h\"", "",
                    "struct Emotion {", "    const char* name;", "    const Point* left_shape;", "    const Point* right_shape;", "};\n"]
    num_emotions = len(all_shape_names)
    header_lines.append(f"const int EMOTION_COUNT = {num_emotions};")
    header_lines.append("const Emotion emotions[EMOTION_COUNT] = {")
    emotion_structs = [f"    {{\"{name}\", {name}_left_shape, {name}_right_shape}}" for name in all_shape_names]
    header_lines.append(",\n".join(emotion_structs)); header_lines.append("};")
    try:
        neutral_index = all_shape_names.index("neutral")
        header_lines.append(f"\nconst int NEUTRAL_STATE_INDEX = {neutral_index};")
    except ValueError:
        print("  - Cảnh báo: Không tìm thấy trạng thái 'neutral'. Trạng thái mặc định sẽ là 0.")
        header_lines.append("\nconst int NEUTRAL_STATE_INDEX = 0;")
    output_dir = os.path.dirname(output_file)
    if not os.path.exists(output_dir): os.makedirs(output_dir)
    try:
        with open(output_file, "w", encoding="utf-8") as f: f.write("\n".join(header_lines))
        print(f">>> ĐÃ TẠO THÀNH CÔNG FILE '{output_file}'!")
    except IOError as e:
        print(f"Lỗi: Không thể ghi file '{output_file}'. {e}")

# ==============================================================================
# PHẦN 3: ĐIỂM VÀO SCRIPT
# ==============================================================================

if __name__ == "__main__":
    default_input_dir = "assets/keyframes_png"
    default_output_file = os.path.join("generated", "vector_shapes.h")
    parser = argparse.ArgumentParser(description="Tự động hóa toàn bộ quá trình tạo file header từ thư mục ảnh PNG.")
    parser.add_argument("--input", default=default_input_dir, help=f"Thư mục chứa các file PNG nguồn (mặc định: {default_input_dir}).")
    parser.add_argument("--output", default=default_output_file, help=f"Đường dẫn file header chứa vector (mặc định: {default_output_file}).")
    args = parser.parse_args()
    
    all_shape_names = generate_vector_shapes_header(args.input, args.output)
    if all_shape_names:
        emotion_manager_output_file = os.path.join("directors", "EmotionManager.h")
        generate_emotion_manager_header(all_shape_names, emotion_manager_output_file)   