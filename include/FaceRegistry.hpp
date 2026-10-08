#pragma once
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
#include "person_types.hpp"

struct FaceRegistry {
public:
    std::unordered_map<PersonID, std::vector<cv::Mat>> registry_;
    const std::filesystem::path enrolled_imgs = std::string("/home/rish/Desktop/projects/EventLogger/featuremap_images");
    FaceProcessor& processor_;
    
    FaceRegistry(FaceProcessor& processor);
    void enroll_persons();
    PersonID stringToPersonID(const std::string& person_name);
    std::string personIDToString(const PersonID& personid);
    std::optional<PersonMatch> match(const cv::Mat& feature);
    std::string id_to_payload(const PersonMatch& match_info);
};