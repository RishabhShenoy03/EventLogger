#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <cstdlib>

int main() {
    cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);
    if (!cap.isOpened()){
        std::cerr << "No camera open";
        exit(1);
    }
    cv::Mat img;

    while (true){
        cap.read(img);
        cv::imshow("Image", img);
        cv::waitKey(20);
    }
}