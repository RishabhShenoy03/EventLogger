#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>

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
    
    const std::filesystem::path capture_dir{"captures"};
    std::filesystem::remove_all(capture_dir);
    std::filesystem::create_directories(capture_dir);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    while (frame_count < 20) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
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

    // initialise model path
    std::string faceDet_model = "models/face_detection_yunet_2023mar.onnx";
    std::filesystem::create_directories("outputs_facedet");

    std::string sface_path = "models/face_recognition_sface_2021dec.onnx";
    std::filesystem::create_directories("outputs_sface");

    for (std::size_t i = 0; i < frame_count; i++) {
        std::string img_path = std::string("captures/frame") + std::to_string(i)
                            + std::string(".png");
        cv::Mat img = cv::imread(img_path);                    
        cv::Size inputSize(img.cols, img.rows);
        
        cv::Ptr<cv::FaceDetectorYN> detector = cv::FaceDetectorYN::create(
            faceDet_model, "", inputSize, 0.9f, 0.3f, 5000, 0, 0
        );

        // 3. Set input size if image dimensions change
        detector->setInputSize(inputSize);

        // 4. Run inference
        cv::Mat faces;
        detector->detect(img, faces);

        // 5. Process results (each row in faces represents a face: x, y, w, h, confidence, landmarks...)
        std::cout << "Faces detected: " << faces.rows << std::endl;
        for (int j = 0; j < faces.rows; ++j) {
            int x = cvRound(faces.at<float>(j, 0));
            int y = cvRound(faces.at<float>(j, 1));
            int w = cvRound(faces.at<float>(j, 2));
            int h = cvRound(faces.at<float>(j, 3));
            
            cv::rectangle(img, cv::Rect(x, y, w, h), cv::Scalar(0, 255, 0), 2);
        }

        // Save or display output
        std::string output_path = std::string("outputs_facedet/frame") + std::to_string(i)
                                + std::string(".png");
        cv::imwrite(output_path, img);

        if (faces.rows > 0){
            cv::Ptr<cv::FaceRecognizerSF> recognizer = cv::FaceRecognizerSF::create(sface_path, "");
            cv::Mat aligned_face;
            recognizer->alignCrop(img, faces.row(0), aligned_face);

            cv::Mat feature;
            recognizer->feature(aligned_face, feature);
            std::cout << "Feature extracted for image " << i << "\n";
        }
        else {
            std::cout << "No feature extracted for image " << i << "\n";
        }
    }
}