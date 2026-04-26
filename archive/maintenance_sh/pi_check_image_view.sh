source /opt/ros/humble/setup.bash
source ~/raspberry_pi4b/ros2_ws/install/setup.bash
echo ---IMAGE_VIEW---
ros2 pkg executables image_view || true
echo ---RQT_IMAGE_VIEW---
ros2 pkg executables rqt_image_view || true
