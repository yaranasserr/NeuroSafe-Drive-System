#include "ppg_stress.h"
#define SHARED_MEMORY_PATH "/dev/shm/ppg_to_cpp_shm"
#define SHARED_MEMORY_SIZE 400
#define MUTEX_PATH "/home/yara/system_motion/ppg_to_cpp_mutex"

PPGStress::PPGStress() : fd(-1), mutex_fd(-1), addr(nullptr), batch_start_index(1) {}

PPGStress::~PPGStress() {
    if (addr != MAP_FAILED && addr != nullptr) {
        munmap(addr, SHARED_MEMORY_SIZE);
    }
    if (fd != -1) {
        close(fd);
    }
    if (mutex_fd != -1) {
        close(mutex_fd);
    }
}

bool PPGStress::initializeSharedMemory() {
    fd = open(SHARED_MEMORY_PATH, O_RDWR);
    if (fd == -1) {
        perror("Failed to open shared memory for input");
        return false;
    }

    mutex_fd = open(MUTEX_PATH, O_RDWR);
    if (mutex_fd == -1) {
        perror("Failed to open mutex");
        close(fd);
        return false;
    }

    addr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("Failed to map shared memory for input");
        close(fd);
        close(mutex_fd);
        return false;
    }

    return true;
}

bool PPGStress::loadModel(const char* model_path) {
    model = tflite::FlatBufferModel::BuildFromFile(model_path);
    if (!model) {
        std::cerr << "Failed to load model from file: " << model_path << std::endl;
        return false;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    if (builder(&interpreter) != kTfLiteOk) {
        std::cerr << "Failed to build interpreter for model" << std::endl;
        return false;
    }

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        std::cerr << "Failed to allocate tensors" << std::endl;
        return false;
    }

    return true;
}

void PPGStress::processBatch() {
    std::vector<float> input_data;
    for (int j = 0; j < 100; ++j) {
        for (float feature : data_vec[j].features) {
            input_data.push_back(feature);
        }
    }

    int batch_end_index = batch_start_index + 99;
    std::cout << "Batch starting at index " << batch_start_index << " ends at " << batch_end_index << " contains 100 points:" << std::endl;

    float* input = interpreter->typed_input_tensor<float>(0);
    std::copy(input_data.begin(), input_data.end(), input);

    if (interpreter->Invoke() != kTfLiteOk) {
        std::cerr << "Error during inference" << std::endl;
    }

    const float* output = interpreter->typed_output_tensor<float>(0);
    int predicted_label = output[0] > 0.5 ? 1 : 0;
    std::cout << "Model output for batch starting at index " << batch_start_index << ": " << output[0] << " (Predicted label: " << predicted_label << ")" << std::endl;

    data_vec.erase(data_vec.begin(), data_vec.begin() + 100);
    batch_start_index = batch_end_index + 1;
}

void PPGStress::readDataFromSharedMemory() {
    while (true) {
        if (flock(mutex_fd, LOCK_EX) == -1) {
            perror("Failed to lock mutex");
            break;
        }

        char* data = static_cast<char*>(addr);
        if (strlen(data) > 0) {
            std::istringstream ss(data);
            std::string row;

            while (std::getline(ss, row, '\n')) {
                std::istringstream rowStream(row);
                std::string value;
                DataRow dataRow;
                while (std::getline(rowStream, value, ',')) {
                    dataRow.features.push_back(std::stof(value));
                }
                dataRow.label = static_cast<int>(dataRow.features.back());
                dataRow.features.pop_back();
                if (dataRow.label == 0 || dataRow.label == 1 || dataRow.label == 2) {
                    if (dataRow.label == 2) dataRow.label = 0;
                    data_vec.push_back(dataRow);
                }

                if (data_vec.size() >= 100) {
                    processBatch();
                }
            }
        }

        if (flock(mutex_fd, LOCK_UN) == -1) {
            perror("Failed to unlock mutex");
            break;
        }
    }
}

void PPGStress::run() {
    if (!initializeSharedMemory()) {
        return;
    }

    const char* model_path = "/home/yara/system_motion/models/tflite_model_cnn_bilstm_aug.tflite";
    if (!loadModel(model_path)) {
        return;
    }

    readDataFromSharedMemory();
}

// #include <iostream>
// #include <sys/mman.h>
// #include <sys/file.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <cstring>
// #include <sstream>
// #include <vector>
// #include <tensorflow/lite/interpreter.h>
// #include <tensorflow/lite/kernels/register.h>
// #include <tensorflow/lite/model.h>

