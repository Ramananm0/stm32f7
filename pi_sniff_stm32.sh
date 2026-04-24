python3 - <<'PY'
import os
import time
import termios

fd = os.open('/dev/stm32', os.O_RDONLY | os.O_NONBLOCK)
attrs = termios.tcgetattr(fd)
attrs[0] = 0
attrs[1] = 0
attrs[2] = attrs[2] | termios.CLOCAL | termios.CREAD
attrs[2] = attrs[2] & ~termios.PARENB & ~termios.CSTOPB & ~termios.CSIZE
attrs[2] = attrs[2] | termios.CS8
attrs[3] = 0
attrs[4] = termios.B115200
attrs[5] = termios.B115200
termios.tcsetattr(fd, termios.TCSANOW, attrs)

start = time.time()
buf = bytearray()
while time.time() - start < 5:
    try:
        chunk = os.read(fd, 256)
        if chunk:
            buf.extend(chunk)
            if len(buf) >= 64:
                break
    except BlockingIOError:
        time.sleep(0.05)

os.close(fd)
print(f"BYTES {len(buf)}")
print(buf[:64].hex())
PY
