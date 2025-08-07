#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VectorEyes Server - Server Python cho ESP32 VectorEyes Project
Chạy trên máy tính để nhận kết nối từ ESP32
"""

from flask import Flask, request, jsonify, render_template_string
from zeroconf import ServiceInfo, Zeroconf
import json
import logging
from datetime import datetime
import threading
import time
import socket

# Cấu hình logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

app = Flask(__name__)

# Biến global để lưu trạng thái
server_state = {
    'connected_clients': [],
    'last_heartbeat': None,
    'emotion_requests': [],
    'is_running': True
}

# HTML template đơn giản cho trang chủ
HOME_TEMPLATE = """
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>VectorEyes Server</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 30px;
        }
        .status {
            background: #e8f5e8;
            border: 1px solid #4caf50;
            border-radius: 5px;
            padding: 15px;
            margin: 20px 0;
        }
        .client-info {
            background: #f0f8ff;
            border: 1px solid #2196f3;
            border-radius: 5px;
            padding: 15px;
            margin: 10px 0;
        }
        .log {
            background: #f9f9f9;
            border: 1px solid #ddd;
            border-radius: 5px;
            padding: 15px;
            margin: 10px 0;
            max-height: 300px;
            overflow-y: auto;
        }
        .button {
            background: #4caf50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            margin: 5px;
        }
        .button:hover {
            background: #45a049;
        }
        .button.danger {
            background: #f44336;
        }
        .button.danger:hover {
            background: #da190b;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🤖 VectorEyes Server</h1>
        
        <div class="status">
            <h3>📊 Trạng thái Server</h3>
            <p><strong>Trạng thái:</strong> <span style="color: green;">🟢 Đang chạy</span></p>
            <p><strong>Thời gian khởi động:</strong> {{ startup_time }}</p>
            <p><strong>Clients đã kết nối:</strong> {{ client_count }}</p>
            <p><strong>Heartbeat cuối:</strong> {{ last_heartbeat }}</p>
        </div>

        <div class="client-info">
            <h3>🔗 Thông tin Kết nối</h3>
            <p><strong>Địa chỉ IP:</strong> {{ server_ip }}</p>
            <p><strong>Cổng:</strong> {{ server_port }}</p>
            <p><strong>URL:</strong> <code>http://{{ server_ip }}:{{ server_port }}</code></p>
        </div>

        <div class="client-info">
            <h3>🎮 Điều khiển ESP32</h3>
            <p>Gửi lệnh thay đổi cảm xúc cho mắt robot:</p>
            <button class="button" onclick="sendEmotion('happy')">😊 Happy</button>
            <button class="button" onclick="sendEmotion('sad')">😢 Sad</button>
            <button class="button" onclick="sendEmotion('angry')">😠 Angry</button>
            <button class="button" onclick="sendEmotion('surprise')">😲 Surprise</button>
            <button class="button" onclick="sendEmotion('love')">🥰 Love</button>
            <button class="button" onclick="sendEmotion('neutral')">😐 Neutral</button>
            <button class="button" onclick="sendEmotion('sus')">🤨 Sus</button>
            <button class="button" onclick="sendEmotion('sus2')">😏 Sus2</button>
        </div>

        <div class="log">
            <h3>📝 Log Hoạt động</h3>
            <div id="log-content">
                {% for log in recent_logs %}
                <div style="margin: 5px 0; padding: 5px; border-bottom: 1px solid #eee;">
                    <small style="color: #666;">{{ log.timestamp }}</small><br>
                    <strong>{{ log.level }}</strong>: {{ log.message }}
                </div>
                {% endfor %}
            </div>
        </div>

        <div style="text-align: center; margin-top: 30px;">
            <button class="button danger" onclick="shutdownServer()">🛑 Tắt Server</button>
        </div>
    </div>

    <script>
        function sendEmotion(emotion) {
            fetch('/api/emotion', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({emotion: emotion})
            })
            .then(response => response.json())
            .then(data => {
                alert('Đã gửi lệnh: ' + emotion + '\\nPhản hồi: ' + data.message);
            })
            .catch(error => {
                alert('Lỗi: ' + error);
            });
        }

        function shutdownServer() {
            if (confirm('Bạn có chắc muốn tắt server?')) {
                fetch('/api/shutdown', {method: 'POST'})
                .then(() => {
                    alert('Server sẽ tắt trong 3 giây...');
                    setTimeout(() => window.close(), 3000);
                });
            }
        }

        // Auto-refresh log mỗi 5 giây
        setInterval(() => {
            fetch('/api/logs')
            .then(response => response.json())
            .then(data => {
                const logContent = document.getElementById('log-content');
                logContent.innerHTML = data.logs.map(log => 
                    `<div style="margin: 5px 0; padding: 5px; border-bottom: 1px solid #eee;">
                        <small style="color: #666;">${log.timestamp}</small><br>
                        <strong>${log.level}</strong>: ${log.message}
                    </div>`
                ).join('');
            });
        }, 5000);
    </script>
