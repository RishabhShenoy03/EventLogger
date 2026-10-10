#include <iostream>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include "CameraCapture.hpp"

CameraCapture::CameraCapture(): cap_("/dev/video0", cv::CAP_V4L2) {
     if (!cap_.isOpened()) {
        throw std::runtime_error("Could not open camera");
    }

    cap_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, 1024);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, 768);

    std::cerr << "Camera mode: "
              << cap_.get(cv::CAP_PROP_FRAME_WIDTH) << "x"
              << cap_.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
}

cv::Mat CameraCapture::capture() {
    cv::Mat frame;
    cap_.read(frame);
    if (frame.empty()) {
        throw std::runtime_error("No picture captured");
        
    }
    return frame;
}