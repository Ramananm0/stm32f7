#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.qos import HistoryPolicy, QoSProfile, ReliabilityPolicy
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
        sensor_qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=10,
        )
        self.create_subscription(Int32MultiArray, status_topic, self.on_status, sensor_qos)

        self.get_logger().info(f'base_status_node ready topic={status_topic}')

    def on_status(self, msg: Int32MultiArray):
        if len(msg.data) < 2:
            return

        esp32_ok = bool(msg.data[0])
        wheels_ok = bool(msg.data[1])
        imu_ok = bool(msg.data[2]) if len(msg.data) > 2 else False
        host_ok = bool(msg.data[3]) if len(msg.data) > 3 else False
        microros_ok = bool(msg.data[4]) if len(msg.data) > 4 else False
        estop = bool(msg.data[5]) if len(msg.data) > 5 else False
        cmd_count = int(msg.data[6]) if len(msg.data) > 6 else 0

        summary = (
            f'ESP32={"CONNECTED" if esp32_ok else "NOT_CONNECTED"} '
            f'WHEELS={"CONNECTED" if wheels_ok else "NOT_CONNECTED"} '
            f'IMU={"CONNECTED" if imu_ok else "NOT_CONNECTED"} '
            f'ROS_CMD={"ACTIVE" if host_ok else "IDLE"} '
            f'MICROROS={"UP" if microros_ok else "DOWN"} '
            f'ESTOP={"ACTIVE" if estop else "CLEAR"} '
            f'CMD_COUNT={cmd_count}'
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
