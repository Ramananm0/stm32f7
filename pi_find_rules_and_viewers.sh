echo ---MATCHING_RULES---
grep -R -n -E 'stm32|rplidar|067b|2303|10c4|ea60' /etc/udev/rules.d /lib/udev/rules.d 2>/dev/null | sed -n '1,200p' || true
echo ---VIEWER_PACKAGES---
dpkg -l | grep -E 'ros-humble-image-view|ros-humble-rqt-image-view' || true
