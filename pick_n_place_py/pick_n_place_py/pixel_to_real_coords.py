#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from arm_interfaces.msg import ColorGoal


class PixelToRealNode(Node):
    def __init__(self):
        super().__init__("pixel_to_real")
        self.pixel_coords_sub = self.create_subscription(
            ColorGoal, "convert_coords", self.convert_coords_callback, 10)
        self.goal_pub_ = self.create_publisher(ColorGoal, "goal_point", 10)
        
        self.x_world = None
        self.y_world = None
        self.z_world = None
        
        self.cx_cam = 320.0
        self.cy_cam = 240.0
        self.fx = 528.4337565587049
        self.fy = 528.4337565587049
        self.camera_height = 3.0

    def convert_coords_callback(self, msg: ColorGoal):
        clr_name = msg.clr
        cx = msg.x
        cy = msg.y
        z = msg.z
        self.y_world = - (cx - self.cx_cam) * z / self.fx
        self.x_world = - (cy - self.cy_cam) * z / self.fy
        self.z_world = self.camera_height - z
        
        goal = ColorGoal()
        goal.clr = clr_name
        goal.x = self.x_world
        goal.y = self.y_world
        goal.z = self.z_world
        self.goal_pub_.publish(goal)

def main(args=None):
    rclpy.init(args=args)
    node = PixelToRealNode()
    rclpy.spin(node)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
