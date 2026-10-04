#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>

int main() {
    cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);
    if (!cap.isOpened()){
        std::cerr << "No camera open";
        return -1;
    }

    cv::Mat frame;
    std::size_t frame_count = 0;

    // while (true) {
    //     if (cap.read(img)){
    //         cv::imshow("Image", img);
    //         cv::waitKey(20);
    //     }        
    // }
    
    const std::filesystem::path capture_dir{"captures"};
    std::filesystem::remove_all(capture_dir);
    std::filesystem::create_directories(capture_dir);

    std::this_thread::sleep_for(std::chrono::seconds(2));


    while (frame_count < 3) {
        cap >> frame; // grab next frame

        if (frame.empty()) {
            std::cout << "End of video stream\n";
            break;
        }

        

        // build filename with frame count
        std::string filename = std::string(capture_dir) + std::string("/frame")
                            + std::to_string(frame_count) + ".png";

        // true if successfully saved to disk
        bool is_saved = cv::imwrite(filename, frame);

        if (is_saved) {
            std::cout << "Saved " << filename << std::endl;
        } else {
            std::cerr << "Failed to save " << filename << std::endl;
        }

        frame_count++;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;

}