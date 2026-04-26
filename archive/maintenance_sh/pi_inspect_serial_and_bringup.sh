echo ---UDEV_RULES---
sudo cat /etc/udev/rules.d/* 2>/dev/null || true
echo ---USB_SERIAL---
ls -l /dev/ttyUSB* 2>/dev/null || true
echo ---BY_ID---
ls -l /dev/serial/by-id 2>/dev/null || true
echo ---BY_PATH---
ls -l /dev/serial/by-path 2>/dev/null || true
echo ---UDEVADM0---
udevadm info -q all -n /dev/ttyUSB0 2>/dev/null | sed -n '1,120p' || true
echo ---UDEVADM1---
udevadm info -q all -n /dev/ttyUSB1 2>/dev/null | sed -n '1,120p' || true
echo ---BASHRC---
sed -n '1,220p' ~/.bashrc
echo ---MICROROS_LAUNCH---
sed -n '1,220p' ~/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/microros_agent.launch.py
echo ---BRINGUP---
sed -n '1,260p' ~/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/bringup.launch.py
