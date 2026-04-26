echo ---DEVICES---
ls -l /dev/stm32 /dev/ttyUSB* /dev/ttyACM* /dev/serial/by-id 2>/dev/null || true
echo ---STTY---
stty -F /dev/stm32 115200 raw -echo -echoe -echok -echonl -icanon -isig -iexten -ixon -ixoff -icrnl -inlcr -igncr -opost 2>/dev/null || true
stty -F /dev/stm32 -a 2>/dev/null || true
echo ---RAW_HEX---
python3 - <<'PY'
import os, sys, time, select
path = "/dev/stm32"
try:
    fd = os.open(path, os.O_RDONLY | os.O_NONBLOCK)
except OSError as e:
    print(f"open_error: {e}")
    sys.exit(0)
deadline = time.time() + 3.0
data = bytearray()
while time.time() < deadline and len(data) < 128:
    r, _, _ = select.select([fd], [], [], 0.25)
    if not r:
        continue
    try:
        chunk = os.read(fd, 128 - len(data))
    except BlockingIOError:
        continue
    if not chunk:
        continue
    data.extend(chunk)
os.close(fd)
if not data:
    print("no_bytes")
else:
    print("bytes", len(data))
    print("hex", data.hex(" "))
PY
