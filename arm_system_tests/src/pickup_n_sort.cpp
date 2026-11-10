#include "rclcpp/rclcpp.hpp"
#include "moveit/move_group_interface/move_group_interface.hpp"
#include "arm_interfaces/msg/color_goal.hpp"

using MoveGroupInterface = moveit::planning_interface::MoveGroupInterface;
using PoseTarget = arm_interfaces::msg::ColorGoal;
using namespace std::placeholders;

class Commander
{
public:
    Commander(std::shared_ptr<rclcpp::Node> node)
    {
        node_ = node;
        arm_ = std::make_shared<MoveGroupInterface>(node_, "arm");
        arm_->setMaxVelocityScalingFactor(1.0);
        arm_->setMaxAccelerationScalingFactor(1.0);
        gripper_ = std::make_shared<MoveGroupInterface>(node_, "gripper");

        goal_sub_ = node_->create_subscription<PoseTarget>(
            "goal_point", 1, std::bind(&Commander::PoseTargetCallback, this, _1));
    }

    void goToPoseTarget(double x, double y, double z, double roll, double pitch, double yaw)
    {

        tf2::Quaternion q;
        q.setRPY(roll, pitch, yaw);
        q.normalize();

        geometry_msgs::msg::PoseStamped target_pose;
        target_pose.header.frame_id = "base_link";
        target_pose.pose.position.x = x;
        target_pose.pose.position.y = y;
        target_pose.pose.position.z = z + 0.1;
        target_pose.pose.orientation.w = q.getW();
        target_pose.pose.orientation.x = q.getX();
        target_pose.pose.orientation.y = q.getY();
        target_pose.pose.orientation.z = q.getZ();

        arm_->setStartStateToCurrentState();
        arm_->setPoseTarget(target_pose);
        planAndExecute(arm_);

        hover_pose_ = target_pose.pose;
        
    }

    void cartesian_pick_up(double dz)
    {
        std::vector<geometry_msgs::msg::Pose> waypoints;

        hover_pose_.position.z -= dz;
        waypoints.push_back(hover_pose_);
        moveit_msgs::msg::RobotTrajectory trajectory;

        double fraction = arm_->computeCartesianPath(waypoints, 0.01, trajectory);

        if (fraction==1) {
            arm_->execute(trajectory);
        }
    }


    void open_gripper()
    {
        gripper_->setStartStateToCurrentState();
        gripper_->setNamedTarget("gripper_open");
        planAndExecute(gripper_);
        gripper_open_ = true;
    }

    void close_gripper()
    {
        gripper_->setStartStateToCurrentState();
        gripper_->setNamedTarget("gripper_closed");
        planAndExecute(gripper_);
        gripper_open_ = false;
    }

private:

    void planAndExecute(const std::shared_ptr<MoveGroupInterface> &interface)
    {
        MoveGroupInterface::Plan plan;
        bool success = (interface->plan(plan) == moveit::core::MoveItErrorCode::SUCCESS);
        
        if (success) {
            interface->execute(plan);
        }
    }

    void PoseTargetCallback(const PoseTarget &msg)
    {
        if (currently_executing_) {
            return;
        }

        if (msg.y < 0.0)
        {
            return;
        }

        currently_executing_ = true;

        goToPoseTarget(0.5, 0.8, 0.5, 3.14, 0.0, 1.57);

        goToPoseTarget(msg.x, msg.y, msg.z, 3.14, 0.0, 1.57);

        cartesian_pick_up(0.05);

        close_gripper();

        cartesian_pick_up(-0.5);

        if (msg.clr == "blue")
        {
            goToPoseTarget(0.2, -1.0, 0.5, 3.14, 0.0, 1.57);

            double place_z = 0.47 - stacked_cubes_blue_ * cube_height_;
            cartesian_pick_up(place_z);

            open_gripper();

            stacked_cubes_blue_ ++;

            cartesian_pick_up(-0.5);

            currently_executing_ = false;
        }

        else if (msg.clr == "green")
        {
            goToPoseTarget(0.0, -1.0, 0.5, 3.14, 0.0, 1.57);

            double place_z = 0.47 - stacked_cubes_green_ * cube_height_;
            cartesian_pick_up(place_z);

            open_gripper();
            
            stacked_cubes_green_ ++;

            cartesian_pick_up(-0.5);

            currently_executing_ = false;
        }

        else if (msg.clr == "red")
        {
            goToPoseTarget(-0.2, -1.0, 0.5, 3.14, 0.0, 1.57);

            double place_z = 0.47 - stacked_cubes_red_ * cube_height_;
            cartesian_pick_up(place_z);

            open_gripper();

            stacked_cubes_red_ ++;

            cartesian_pick_up(-0.5);

            currently_executing_ = false;
        }
    }

    std::shared_ptr<rclcpp::Node> node_;
    std::shared_ptr<MoveGroupInterface> arm_;
    std::shared_ptr<MoveGroupInterface> gripper_;

    rclcpp::Subscription<PoseTarget>::SharedPtr goal_sub_;
    geometry_msgs::msg::Pose hover_pose_;

    int stacked_cubes_blue_ = 0;
    int stacked_cubes_green_ = 0;
    int stacked_cubes_red_ = 0;
    double cube_height_ = 0.08;
    bool currently_executing_ = false;
    bool gripper_open_ = true;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("commander");
    node->set_parameter(rclcpp::Parameter("use_sim_time", true));
    auto commander = Commander(node);
    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}