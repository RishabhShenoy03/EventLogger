#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>

class FaceProcessor {
    cv::Ptr<cv::FaceDetectorYN> detector_;
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;
    const std::string faceDet_model = "models/face_detection_yunet_2023mar.onnx";
    const std::string sface_path = "models/face_recognition_sface_2021dec.onnx";

public:

    FaceProcessor();
    cv::Mat detect(const cv::Mat& frame);
    std::vector<cv::Mat> FaceProcessor::extract(const cv::Mat& img, const cv::Mat& faces);
};