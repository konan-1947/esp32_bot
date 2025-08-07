#ifndef SECRETS_H
#define SECRETS_H

// --- Thông tin Server ---
const char* SERVER_IP = "192.168.1.28"; // Thay bằng IP thực tế của máy tính bạn
const int SERVER_PORT = 5000;

// --- Danh sách các mạng WiFi đã biết ---
const char* known_wifis[][2] = {
  { "Hồng Anh", "0915597079" },
  { "Thai Binh 2", "0915597079" }
};

#endif