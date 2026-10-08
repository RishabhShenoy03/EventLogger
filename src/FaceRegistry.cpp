#include <unordered_map>
#include <vector>
#include <filesystem>
#include <optional>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>

#include "FaceProcessor.hpp"
#include "FaceRegistry.hpp"
#include "person_types.hpp"

#define MATCH_THRESHOLD 0.5

FaceRegistry::FaceRegistry() {
    enroll_persons();
}

std::optional<PersonID> FaceRegistry::convert_personID(const std::string& person_name) {
    if (person_name == "Rishabh") {
        return PersonID::Rishabh;
    }
    if (person_name == "Daddy") {
        return PersonID:: Daddy;
    }
    if (person_name == "Mummy") {
        return PersonID::Mummy;
    }
    if (person_name == "Megan") {
        return PersonID::Megan;
    }
    if (person_name == "Shreya") {
        return PersonID::Shreya;
    }
    return std::nullopt;
}

void FaceRegistry::enroll_persons() {
    for (const auto& person_dir : std::filesystem::directory_iterator(enrolled_imgs)) {
        if (!person_dir.is_directory()) {
            continue;
        }
        const std::string person_name = person_dir.path().filename().string();
        const std::optional<PersonID> person_id = convert_personID(person_name);
        if (!person_id.has_value()) {
            continue;
        }

        for (const auto& imgentry : std::filesystem::directory_iterator(person_dir.path())) {
            
            if (!imgentry.is_regular_file()){
                continue;
            }
            cv::Mat pic = cv::imread(imgentry.path().string());
            if (pic.empty()) {
                continue;
            }

            cv::Mat faces  = processor.detect(pic);
            if (faces.rows != 1) { // rej more than 1 face detected
                continue;
            }

            std::vector<cv::Mat> features = processor.extract(pic, faces);
            if (features.size() != 1){ // rej >1 face feature extracted
                continue;
            }

            registry_[*person_id].push_back(features.front());
        }
    }
}

std::optional<PersonMatch> FaceRegistry::match(const cv::Mat& feature) {
    
    std::vector<double> match_scores;
    match_scores.resize(std::size_t(PersonID::COUNT), 0);

    for (const auto& pair: registry_) {
        for (const cv::Mat& feature_enrolled: pair.second) {
            double score = processor.similarity(feature, feature_enrolled);
            if (score > )
        }
    }
    return std::nullopt;
}
