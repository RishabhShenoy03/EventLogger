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

FaceRegistry::FaceRegistry(FaceProcessor& processor)
    : processor_(processor)
{
    enroll_persons();
}

PersonID FaceRegistry::stringToPersonID(const std::string& person_name) {
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
    return PersonID::Unknown;
}

std::string FaceRegistry::personIDToString(const PersonID& personid) {
    switch (personid) {
        case PersonID::Rishabh  : return "Rishabh";
        case PersonID::Daddy    : return "Daddy";
        case PersonID::Megan    : return "Megan";
        case PersonID::Mummy    : return "Mummy";
        case PersonID::Shreya   : return "Shreya";
        default                 : return "Unknown Person";
    }
}

void FaceRegistry::enroll_persons() {
    for (const auto& person_dir : std::filesystem::directory_iterator(enrolled_imgs)) {
        if (!person_dir.is_directory()) {
            continue;
        }
        const std::string person_name = person_dir.path().filename().string();
        const PersonID person_id = stringToPersonID(person_name);
        if (person_id == PersonID::Unknown) {
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

            cv::Mat faces  = processor_.detect(pic);
            if (faces.rows != 1) { // rej more than 1 face detected
                continue;
            }

            std::vector<cv::Mat> features = processor_.extract(pic, faces);
            if (features.size() != 1){ // rej >1 face feature extracted
                continue;
            }

            registry_[person_id].push_back(features.front());
        }
    }
}

PersonMatch FaceRegistry::match(const cv::Mat& feature) {
    if (feature.empty()) {
        return PersonMatch{PersonID::Unknown, 0.0};
    }
    PersonID best_id = PersonID::Unknown;
    double best_similarity = -1;

    for (const auto& pair: registry_) {
        for (const cv::Mat& feature_enrolled: pair.second) {
            if (feature_enrolled.empty()) {
                continue;
            }
            double score = processor_.similarity(feature, feature_enrolled);
            if (score > best_similarity){
                best_id = pair.first;
                best_similarity = score;
            }
        }
    }
    if (best_similarity > MATCH_THRESHOLD) {
        return PersonMatch{best_id, best_similarity};
    }
    return PersonMatch{PersonID::Unknown, best_similarity};
}

std::string FaceRegistry::id_to_payload(const PersonMatch& match) {
    // if (match.id == PersonID::Unknown) {
    //     return "Unknown person has entered";
    // }
    return personIDToString(match.id) + " has entered ("
        + std::to_string(match.similarity) + ")";
}

