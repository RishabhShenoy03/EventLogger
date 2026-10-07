#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>
#include "FaceProcessor.hpp"

FaceProcessor::FaceProcessor():
    detector_(cv::FaceDetectorYN::create(
        faceDet_model, "", cv::Size(1,1), 0.9f, 0.3f, 5000, 0, 0
    )),
    recognizer_(cv::FaceRecognizerSF::create(
        sface_path, ""
    )){}

cv::Mat FaceProcessor::detect(const cv::Mat& frame) {
    if (frame.empty()){
        return frame;
    }
    cv::Size inputSize(frame.cols, frame.rows);
    detector_->setInputSize(inputSize);

    cv::Mat faces; // detects from frame, saved into detection results
    detector_->detect(frame, faces);

    std::cout << "Faces detected: " << faces.rows << std::endl;
    return faces;
}

std::vector<cv::Mat> FaceProcessor::extract(const cv::Mat& frame, const cv::Mat& faces) {
    if (faces.empty()) {
        return std::vector<cv::Mat>{};
    }
    
    cv::Mat aligned_face;
    std::vector<cv::Mat> features{};
    features.reserve(faces.rows);

    for (int i = 0; i < faces.rows; i++) {
        recognizer_->alignCrop(frame, faces.row(i), aligned_face);

        cv::Mat feature;
        recognizer_->feature(aligned_face, feature);
        features.push_back(feature);
    }
    return features;
}
