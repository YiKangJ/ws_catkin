#include <ros/ros.h>
#include <fake_ar_publisher/ARMarker.h>
#include <myworkcell_core/LocalizePart.h>
#include <tf/transform_listener.h>

class Localizer
{
    public:
        Localizer(ros::NodeHandle& nh)
        {
            ar_sub_ = nh.subscribe<fake_ar_publisher::ARMarker>("ar_pose_marker", 1, &Localizer::visionCallback, this);

            server_ = nh.advertiseService("localize_part", &Localizer::localizePart, this);
        }
       
        bool localizePart(myworkcell_core::LocalizePart::Request& req, myworkcell_core::LocalizePart::Response& res)
        {

            fake_ar_publisher::ARMarkerConstPtr p = last_msg_;
            if (!p) return false;

            tf::Transform cam_to_target;
            tf::poseMsgToTF(p->pose.pose, cam_to_target);

            tf::StampedTransform req_to_cam;
            listener_.lookupTransform(req.base_frame, p->header.frame_id, ros::Time(0), req_to_cam);
           
            tf::Transform req_to_target;
            req_to_target = req_to_cam * cam_to_target;

            tf::poseTFToMsg(req_to_target, res.pose);

            //res.pose = p->pose.pose;
            return true;
        }

        void visionCallback(const fake_ar_publisher::ARMarkerConstPtr& msg)
        {
            last_msg_ = msg;
            //ROS_INFO_STREAM(last_msg_->pose.pose);
        }
    
        ros::Subscriber ar_sub_;
        fake_ar_publisher::ARMarkerConstPtr last_msg_;
        ros::ServiceServer server_;
        tf::TransformListener listener_;
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "vision_node");
    
    ros::NodeHandle nh;

    Localizer localizer(nh);

    ROS_INFO("Vision node starting.");

    ros::spin();
}
