#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>
#include "FaceProcessor.hpp"

class FaceProcessor {
    cv::Ptr<cv::FaceDetectorYN> detector_;
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;
    const std::string faceDet_model = "models/face_detection_yunet_2023mar.onnx";
    const std::string sface_path = "models/face_recognition_sface_2021dec.onnx";

public:

    FaceProcessor():
        detector_(cv::FaceDetectorYN::create(
            faceDet_model, "", cv::Size(1,1), 0.9f, 0.3f, 5000, 0, 0
        )),
        recognizer_(cv::FaceRecognizerSF::create(
            sface_path, ""
        )){}

    void detect(const cv::Mat& frame) {
        cv::Mat frame_orig = frame;
        if (frame.empty()){
            return;
        }
        cv::Size inputSize(frame.cols, frame.rows);
        detector_->setInputSize(inputSize);

        cv::Mat faces; // detects from frame, saved into detection results
        detector_->detect(frame, faces);

        std::cout << "Faces detected: " << faces.rows << std::endl;
    }
};