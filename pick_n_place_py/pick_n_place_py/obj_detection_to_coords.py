#!/usr/bin/env python3
import rclpy
import cv2
import numpy as np

from rclpy.node import Node
from cv_bridge import CvBridge
from sensor_msgs.msg import Image
from geometry_msgs.msg import Point
from .util import get_limits

class ObjToCoordsNode(Node):
    def __init__(self):
        super().__init__("obj_to_coords")
        self.bridge_ = CvBridge()
        self.camera_sub_ = self.create_subscription(
            Image, "/camera/image", self.image_callback, 10)
        self.depth_img_sub_ = self.create_subscription(
            Image, "/camera/depth_image", self.depth_img_callback, 10)
        self.pixel_coords_pub = self.create_publisher(Point, "convert_coords", 10)
        
        self.depth_img = None
        self.yellow = [255, 0, 0]
        
    def depth_img_callback(self, msg: Image):
        self.depth_img = self.bridge_.imgmsg_to_cv2(msg, desired_encoding="passthrough")
        
    def image_callback(self, msg: Image):
        frame = self.bridge_.imgmsg_to_cv2(msg, desired_encoding="bgr8")

        hsvImage = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        
        lowerLimit, upperLimit = get_limits(color=self.yellow)
        
        mask = cv2.inRange(hsvImage, lowerLimit, upperLimit)

        blur = cv2.GaussianBlur(mask,(5, 5), cv2.BORDER_DEFAULT)
        
        _, thresh = cv2.threshold(blur, 200, 255, cv2.THRESH_BINARY)
        
        contours_, hierarchy_ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        for i in contours_:
            M = cv2.moments(i)
            if M['m00'] != 0:
                cx = int(M['m10']/M['m00'])
                cy = int(M['m01']/M['m00'])
                cv2.drawContours(frame, [i], -1, (0, 255, 0), 2)
                cv2.circle(frame, (cx, cy), 7, (0, 0, 255), -1)
                cv2.putText(frame, "center", (cx - 20, cy - 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 2)
                pix_coords = Point()
                pix_coords.x = float(cx)
                pix_coords.y = float(cy)
                if self.depth_img is None:
                    return
                pix_coords.z = float(self.depth_img[cy, cx])
                if np.isnan(pix_coords.z) or np.isinf(pix_coords.z) or pix_coords.z <= 0.0:
                    return
                self.pixel_coords_pub.publish(pix_coords)
        
        cv2.waitKey(1)


def main(args=None):
    rclpy.init(args=args)
    node = ObjToCoordsNode()
    rclpy.spin(node)
    rclpy.shutdown()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()