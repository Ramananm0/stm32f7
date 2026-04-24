source ~/.bashrc >/dev/null 2>&1 || true
echo ---PATH---
echo "$PATH"
echo ---WHICH_ROS2---
which ros2 || true
echo ---PYTHON---
python3 - <<'PY'
import sys
print(sys.executable)
print(sys.path)
try:
    import importlib.metadata as m
    print("ros2cli", m.version("ros2cli"))
except Exception as e:
    print("ros2cli_error", repr(e))
PY
echo ---EXECUTABLES---
timeout 5s ros2 pkg executables terrain_ai 2>/dev/null || true