// #define SHARED_MEMORY_PATH "/dev/shm/csv_to_cpp_shm"
// #define SHARED_MEMORY_SIZE 600
// #define MUTEX_PATH "/home/yara/system_motion/csv_to_cpp_mutex"

// struct DataRow {
//     std::vector<float> features;
//     int label;
// };

// int main() {
//     // Open shared memory for input
//     int fd = open(SHARED_MEMORY_PATH, O_RDWR);
//     if (fd == -1) {
//         perror("Failed to open shared memory for input");
//         return 1;
//     }

//     // Open mutex for synchronization
//     int mutex_fd = open(MUTEX_PATH, O_RDWR);
//     if (mutex_fd == -1) {
//         perror("Failed to open mutex");
//         close(fd);
//         return 1;
//     }

//     // Map shared memory to process address space
//     void* addr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//     if (addr == MAP_FAILED) {
//         perror("Failed to map shared memory for input");
//         close(fd);
//         close(mutex_fd);
//         return 1;
//     }

//     // Load the TensorFlow Lite model
//     const char* model_path = "/home/yara/system_motion/models/tflite_model_cnn_bilstm_aug.tflite";
//     auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
//     if (!model) {
//         std::cerr << "Failed to load model from file: " << model_path << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         return 1;
//     }

//     tflite::ops::builtin::BuiltinOpResolver resolver;
//     std::unique_ptr<tflite::Interpreter> interpreter;
//     tflite::InterpreterBuilder builder(*model, resolver);
//     if (builder(&interpreter) != kTfLiteOk) {
//         std::cerr << "Failed to build interpreter for model" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         return 1;
//     }

//     if (interpreter->AllocateTensors() != kTfLiteOk) {
//         std::cerr << "Failed to allocate tensors" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         return 1;
//     }

//     std::vector<DataRow> data_vec;
//     int batch_start_index = 1;

//     while (true) {
//         // Lock mutex
//         if (flock(mutex_fd, LOCK_EX) == -1) {
//             perror("Failed to lock mutex");
//             break;
//         }

//         // Read from shared memory
//         char* data = static_cast<char*>(addr);
//         if (strlen(data) > 0) {
//             std::istringstream ss(data);
//             std::string row;

//             while (std::getline(ss, row, '\n')) {
//                 std::istringstream rowStream(row);
//                 std::string value;
//                 DataRow dataRow;
//                 while (std::getline(rowStream, value, ',')) {
//                     dataRow.features.push_back(std::stof(value));
//                 }
//                 dataRow.label = static_cast<int>(dataRow.features.back());
//                 dataRow.features.pop_back();
//                 if (dataRow.label == 0 || dataRow.label == 1 || dataRow.label == 2) {
//                     if (dataRow.label == 2) dataRow.label = 0;
//                     data_vec.push_back(dataRow);
//                 }

//                 // Process data in batches of 100 points
//                 if (data_vec.size() >= 100) {
//                     std::vector<float> input_data;
//                     for (int j = 0; j < 100; ++j) {
//                         for (float feature : data_vec[j].features) {
//                             input_data.push_back(feature);
//                         }
//                     }

//                     // Print the 100 points of data
//                     int batch_end_index = batch_start_index + 99;
//                     std::cout << "Batch starting at index " << batch_start_index << " ends at " << batch_end_index << " contains 100 points:" << std::endl;

//                     // Set input data for the model
//                     float* input = interpreter->typed_input_tensor<float>(0);
//                     std::copy(input_data.begin(), input_data.end(), input);

//                     // Invoke the interpreter to perform inference
//                     if (interpreter->Invoke() != kTfLiteOk) {
//                         std::cerr << "Error during inference" << std::endl;
//                     }

//                     // Get output from the model
//                     const float* output = interpreter->typed_output_tensor<float>(0);

//                     // Convert output to label based on threshold 0.5
//                     int predicted_label = output[0] > 0.5 ? 1 : 0;

//                     // Print the predicted label for the batch
//                     std::cout << "Model output for batch starting at index " << batch_start_index << ": " << output[0] << " (Predicted label: " << predicted_label << ")" << std::endl;

//                     // Clear processed data and update batch start index
//                     data_vec.erase(data_vec.begin(), data_vec.begin() + 100);
//                     batch_start_index = batch_end_index + 1;
//                 }
//             }
//         }

//         // Unlock the mutex
//         if (flock(mutex_fd, LOCK_UN) == -1) {
//             perror("Failed to unlock mutex");
//             break;
//         }

//     }

//     // Clean up resources
//     munmap(addr, SHARED_MEMORY_SIZE);
//     close(fd);
//     close(mutex_fd);

//     return 0;
// }
