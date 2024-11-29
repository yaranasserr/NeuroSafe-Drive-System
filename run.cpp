#include <opencv2/opencv.hpp>
#include <torch/torch.h>
#include <torch/script.h>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <thread>
#include <Eigen/Dense>

// Function to preprocess a frame for TensorFlow Lite
cv::Mat preprocess_frame(const cv::Mat& frame) {
    cv::Mat processed_frame;
    cv::resize(frame, processed_frame, cv::Size(128, 128));
    cv::cvtColor(processed_frame, processed_frame, cv::COLOR_BGR2GRAY);
    processed_frame = processed_frame.reshape(1, {1, 128, 128, 1});
    return processed_frame;
}

// Function to perform TensorFlow Lite model inference for rPPG and HR
void calculate_rPPG(tflite::Interpreter* rppg_interpreter, const std::vector<cv::Mat>& frames, std::map<std::string, float>& result) {
    std::vector<cv::Mat> processed_frames;
    for (const auto& frame : frames) {
        processed_frames.push_back(preprocess_frame(frame));
    }
    cv::Mat input_data = cv::Mat(processed_frames).reshape(1, {1, 3, 160, 128, 128});
    input_data.convertTo(input_data, CV_32F, 1.0 / 255.0);

    rppg_interpreter->typed_tensor<float>(0)[0] = input_data.ptr<float>(0);

    rppg_interpreter->Invoke();

    float* output = rppg_interpreter->typed_output_tensor<float>(0);
    torch::Tensor rppg_output = torch::from_blob(output, {1, 160, 128, 128}, torch::kFloat32);

    torch::Tensor HR_predicted = TorchLossComputer::cross_entropy_power_spectrum_forward_pred(rppg_output, 30) + 40;
    result["rPPG"] = rppg_output.mean().item<float>();
    result["HR"] = HR_predicted.item<float>();
}

// Function to perform TensorFlow Lite model inference
void Prediction_from_rPPG(tflite::Interpreter* interpreter, const std::vector<cv::Mat>& frames, std::map<std::string, float>& result) {
    std::vector<cv::Mat> processed_frames;
    for (const auto& frame : frames) {
        processed_frames.push_back(preprocess_frame(frame));
    }
    cv::Mat input_data = cv::Mat(processed_frames).reshape(1, {1, 160, 1});
    input_data.convertTo(input_data, CV_32F);

    interpreter->typed_tensor<float>(0)[0] = input_data.ptr<float>(0);
    interpreter->Invoke();
    result["tf_prediction"] = interpreter->typed_output_tensor<float>(0)[0];
}

int main() {
    std::unique_ptr<tflite::Interpreter> rppg_interpreter;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::FlatBufferModel::BuildFromFile("/home/yara/Desktop/GP/71-20241105T115730Z-001/71/model30.tflite")->BuildInterpreter(&rppg_interpreter);
    tflite::FlatBufferModel::BuildFromFile("/home/yara/Desktop/GP/71-20241105T115730Z-001/71/Prediction.tflite")->BuildInterpreter(&interpreter);

    rppg_interpreter->AllocateTensors();
    interpreter->AllocateTensors();

    cv::VideoCapture cap(0);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    int frame_rate = 30;
    int clip_size = 160;
    std::vector<cv::Mat> frames;
    std::map<std::string, float> result = {{"rPPG", 0}, {"HR", 0}, {"tf_prediction", 0}};

    while (true) {
        cv::Mat frame;
        cap.read(frame);
        frames.push_back(frame);

        if (frames.size() == clip_size) {
            std::thread rppg_thread(calculate_rPPG, rppg_interpreter.get(), frames, std::ref(result));
            std::thread prediction_thread(Prediction_from_rPPG, interpreter.get(), frames, std::ref(result));

            rppg_thread.join();
            prediction_thread.join();
            frames.clear();
        }

        std::string prediction_text = "Prediction: " + std::to_string(result["tf_prediction"]);
        cv::putText(frame, "HR: " + std::to_string(result["HR"]), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
        cv::putText(frame, prediction_text, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
        cv::imshow("Webcam", frame);

        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
