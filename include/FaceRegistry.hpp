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

struct FaceRegistry {
public:
    std::unordered_map<std::string, std::vector<cv::Mat>> registry;
    std::filesystem::path enrol_imgs = std::string("/home/rish/Desktop/projects/EventLogger/featuremap_images");
    FaceProcessor processor{};
    
    FaceRegistry();
    void enroll_persons();
};