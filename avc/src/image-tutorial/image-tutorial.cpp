#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>

//To make image_callback show the images
//Get the bag file, put in in the catkin workspace.
//rosbag play circuitfiltered.bag
//rosrun rqt_gui rqt_gui &
// Add image views via the plugin menu bar
//rosrun avc image-tutorial

image_transport::Publisher img_pub;

void image_callback(const sensor_msgs::ImageConstPtr &msg) {
	//OpenCV stores messages as cv::Mat (Convert ROS to OpenCV)
	auto bridge_ptr = cv_bridge::toCvShare(msg, "bgr8"); //This must match the encoding on message

	cv::Mat image = bridge_ptr->image;

	//With this conversion we can just act like we are using openCV
	//cv::imshow("image", image);
	//cv::waitKey(10);


	// Stored images
	//Pixel is 3 number in a row, channels go in a row like this
	/*
	[ b1 g1 r1    b2 g2 r2 ...]
	*/
	//Do vision stuff here
	cv::Mat output{image.rows, image.cols, CV_8UC1};//output has same rows and cols as input image, the image size will be the same. We will still used 8 unsigned bits per channel but only have 1 channel (grayscale)
	for(int r = 0; r < image.rows; r++) {
		unsigned char* out_row = output.ptr<unsigned char>(r);
		unsigned char* row = image.ptr<unsigned char>(r); //get us a pointer to raw data for row at coordinate r
		for(int c = 0; c < image.cols * image.channels(); c += image.channels()) {
			auto b = row[c];
			auto g = row[c+1];
			auto r = row[c+2];

			//roughly detect red things
			if (r > 150 && b < 100 && g < 100) {
				// set output = 255
				out_row[c/3] = 255;
			} else {
				// set output = 0
				out_row[c/3] = 0;
			}

		}
	}

	//Build ROS message from opencv image (convert OpenCV to ROS)
	auto out_msg = cv_bridge::CvImage(msg->header, "mono8", output).toImageMsg();//header, encoding, output
	img_pub.publish(out_msg);
}

int main(int argc, char** argv) {

	//Make Node
	ros::init(argc, argv, "imagetutorial");
	//Grab Node Handle
	ros::NodeHandle nh;

	//Images take much more data and everything
	//Put node handle in constructor for ImageTransport
	image_transport::ImageTransport it{nh};

	//subscribe to topic
	auto img_sub = it.subscribe("/camera/image_rect", 1, image_callback);

	img_pub = it.advertise("/camera/image_filtered", 1);

	//Start event loop
	ros::spin();


	return 0;
}