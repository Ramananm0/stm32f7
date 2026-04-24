set -e

cat <<'EOF' > /home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_odom/terrain_odom/base_status_node.py
#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32MultiArray, String


class BaseStatusNode(Node):
    def __init__(self):
        super().__init__('terrain_base_status')

        self.declare_parameter('status_topic', '/base_status')
        self.declare_parameter('status_text_topic', '/base_status_text')

        status_topic = self.get_parameter('status_topic').value
        status_text_topic = self.get_parameter('status_text_topic').value

        self.last_summary = None
        self.pub_text = self.create_publisher(String, status_text_topic, 10)
        self.create_subscription(Int32MultiArray, status_topic, self.on_status, 10)

        self.get_logger().info(f'base_status_node ready topic={status_topic}')

    def on_status(self, msg: Int32MultiArray):
        if len(msg.data) < 2:
            return

        esp32_ok = bool(msg.data[0])
        wheels_ok = bool(msg.data[1])
        imu_ok = bool(msg.data[2]) if len(msg.data) > 2 else False
        host_ok = bool(msg.data[3]) if len(msg.data) > 3 else False
        ros_ok = bool(msg.data[4]) if len(msg.data) > 4 else False

        summary = (
            f'ESP32={"CONNECTED" if esp32_ok else "NOT_CONNECTED"} '
            f'WHEELS={"CONNECTED" if wheels_ok else "NOT_CONNECTED"} '
            f'IMU={"CONNECTED" if imu_ok else "NOT_CONNECTED"} '
            f'ROS_CMD={"ACTIVE" if host_ok else "IDLE"} '
            f'MICROROS={"UP" if ros_ok else "DOWN"}'
        )

        if summary != self.last_summary:
            self.get_logger().info(summary)
            self.last_summary = summary

        out = String()
        out.data = summary
        self.pub_text.publish(out)


def main():
    rclpy.init()
    node = BaseStatusNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
EOF

python3 - <<'PY'
from pathlib import Path

setup_path = Path('/home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_odom/setup.py')
text = setup_path.read_text()
old = "        'console_scripts': [\n            'odom_node = terrain_odom.odom_node:main',\n        ],\n"
new = "        'console_scripts': [\n            'odom_node = terrain_odom.odom_node:main',\n            'base_status_node = terrain_odom.base_status_node:main',\n        ],\n"
if old in text and "base_status_node" not in text:
    text = text.replace(old, new)
setup_path.write_text(text)
PY

source /opt/ros/humble/setup.bash
cd /home/ubuntu/raspberry_pi4b/ros2_ws
colcon build --packages-select terrain_odom terrain_bringup --symlink-install
source /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash

pkill -f "micro_ros_agent serial --dev /dev/stm32 -b 115200" 2>/dev/null || true
pkill -f "ros2 launch terrain_bringup bringup.launch.py" 2>/dev/null || true
sleep 2

nohup ros2 launch terrain_bringup bringup.launch.py enable_base_status:=true >/tmp/terrain_bringup.log 2>&1 &
sleep 15

echo "=== PROCESSES ==="
ps -ef | grep -E "micro_ros_agent|terrain_|rplidar|ekf_node|slam_toolbox|ros2 launch terrain_bringup" | grep -v grep || true

echo "=== TOPICS ==="
ros2 topic list | grep -E "^/base_status$|^/base_status_text$|^/wheel_ticks$|^/wheel_velocity$|^/imu/data$" || true

echo "=== TICKS ONCE ==="
timeout 8s ros2 topic echo /wheel_ticks --once || true

echo "=== VELOCITY ONCE ==="
timeout 8s ros2 topic echo /wheel_velocity --once || true

echo "=== BASE STATUS ONCE ==="
timeout 8s ros2 topic echo /base_status --once || true

echo "=== LOG TAIL ==="
tail -n 80 /tmp/terrain_bringup.log || true
