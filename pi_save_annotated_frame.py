import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from PIL import Image as PILImage
import numpy as np


class Saver(Node):
    def __init__(self):
        super().__init__('annotated_frame_saver')
        self.create_subscription(Image, '/terrain/annotated_image', self.cb, 1)
        self.done = False

    def cb(self, msg):
        arr = np.frombuffer(msg.data, dtype=np.uint8).reshape((msg.height, msg.width, 3))
        PILImage.fromarray(arr, 'RGB').save('/tmp/annotated_frame.png')
        print(f"saved {msg.width}x{msg.height} {msg.encoding}")
        self.done = True


def main():
    rclpy.init()
    node = Saver()
    end = node.get_clock().now().nanoseconds + 20_000_000_000
    while rclpy.ok() and not node.done and node.get_clock().now().nanoseconds < end:
        rclpy.spin_once(node, timeout_sec=0.5)
    node.destroy_node()
    rclpy.shutdown()
    if not node.done:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