</body>
</html>
"""

# Lưu trữ logs gần đây
recent_logs = []
MAX_LOGS = 50

def add_log(level, message):
    """Thêm log vào danh sách"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    log_entry = {
        'timestamp': timestamp,
        'level': level,
        'message': message
    }
    recent_logs.append(log_entry)
    
    # Giữ chỉ MAX_LOGS entries gần nhất
    if len(recent_logs) > MAX_LOGS:
        recent_logs.pop(0)
    
    logger.info(f"[{level}] {message}")

@app.route('/')
def home():
    """Trang chủ - Dashboard"""
    import socket
    hostname = socket.gethostname()
    local_ip = socket.gethostbyname(hostname)
    
    return render_template_string(HOME_TEMPLATE, 
        startup_time=datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        client_count=len(server_state['connected_clients']),
        last_heartbeat=server_state['last_heartbeat'] or "Chưa có",
        server_ip=local_ip,
        server_port=5000,
        recent_logs=recent_logs[-10:]  # Chỉ hiển thị 10 log gần nhất
    )

@app.route('/api/status')
def api_status():
    """API endpoint để kiểm tra trạng thái server"""
    add_log("INFO", "ESP32 đã gọi API status")
    return jsonify({
        'status': 'running',
        'timestamp': datetime.now().isoformat(),
        'clients': len(server_state['connected_clients'])
    })

@app.route('/api/heartbeat', methods=['POST'])
def api_heartbeat():
    """API endpoint để ESP32 gửi heartbeat"""
    data = request.get_json() or {}
    client_ip = request.remote_addr
    
    if client_ip not in server_state['connected_clients']:
        server_state['connected_clients'].append(client_ip)
        add_log("INFO", f"ESP32 mới kết nối từ {client_ip}")
    
    server_state['last_heartbeat'] = datetime.now().strftime("%H:%M:%S")
    
    add_log("DEBUG", f"Heartbeat từ {client_ip}: {data}")
    
    return jsonify({
        'status': 'ok',
        'message': 'Heartbeat received',
        'timestamp': datetime.now().isoformat()
    })

@app.route('/api/emotion', methods=['POST'])
def api_emotion():
    """API endpoint để gửi lệnh thay đổi cảm xúc"""
    data = request.get_json()
    
    if not data or 'emotion' not in data:
        return jsonify({'error': 'Missing emotion parameter'}), 400
    
    emotion = data['emotion']
    valid_emotions = ['happy', 'sad', 'angry', 'surprise', 'love', 'neutral', 'sus', 'sus2']
    
    if emotion not in valid_emotions:
        return jsonify({'error': f'Invalid emotion. Valid emotions: {valid_emotions}'}), 400
    
    # Lưu request vào queue
    emotion_request = {
        'emotion': emotion,
        'timestamp': datetime.now().isoformat(),
        'client_ip': request.remote_addr
    }
    server_state['emotion_requests'].append(emotion_request)
    
    add_log("INFO", f"Lệnh thay đổi cảm xúc: {emotion} từ {request.remote_addr}")
    
    return jsonify({
        'status': 'ok',
        'message': f'Emotion {emotion} queued successfully',
        'emotion': emotion
    })

@app.route('/api/emotion/next', methods=['GET'])
def api_get_next_emotion():
    """API endpoint để ESP32 lấy lệnh cảm xúc tiếp theo"""
    if server_state['emotion_requests']:
        next_request = server_state['emotion_requests'].pop(0)
        add_log("INFO", f"ESP32 lấy lệnh cảm xúc: {next_request['emotion']}")
        return jsonify(next_request)
    else:
        return jsonify({'status': 'no_emotion'})

