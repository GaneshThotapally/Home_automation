# Main_AllInOne_WS_FINAL.py
# OpenCV + MediaPipe finger detection -> WebSocket to ESP8266
# Index->relay1, Middle->relay2, Ring->relay3, Pinky->relay4
# Hand removed -> state stays (NO reset)

import cv2
import mediapipe as mp
import time
import threading
from collections import deque
from websocket import WebSocketApp

# -------- CONFIG --------
ESP_WS_URL = "ws://10.242.5.176:81"
DEBOUNCE_FRAMES = 5
CAM_W, CAM_H = 640, 480
PROCESS_W, PROCESS_H = 320, 240
REQUEST_FPS = 60
DRAW_LANDMARKS = True
# ------------------------

# -------- WebSocket Class --------
class ESPWebSocket:

    def __init__(self, url):
        self.url = url
        self.ws = None
        self.connected = threading.Event()
        self.lock = threading.Lock()
        self.thread = None
        self._stop = threading.Event()
        self._latest_state = None

    def start(self):
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def stop(self):
        self._stop.set()
        if self.ws:
            self.ws.close()
        self.connected.clear()

    def send(self, text):
        self._latest_state = text

        if not self.connected.is_set():
            return False

        try:
            with self.lock:
                self.ws.send(text)
            return True
        except:
            self.connected.clear()
            return False

    def _on_open(self, ws):
        print("[WS] Connected")
        self.connected.set()

        if self._latest_state:
            ws.send(self._latest_state)

    def _on_close(self, ws, code, msg):
        print("[WS] Disconnected")
        self.connected.clear()

    def _on_error(self, ws, err):
        print("[WS] Error:", err)
        self.connected.clear()

    def _on_message(self, ws, msg):
        pass

    def _run(self):

        while not self._stop.is_set():

            try:

                self.ws = WebSocketApp(
                    self.url,
                    on_open=self._on_open,
                    on_message=self._on_message,
                    on_error=self._on_error,
                    on_close=self._on_close
                )

                self.ws.run_forever(ping_interval=10)

            except:
                pass

            time.sleep(2)

# -------- MediaPipe --------
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=1,
    min_detection_confidence=0.6,
    min_tracking_confidence=0.6
)

mp_draw = mp.solutions.drawing_utils

# -------- Camera --------
cap = cv2.VideoCapture(0)

cap.set(cv2.CAP_PROP_FRAME_WIDTH, CAM_W)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, CAM_H)
cap.set(cv2.CAP_PROP_FPS, REQUEST_FPS)
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

# -------- Debounce --------
buffer = deque(maxlen=DEBOUNCE_FRAMES)
last_sent = None

# -------- WebSocket --------
esp = ESPWebSocket(ESP_WS_URL)
esp.start()

# -------- Functions --------
def tuple_to_state_str(t):
    return ''.join('1' if v else '0' for v in t)

def detect_fingers(hand_landmarks):

    def up(tip, pip):
        return 1 if hand_landmarks.landmark[tip].y < hand_landmarks.landmark[pip].y else 0

    index = up(8, 6)
    middle = up(12, 10)
    ring = up(16, 14)
    pinky = up(20, 18)

    return (index, middle, ring, pinky)

# -------- Main Loop --------
prev_time = 0

try:

    while True:

        ok, frame = cap.read()

        if not ok:
            break

        frame = cv2.flip(frame, 1)

        small = cv2.resize(frame, (PROCESS_W, PROCESS_H))
        img_rgb = cv2.cvtColor(small, cv2.COLOR_BGR2RGB)

        results = hands.process(img_rgb)

        finger_states = None

        if results.multi_hand_landmarks:

            hl = results.multi_hand_landmarks[0]

            if DRAW_LANDMARKS:
                mp_draw.draw_landmarks(frame, hl, mp_hands.HAND_CONNECTIONS)

            finger_states = detect_fingers(hl)

            buffer.append(finger_states)

        # ----- Debounce -----
        stable = None

        if len(buffer) == DEBOUNCE_FRAMES and len(set(buffer)) == 1:
            stable = buffer[-1]

        # ----- Send only new state -----
        if stable is not None and stable != last_sent:

            state_str = tuple_to_state_str(stable)

            esp.send(state_str)

            print("Sent:", state_str)

            last_sent = stable

        # ----- FPS -----
        now = time.time()
        fps = 1/(now-prev_time) if prev_time else 0
        prev_time = now

        # ----- Display -----
        if finger_states:
            status = f"I:{finger_states[0]} M:{finger_states[1]} R:{finger_states[2]} P:{finger_states[3]}"
        else:
            status = "No Hand"

        cv2.putText(frame, status, (10,60),
                    cv2.FONT_HERSHEY_SIMPLEX,0.9,(0,255,0),2)

        cv2.putText(frame, f"FPS:{int(fps)}", (10,100),
                    cv2.FONT_HERSHEY_SIMPLEX,0.9,(255,0,0),2)

        cv2.imshow("Hand Gesture Relay Control", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

finally:

    cap.release()
    cv2.destroyAllWindows()
    esp.stop()