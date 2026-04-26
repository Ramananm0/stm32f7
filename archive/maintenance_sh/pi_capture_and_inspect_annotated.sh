source /opt/ros/humble/setup.bash
source ~/raspberry_pi4b/ros2_ws/install/setup.bash
python3 ~/pi_save_annotated_frame.py
ls -l /tmp/annotated_frame.png
python3 - <<'PY'
from PIL import Image
img = Image.open('/tmp/annotated_frame.png')
print("size", img.size)
print("bbox", img.getbbox())
small = list(img.resize((8, 6)).getdata())
print("pixels", small)
PY