@app.route('/api/logs')
def api_logs():
    """API endpoint để lấy logs gần đây"""
    return jsonify({'logs': recent_logs})

@app.route('/api/shutdown', methods=['POST'])
def api_shutdown():
    """API endpoint để tắt server"""
    add_log("WARNING", "Server shutdown requested")
    
    def delayed_shutdown():
        time.sleep(3)
        server_state['is_running'] = False
        import os
        os._exit(0)
    
    threading.Thread(target=delayed_shutdown, daemon=True).start()
    return jsonify({'status': 'shutdown_scheduled'})

@app.errorhandler(404)
def not_found(error):
    """Xử lý lỗi 404"""
    add_log("WARNING", f"404 Not Found: {request.url}")
    return jsonify({
        'error': 'Not Found',
        'message': 'The requested URL was not found on the server.',
        'available_endpoints': [
            '/',
            '/api/status',
            '/api/heartbeat',
            '/api/emotion',
            '/api/emotion/next',
            '/api/logs'
        ]
    }), 404

def get_local_ip():
    """Lấy địa chỉ IP local của máy tính"""
    try:
        # Kết nối tạm thời để lấy IP local
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
        return local_ip
    except Exception:
        return "127.0.0.1"

def register_mdns_service():
    """Hàm chạy trong một luồng riêng để quảng bá dịch vụ mDNS."""
    ip_address = get_local_ip()
    port = 5000
    
    info = ServiceInfo(
        "_http._tcp.local.",  # Loại dịch vụ (web server)
        "Robot Brain Server._http._tcp.local.", # Tên dịch vụ
        addresses=[socket.inet_aton(ip_address)],
        port=port,
        properties={'path': '/'},
        server="robot-server.local." # Tên host quan trọng nhất
    )

    zeroconf = Zeroconf()
    add_log("INFO", f"Đang quảng bá dịch vụ mDNS 'robot-server.local' tại {ip_address}:{port}")
    print(f"🔍 mDNS: Đang quảng bá 'robot-server.local' tại {ip_address}:{port}")
    zeroconf.register_service(info)
    
    try:
        # Giữ cho luồng chạy để tiếp tục quảng bá
        while server_state['is_running']:
            time.sleep(0.1)
    finally:
        add_log("INFO", "Ngừng quảng bá dịch vụ mDNS")
        print("🔍 mDNS: Ngừng quảng bá dịch vụ")
        zeroconf.unregister_service(info)
        zeroconf.close()

if __name__ == '__main__':
    local_ip = get_local_ip()
    add_log("INFO", f"VectorEyes Server khởi động...")
    add_log("INFO", f"Server sẽ chạy tại: http://{local_ip}:5000")
    add_log("INFO", f"ESP32 có thể kết nối đến: {local_ip}:5000")
    
    print(f"\n{'='*60}")
    print(f"🤖 VectorEyes Server")
    print(f"{'='*60}")
    print(f"🌐 Địa chỉ: http://{local_ip}:5000")
    print(f"📱 ESP32 kết nối đến: {local_ip}:5000")
    print(f"🔍 mDNS: robot-server.local")
    print(f"⏰ Khởi động lúc: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"{'='*60}")
    print(f"💡 Mở trình duyệt và truy cập: http://{local_ip}:5000")
    print(f"🛑 Nhấn Ctrl+C để tắt server")
    print(f"{'='*60}\n")
    
    # Chạy mDNS trong một luồng riêng để không chặn Flask
    mdns_thread = threading.Thread(target=register_mdns_service)
    mdns_thread.daemon = True
    mdns_thread.start()
    
    try:
        app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)
    except KeyboardInterrupt:
        server_state['is_running'] = False
        add_log("INFO", "Server tắt bởi người dùng (Ctrl+C)")
        print("\n👋 Server đã tắt!")
    except Exception as e:
        server_state['is_running'] = False
        add_log("ERROR", f"Lỗi server: {str(e)}")
        print(f"\n❌ Lỗi: {e}") 