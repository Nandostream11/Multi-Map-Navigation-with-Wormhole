#!/usr/bin/env python3
import rclpy
from rclpy.action import ActionClient
from rclpy.node import Node
from wormhole_action_interfaces.action import MultiMapNavigate
from geometry_msgs.msg import PoseStamped

class MultiMapNavClient(Node):
    def __init__(self):
        super().__init__('multi_map_nav_client')
        self.action_client = ActionClient(self, MultiMapNavigate, 'multi_map_navigate')
        
    def send_goal(self, target_map, x, y):
        goal_msg = MultiMapNavigate.Goal()
        goal_msg.target_map = target_map
        goal_msg.target_pose.header.frame_id = "map"
        goal_msg.target_pose.pose.position.x = x
        goal_msg.target_pose.pose.position.y = y
        goal_msg.target_pose.pose.orientation.w = 1.0
        
        self.action_client.wait_for_server()
        self.get_logger().info(f'Sending goal to map: {target_map} at ({x}, {y})')
        
        future = self.action_client.send_goal_async(goal_msg)
        future.add_done_callback(self.goal_response_callback)
        
    def goal_response_callback(self, future):
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.get_logger().info('Goal rejected')
            return
            
        self.get_logger().info('Goal accepted')
        result_future = goal_handle.get_result_async()
        result_future.add_done_callback(self.get_result_callback)
        
    def get_result_callback(self, future):
        result = future.result().result
        self.get_logger().info(f'Result: {result.success} - {result.message}')
        rclpy.shutdown()

def main():
    rclpy.init()
    client = MultiMapNavClient()
    
    # Test navigation to room2
    client.send_goal('room2', 3.0, 2.0)
    
    rclpy.spin(client)

if __name__ == '__main__':
    main()