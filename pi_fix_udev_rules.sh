cat <<'EOF' | sudo tee /etc/udev/rules.d/99-stm32-usb.rules >/dev/null
# STM32 micro-ROS USB-TTL adapter (PL2303)
SUBSYSTEM=="tty", ATTRS{idVendor}=="067b", ATTRS{idProduct}=="2303", \
    SYMLINK+="stm32", MODE="0666", GROUP="dialout"
EOF

cat <<'EOF' | sudo tee /etc/udev/rules.d/99-rplidar.rules >/dev/null
# RPLidar USB adapter (CP2102)
SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", \
    SYMLINK+="rplidar", MODE="0666", GROUP="dialout"
EOF

sudo udevadm control --reload-rules
sudo udevadm trigger
sleep 2
echo ---RULES---
cat /etc/udev/rules.d/99-stm32-usb.rules
cat /etc/udev/rules.d/99-rplidar.rules
echo ---LINKS---
ls -l /dev/stm32 /dev/rplidar /dev/ttyUSB* 2>/dev/null || true
