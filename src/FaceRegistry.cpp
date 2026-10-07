#include <unordered_map>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>
#include "FaceProcessor.hpp"
#include "FaceRegistry.hpp"

FaceRegistry::FaceRegistry() {
    enroll_persons();
}

void FaceRegistry::enroll_persons() {
    for (const auto& person_dir : std::filesystem::directory_iterator(enrol_imgs)) {
        if (!person_dir.is_directory()) {
            continue;
        }
        const auto person_path = person_dir.path();
        const std::string person_name = person_path.filename().string();

        for (const auto& imgentry : std::filesystem::directory_iterator(person_path)) {
            
            if (!imgentry.is_regular_file()){
                continue;
            }
            cv::Mat pic = cv::imread(imgentry.path().string());
            if (pic.empty()) {
                continue;
            }

            cv::Mat faces  = processor.detect(pic);
            if (faces.empty()) {
                continue;
            }

            std::vector<cv::Mat> features = processor.extract(pic, faces);
            if (features.empty()) {
                continue;
            }

            auto& enrolled_features = registry[person_name];
            enrolled_features.insert(
                enrolled_features.begin(),
                features.begin(),
                features.end()
            );
        }
    }
}