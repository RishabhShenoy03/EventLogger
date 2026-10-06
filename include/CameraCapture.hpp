#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>

class CameraCapture {
    cv::VideoCapture cap_;

public:
    CameraCapture();
    cv::Mat capture();
};