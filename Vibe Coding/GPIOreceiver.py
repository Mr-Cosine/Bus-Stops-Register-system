import lgpio
import socket
import time
import os

CHIP = 4
PININFO_PATH = "./resources/PinInfo.txt"
SOCKET_PATH = "/tmp/gpio.sock"

# Load GPIO pins
def load_pins(path):
    pins = []
    try:
        with open(path) as f:
            for line in f:
                line = line.strip()
                if line.isdigit():
                    pins.append(int(line))
    except Exception as e:
        print("Error loading pin file:", e)
    return pins

BUTTON_PINS = load_pins(PININFO_PATH)

# -----------------------------
# Prepare socket
# -----------------------------
# Remove old socket file if exists
try:
    os.unlink(SOCKET_PATH)
except FileNotFoundError:
    pass

sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
sock.bind(SOCKET_PATH)

print("GPIO receiver running.")

# Open GPIO chip
h = lgpio.gpiochip_open(CHIP)

# Claim pins as input with pull-up
for pin in BUTTON_PINS:
    lgpio.gpio_claim_input(h, pin, lgpio.SET_PULL_UP)

# Store previous state for edge detection
prev_state = {pin: lgpio.gpio_read(h, pin) for pin in BUTTON_PINS}

# Main loop
while True:
    for pin in BUTTON_PINS:
        cur = lgpio.gpio_read(h, pin)

        # Detect falling edge (button press)
        if cur == 0 and prev_state[pin] == 1:
            event = f"BTN{pin}"
            sock.send(event.encode())
            print("Sent:", event)

        prev_state[pin] = cur

    time.sleep(0.01)
