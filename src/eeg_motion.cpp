#include "eeg_motion.h"


// Constants for shared memory paths and sizes
#define SHARED_MEMORY_PATH "/dev/shm/csv_to_cpp_shm"
#define SHARED_MEMORY_SIZE 600
#define MUTEX_PATH "/home/yara/system_motion/src/csv_to_cpp_mutex"

// Output shared memory
#define OUTPUT_SHARED_MEMORY_PATH "/dev/shm/cpp_to_cpp_shm"
#define OUTPUT_SHARED_MEMORY_SIZE sizeof(int) * 5  // 5 integers
#define OUTPUT_MUTEX_PATH "/home/yara/system_motion/src/motion_output_mutex"

static int fd = -1;
static int mutex_fd = -1;
static void* addr = MAP_FAILED;
static int output_fd = -1;
static int output_mutex_fd = -1;
static void* output_addr = MAP_FAILED;

static void cleanup() {
    // Clean up output shared memory and mutex
    if (output_addr != MAP_FAILED) munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
    if (output_fd != -1) close(output_fd);
    if (output_mutex_fd != -1) close(output_mutex_fd);

    // Unlink output shared memory and mutex files
    shm_unlink(OUTPUT_SHARED_MEMORY_PATH);
    unlink(OUTPUT_MUTEX_PATH);

    // Clean up input shared memory and mutex
    if (addr != MAP_FAILED) munmap(addr, SHARED_MEMORY_SIZE);
    if (fd != -1) close(fd);
    if (mutex_fd != -1) close(mutex_fd);

    // Unlink input shared memory and mutex files
    shm_unlink(SHARED_MEMORY_PATH);
    unlink(MUTEX_PATH);
}

static void signal_handler(int signal) {
    std::cout << "Interrupt signal (" << signal << ") received.\n";
    cleanup();
    exit(signal);
}

void EEGMotion::normalize(std::vector<float>& values) {
    float min_val = *std::min_element(values.begin(), values.end());
    float max_val = *std::max_element(values.begin(), values.end());
    float range = max_val - min_val;

    if (range == 0) {
        for (auto& value : values) {
            value = 0.0f;
        }
    } else {
        for (size_t i = 0; i < values.size(); ++i) {
            values[i] = (values[i] - min_val) / range;
        }
    }
}

std::vector<std::vector<float>> EEGMotion::reshape_row(const std::vector<float>& row) {
    std::vector<std::vector<float>> electrode_matrix = {
        { 0, 0, 0, row[0], 0, 0, row[13], 0, 0, 0 },
        { 0, row[1], 0, 0, row[2], row[11], 0, 0, row[12], 0 },
        { 0, 0, row[3], 0, 0, 0, 0, row[10], 0, 0 },
        { row[4], 0, 0, 0, 0, 0, 0, 0, 0, row[9] },
        { 0, 0, row[5], 0, 0, 0, 0, row[8], 0, 0 },
        { 0, 0, 0, 0, row[6], row[7], 0, 0, 0, 0 }
    };

    return electrode_matrix;
}

// Custom hash function for vector<int>
struct VectorHash {
    std::size_t operator()(const std::vector<int>& vec) const {
        std::size_t seed = vec.size();
        for (auto& i : vec) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

// Custom equality function for vector<int>
struct VectorEqual {
    bool operator()(const std::vector<int>& lhs, const std::vector<int>& rhs) const {
        return lhs == rhs;
    }
};

// Function to calculate the mode of a vector of vectors
std::vector<int> calculate_mode(const std::vector<std::vector<int>>& vecs) {
    std::unordered_map<std::vector<int>, int, VectorHash, VectorEqual> frequency_map;

    // Count the frequency of each vector
    for (const auto& vec : vecs) {
        frequency_map[vec]++;
    }

    // Find the vector with the highest frequency
    auto mode = vecs[0];
    int max_count = 0;
    for (const auto& [vec, count] : frequency_map) {
        if (count > max_count) {
            max_count = count;
            mode = vec;
        }
    }

    return mode;
}

// Function to convert one-hot encoded labels to their original label
std::string decode_label(const std::vector<int>& one_hot) {
    if (one_hot == std::vector<int>{1, 0, 0, 0, 0}) return "forward";
    if (one_hot == std::vector<int>{0, 1, 0, 0, 0}) return "backward";
    if (one_hot == std::vector<int>{0, 0, 1, 0, 0}) return "left";
    if (one_hot == std::vector<int>{0, 0, 0, 1, 0}) return "right";
    if (one_hot == std::vector<int>{0, 0, 0, 0, 1}) return "stop";
    return "unknown";
}

void EEGMotion::run() {
    // Register signal handler
    signal(SIGINT, signal_handler);

    // Open additional shared memory for input
    fd = open(SHARED_MEMORY_PATH, O_RDWR);
    if (fd == -1) {
        perror("Failed to open shared memory for input");
        return;
    }

    // Open additional mutex for synchronization
    mutex_fd = open(MUTEX_PATH, O_RDWR);
    if (mutex_fd == -1) {
        perror("Failed to open mutex for input");
        close(fd);
        return;
    }

    // Map additional shared memory to process address space
    addr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("Failed to map shared memory for input");
        close(fd);
        close(mutex_fd);
        return;
    }

    // Open output shared memory
    output_fd = open(OUTPUT_SHARED_MEMORY_PATH, O_RDWR | O_CREAT, 0666);
    if (output_fd == -1) {
        perror("Failed to open output shared memory");
        munmap(addr, SHARED_MEMORY_SIZE);
        close(fd);
        close(mutex_fd);
        return;
    }

    if (ftruncate(output_fd, OUTPUT_SHARED_MEMORY_SIZE) == -1) {
        perror("Failed to set size of output shared memory");
        munmap(addr, SHARED_MEMORY_SIZE);
        close(fd);
        close(mutex_fd);
        close(output_fd);
        return;
    }

    // Open output mutex for synchronization
    output_mutex_fd = open(OUTPUT_MUTEX_PATH, O_RDWR | O_CREAT, 0666);
    if (output_mutex_fd == -1) {
        perror("Failed to open output mutex");
        munmap(addr, SHARED_MEMORY_SIZE);
        close(fd);
        close(mutex_fd);
        close(output_fd);
        return;
    }

    // Map output shared memory to process address space
    output_addr = mmap(NULL, OUTPUT_SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);
    if (output_addr == MAP_FAILED) {
        perror("Failed to map output shared memory");
        munmap(addr, SHARED_MEMORY_SIZE);
        close(fd);
        close(mutex_fd);
        close(output_fd);
        close(output_mutex_fd);
        return;
    }

    // Ensure output memory alignment
    if (reinterpret_cast<uintptr_t>(output_addr) % alignof(int) != 0) {
        std::cerr << "Output shared memory is not properly aligned" << std::endl;
        munmap(addr, SHARED_MEMORY_SIZE);
        close(fd);
        close(mutex_fd);
        munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
        close(output_fd);
        close(output_mutex_fd);
        return;
    }

    // Load the TensorFlow Lite model
    const char* model_path = "/home/yara/system_motion/models/motion_model_3d_00-07-07_29-06-2024.tflite";
    auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
    if (!model) {
        std::cerr << "Failed to load model from file: " << model_path << std::endl;
        munmap(addr, SHARED_MEMORY_SIZE);
        close(fd);
        close(mutex_fd);
        munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
        close(output_fd);
        close(output_mutex_fd);
        return;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::InterpreterBuilder builder(*model, resolver);
    if (builder(&interpreter) != kTfLiteOk) {
        std::cerr << "Failed to build interpreter for model" << std::endl;
        munmap(addr, SHARED_MEMORY_SIZE);
        close(fd);
        close(mutex_fd);
        munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
        close(output_fd);
        close(output_mutex_fd);
        return;
    }

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        std::cerr << "Failed to allocate tensors" << std::endl;
        munmap(addr, SHARED_MEMORY_SIZE);
        close(fd);
        close(mutex_fd);
        munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
        close(output_fd);
        close(output_mutex_fd);
        return;
    }

    std::vector<std::vector<int>> all_predictions;
    std::vector<std::vector<int>> mode_vector;
    int batch_count = 0;

    while (true) {
        // Lock mutex
        if (flock(mutex_fd, LOCK_EX) == -1) {
            perror("Failed to lock mutex");
            break;
        }

        // Read from shared memory
        char* data = static_cast<char*>(addr);
        if (strlen(data) > 0) {
            // std::cout << "Data read from shared memory: " << std::endl;
            // std::cout << data << std::endl;

            std::vector<float> data_vec;
            std::istringstream ss(data);
            std::string token;
            while (std::getline(ss, token, ',')) {
                try {
                    data_vec.push_back(std::stof(token));
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid data encountered in shared memory: " << token << std::endl;
                    continue;
                }
            }

            // Normalize the data
            normalize(data_vec);

            // Reshape the data into a 2D matrix
            std::vector<std::vector<float>> reshaped_data = reshape_row(data_vec);

            // Prepare input tensor
            float* input = interpreter->typed_input_tensor<float>(0);
            if (input == nullptr) {
                std::cerr << "Failed to get input tensor" << std::endl;
                break;
            }
            for (size_t i = 0; i < reshaped_data.size(); ++i) {
                for (size_t j = 0; j < reshaped_data[i].size(); ++j) {
                    input[i * reshaped_data[i].size() + j] = reshaped_data[i][j];
                }
            }

            // Invoke the interpreter
            if (interpreter->Invoke() != kTfLiteOk) {
                std::cerr << "Failed to invoke interpreter" << std::endl;
                break;
            }

            // Get the output
            float* output = interpreter->typed_output_tensor<float>(0);
            if (output == nullptr) {
                std::cerr << "Failed to get output tensor" << std::endl;
                break;
            }

            std::vector<int> prediction(5);
            for (size_t i = 0; i < 5; ++i) {
                prediction[i] = static_cast<int>(output[i]);
            }

            // Append prediction to all_predictions
            all_predictions.push_back(prediction);

            // Check if we have accumulated 25 samples
            if (all_predictions.size() == 25) {
                batch_count++;
                std::vector<int> mode = calculate_mode(all_predictions);

                // Store the mode in mode_vector
                mode_vector.push_back(mode);

                // Print the mode and sample indices
                int start_sample = (batch_count - 1) * 25 + 1;
                int end_sample = batch_count * 25;
                std::cout << "Calculated mode for samples " << start_sample << " to " << end_sample << ": ";
                for (int val : mode) {
                    std::cout << val << " ";
                }
                std::cout << std::endl;

                // Convert mode to label and print
                std::string mode_label = decode_label(mode);
                std::cout << "Mode label: " << mode_label << std::endl;

                // Write the mode to shared memory
                if (flock(output_mutex_fd, LOCK_EX) == -1) {
                    perror("Failed to lock output mutex");
                    break;
                }

                // Ensure the memory being accessed is valid and within bounds
                if (OUTPUT_SHARED_MEMORY_SIZE >= sizeof(int) * 5) {
                    std::memcpy(output_addr, mode.data(), sizeof(int) * 5);
                    std::cout << "Successfully copied to output shared memory" << std::endl;
                } else {
                    std::cerr << "Output shared memory size is too small" << std::endl;
                    break;
                }

                std::cout << "Written to output shared memory: ";
                for (int val : mode) {
                    std::cout << val << " ";
                }
                std::cout << std::endl;

                if (flock(output_mutex_fd, LOCK_UN) == -1) {
                    perror("Failed to unlock output mutex");
                    break;
                }

                // Clear the all_predictions vector for the next batch
                all_predictions.clear();
            }

            // Clear the input shared memory to avoid re-processing the same data
            std::memset(addr, 0, SHARED_MEMORY_SIZE);
           // std::memset(addr, 0, OUTPUT_SHARED_MEMORY_SIZE);
            //std::cout << "Cleared input shared memory" << std::endl;
        }

        // Unlock mutex
        if (flock(mutex_fd, LOCK_UN) == -1) {
            perror("Failed to unlock mutex");
            break;
        }

        // Sleep for a short interval before reading again
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
       // std::cout << "Loop iteration completed" << std::endl;
    }

    // Clean up
    cleanup();
}

// #include "eeg_motion.h"

// // Constants for shared memory paths and sizes
// #define SHARED_MEMORY_PATH "/dev/shm/csv_to_cpp_shm"
// #define SHARED_MEMORY_SIZE 600
// #define MUTEX_PATH "/home/yara/system_motion/src/csv_to_cpp_mutex"

// // Output shared memory
// #define OUTPUT_SHARED_MEMORY_PATH "/dev/shm/cpp_to_cpp_shm"
// #define OUTPUT_SHARED_MEMORY_SIZE sizeof(int) * 5  // 5 integers
// #define OUTPUT_MUTEX_PATH "/home/yara/system_motion/src/motion_output_mutex"

// static int fd = -1;
// static int mutex_fd = -1;
// static void* addr = MAP_FAILED;
// static int output_fd = -1;
// static int output_mutex_fd = -1;
// static void* output_addr = MAP_FAILED;

// static void cleanup() {
//     // Clean up output shared memory and mutex
//     if (output_addr != MAP_FAILED) munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//     if (output_fd != -1) close(output_fd);
//     if (output_mutex_fd != -1) close(output_mutex_fd);

//     // Unlink output shared memory and mutex files
//     shm_unlink(OUTPUT_SHARED_MEMORY_PATH);
//     unlink(OUTPUT_MUTEX_PATH);
// }

// static void signal_handler(int signal) {
//     std::cout << "Interrupt signal (" << signal << ") received.\n";
//     cleanup();
//     exit(signal);
// }

// void EEGMotion::normalize(std::vector<float>& values) {
//     float min_val = *std::min_element(values.begin(), values.end());
//     float max_val = *std::max_element(values.begin(), values.end());
//     float range = max_val - min_val;

//     if (range == 0) {
//         for (auto& value : values) {
//             value = 0.0f;
//         }
//     } else {
//         for (size_t i = 0; i < values.size(); ++i) {
//             values[i] = (values[i] - min_val) / range;
//         }
//     }
// }

// std::vector<std::vector<float>> EEGMotion::reshape_row(const std::vector<float>& row) {
//     std::vector<std::vector<float>> electrode_matrix = {
//         { 0, 0, 0, row[0], 0, 0, row[13], 0, 0, 0 },
//         { 0, row[1], 0, 0, row[2], row[11], 0, 0, row[12], 0 },
//         { 0, 0, row[3], 0, 0, 0, 0, row[10], 0, 0 },
//         { row[4], 0, 0, 0, 0, 0, 0, 0, 0, row[9] },
//         { 0, 0, row[5], 0, 0, 0, 0, row[8], 0, 0 },
//         { 0, 0, 0, 0, row[6], row[7], 0, 0, 0, 0 }
//     };

//     return electrode_matrix;
// }

// // Custom hash function for vector<int>
// struct VectorHash {
//     std::size_t operator()(const std::vector<int>& vec) const {
//         std::size_t seed = vec.size();
//         for (auto& i : vec) {
//             seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//         }
//         return seed;
//     }
// };

// // Custom equality function for vector<int>
// struct VectorEqual {
//     bool operator()(const std::vector<int>& lhs, const std::vector<int>& rhs) const {
//         return lhs == rhs;
//     }
// };

// // Function to calculate the mode of a vector of vectors
// std::vector<int> calculate_mode(const std::vector<std::vector<int>>& vecs) {
//     std::unordered_map<std::vector<int>, int, VectorHash, VectorEqual> frequency_map;

//     // Count the frequency of each vector
//     for (const auto& vec : vecs) {
//         frequency_map[vec]++;
//     }

//     // Find the vector with the highest frequency
//     auto mode = vecs[0];
//     int max_count = 0;
//     for (const auto& [vec, count] : frequency_map) {
//         if (count > max_count) {
//             max_count = count;
//             mode = vec;
//         }
//     }

//     return mode;
// }

// // Function to convert one-hot encoded labels to their original label
// std::string decode_label(const std::vector<int>& one_hot) {
//     if (one_hot == std::vector<int>{1, 0, 0, 0, 0}) return "forward";
//     if (one_hot == std::vector<int>{0, 1, 0, 0, 0}) return "backward";
//     if (one_hot == std::vector<int>{0, 0, 1, 0, 0}) return "left";
//     if (one_hot == std::vector<int>{0, 0, 0, 1, 0}) return "right";
//     if (one_hot == std::vector<int>{0, 0, 0, 0, 1}) return "stop";
//     return "unknown";
// }

// void EEGMotion::run() {
//     // Register signal handler
//     signal(SIGINT, signal_handler);

//     // Open additional shared memory for input
//     fd = open(SHARED_MEMORY_PATH, O_RDWR);
//     if (fd == -1) {
//         perror("Failed to open shared memory for input");
//         return;
//     }

//     // Open additional mutex for synchronization
//     mutex_fd = open(MUTEX_PATH, O_RDWR);
//     if (mutex_fd == -1) {
//         perror("Failed to open mutex for input");
//         close(fd);
//         return;
//     }

//     // Map additional shared memory to process address space
//     addr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//     if (addr == MAP_FAILED) {
//         perror("Failed to map shared memory for input");
//         close(fd);
//         close(mutex_fd);
//         return;
//     }

//     // Open output shared memory
//     output_fd = open(OUTPUT_SHARED_MEMORY_PATH, O_RDWR | O_CREAT, 0666);
//     if (output_fd == -1) {
//         perror("Failed to open output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         return;
//     }

//     if (ftruncate(output_fd, OUTPUT_SHARED_MEMORY_SIZE) == -1) {
//         perror("Failed to set size of output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Open output mutex for synchronization
//     output_mutex_fd = open(OUTPUT_MUTEX_PATH, O_RDWR | O_CREAT, 0666);
//     if (output_mutex_fd == -1) {
//         perror("Failed to open output mutex");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Map output shared memory to process address space
//     output_addr = mmap(NULL, OUTPUT_SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);
//     if (output_addr == MAP_FAILED) {
//         perror("Failed to map output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Ensure output memory alignment
//     if (reinterpret_cast<uintptr_t>(output_addr) % alignof(int) != 0) {
//         std::cerr << "Output shared memory is not properly aligned" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Load the TensorFlow Lite model
//     const char* model_path = "/home/yara/system_motion/models/motion_model_3d_00-07-07_29-06-2024.tflite";
//     auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
//     if (!model) {
//         std::cerr << "Failed to load model from file: " << model_path << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     tflite::ops::builtin::BuiltinOpResolver resolver;
//     std::unique_ptr<tflite::Interpreter> interpreter;
//     tflite::InterpreterBuilder builder(*model, resolver);
//     if (builder(&interpreter) != kTfLiteOk) {
//         std::cerr << "Failed to build interpreter for model" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     if (interpreter->AllocateTensors() != kTfLiteOk) {
//         std::cerr << "Failed to allocate tensors" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     std::vector<std::vector<int>> all_predictions;
//     std::vector<std::vector<int>> mode_vector;
//     int batch_count = 0;

//     while (true) {
//         // Lock mutex
//         if (flock(mutex_fd, LOCK_EX) == -1) {
//             perror("Failed to lock mutex");
//             break;
//         }

//         // Read from shared memory
//         char* data = static_cast<char*>(addr);
//         if (strlen(data) > 0) {
//             // std::cout << "Data read from shared memory: " << std::endl;
//             // std::cout << data << std::endl;

//             std::vector<float> data_vec;
//             std::istringstream ss(data);
//             std::string token;
//             while (std::getline(ss, token, ',')) {
//                 try {
//                     data_vec.push_back(std::stof(token));
//                 } catch (const std::invalid_argument& e) {
//                     std::cerr << "Invalid data encountered in shared memory: " << token << std::endl;
//                     continue;
//                 }
//             }

//             // Normalize the data
//             normalize(data_vec);

//             // Reshape the data into a 2D matrix
//             std::vector<std::vector<float>> reshaped_data = reshape_row(data_vec);

//             // Prepare input tensor
//             float* input = interpreter->typed_input_tensor<float>(0);
//             if (input == nullptr) {
//                 std::cerr << "Failed to get input tensor" << std::endl;
//                 break;
//             }
//             for (size_t i = 0; i < reshaped_data.size(); ++i) {
//                 for (size_t j = 0; j < reshaped_data[i].size(); ++j) {
//                     input[i * reshaped_data[i].size() + j] = reshaped_data[i][j];
//                 }
//             }

//             // Invoke the interpreter
//             if (interpreter->Invoke() != kTfLiteOk) {
//                 std::cerr << "Failed to invoke interpreter" << std::endl;
//                 break;
//             }

//             // Get the output
//             float* output = interpreter->typed_output_tensor<float>(0);
//             if (output == nullptr) {
//                 std::cerr << "Failed to get output tensor" << std::endl;
//                 break;
//             }

//             std::vector<int> prediction(5);
//             for (size_t i = 0; i < 5; ++i) {
//                 prediction[i] = static_cast<int>(output[i]);
//             }

//             // Append prediction to all_predictions
//             all_predictions.push_back(prediction);

//             // Check if we have accumulated 25 samples
//             if (all_predictions.size() == 25) {
//                 batch_count++;
//                 std::vector<int> mode = calculate_mode(all_predictions);

//                 // Store the mode in mode_vector
//                 mode_vector.push_back(mode);

//                 // Print the mode and sample indices
//                 int start_sample = (batch_count - 1) * 25 + 1;
//                 int end_sample = batch_count * 25;
//                 std::cout << "Calculated mode for samples " << start_sample << " to " << end_sample << ": ";
//                 for (int val : mode) {
//                     std::cout << val << " ";
//                 }
//                 std::cout << std::endl;

//                 // Convert mode to label and print
//                 std::string mode_label = decode_label(mode);
//                 std::cout << "Mode label: " << mode_label << std::endl;

//                 // Write the mode to shared memory
//                 if (flock(output_mutex_fd, LOCK_EX) == -1) {
//                     perror("Failed to lock output mutex");
//                     break;
//                 }

//                 // Ensure the memory being accessed is valid and within bounds
//                 if (OUTPUT_SHARED_MEMORY_SIZE >= sizeof(int) * 5) {
//                     std::memcpy(output_addr, mode.data(), sizeof(int) * 5);
//                     std::cout << "Successfully copied to output shared memory" << std::endl;
//                 } else {
//                     std::cerr << "Output shared memory size is too small" << std::endl;
//                     break;
//                 }

//                 std::cout << "Written to output shared memory: ";
//                 for (int val : mode) {
//                     std::cout << val << " ";
//                 }
//                 std::cout << std::endl;

//                 if (flock(output_mutex_fd, LOCK_UN) == -1) {
//                     perror("Failed to unlock output mutex");
//                     break;
//                 }

//                 // Clear the all_predictions vector for the next batch
//                 all_predictions.clear();
//             }

//             // Clear the input shared memory to avoid re-processing the same data
//             std::memset(addr, 0, SHARED_MEMORY_SIZE);
//             //std::cout << "Cleared input shared memory" << std::endl;
//         }

//         // Unlock mutex
//         if (flock(mutex_fd, LOCK_UN) == -1) {
//             perror("Failed to unlock mutex");
//             break;
//         }

//         // Sleep for a short interval before reading again
//         std::this_thread::sleep_for(std::chrono::milliseconds(25));
//        // std::cout << "Loop iteration completed" << std::endl;
//     }

//     // Clean up
//     cleanup();
// }

// was working with mode of 25
// #include "eeg_motion.h"


// // Constants for shared memory paths and sizes
// #define SHARED_MEMORY_PATH "/dev/shm/csv_to_cpp_shm"
// #define SHARED_MEMORY_SIZE 600
// #define MUTEX_PATH "/home/yara/system_motion/src/csv_to_cpp_mutex"

// // Output shared memory
// #define OUTPUT_SHARED_MEMORY_PATH "/dev/shm/cpp_to_cpp_shm"
// #define OUTPUT_SHARED_MEMORY_SIZE sizeof(int) * 5  // 5 integers
// #define OUTPUT_MUTEX_PATH "/home/yara/system_motion/src/motion_output_mutex"

// static int fd = -1;
// static int mutex_fd = -1;
// static void* addr = MAP_FAILED;
// static int output_fd = -1;
// static int output_mutex_fd = -1;
// static void* output_addr = MAP_FAILED;

// static void cleanup() {
//     // Clean up output shared memory and mutex
//     if (output_addr != MAP_FAILED) munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//     if (output_fd != -1) close(output_fd);
//     if (output_mutex_fd != -1) close(output_mutex_fd);

//     // Unlink output shared memory and mutex files
//     shm_unlink(OUTPUT_SHARED_MEMORY_PATH);
//     unlink(OUTPUT_MUTEX_PATH);
// }

// static void signal_handler(int signal) {
//     std::cout << "Interrupt signal (" << signal << ") received.\n";
//     cleanup();
//     exit(signal);
// }

// void EEGMotion::normalize(std::vector<float>& values) {
//     float min_val = *std::min_element(values.begin(), values.end());
//     float max_val = *std::max_element(values.begin(), values.end());
//     float range = max_val - min_val;

//     if (range == 0) {
//         for (auto& value : values) {
//             value = 0.0f;
//         }
//     } else {
//         for (size_t i = 0; i < values.size(); ++i) {
//             values[i] = (values[i] - min_val) / range;
//         }
//     }
// }

// std::vector<std::vector<float>> EEGMotion::reshape_row(const std::vector<float>& row) {
//     std::vector<std::vector<float>> electrode_matrix = {
//         { 0, 0, 0, row[0], 0, 0, row[13], 0, 0, 0 },
//         { 0, row[1], 0, 0, row[2], row[11], 0, 0, row[12], 0 },
//         { 0, 0, row[3], 0, 0, 0, 0, row[10], 0, 0 },
//         { row[4], 0, 0, 0, 0, 0, 0, 0, 0, row[9] },
//         { 0, 0, row[5], 0, 0, 0, 0, row[8], 0, 0 },
//         { 0, 0, 0, 0, row[6], row[7], 0, 0, 0, 0 }
//     };

//     return electrode_matrix;
// }

// // Custom hash function for vector<int>
// struct VectorHash {
//     std::size_t operator()(const std::vector<int>& vec) const {
//         std::size_t seed = vec.size();
//         for (auto& i : vec) {
//             seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//         }
//         return seed;
//     }
// };

// // Custom equality function for vector<int>
// struct VectorEqual {
//     bool operator()(const std::vector<int>& lhs, const std::vector<int>& rhs) const {
//         return lhs == rhs;
//     }
// };

// // Function to calculate the mode of a vector of vectors
// std::vector<int> calculate_mode(const std::vector<std::vector<int>>& vecs) {
//     std::unordered_map<std::vector<int>, int, VectorHash, VectorEqual> frequency_map;

//     // Count the frequency of each vector
//     for (const auto& vec : vecs) {
//         frequency_map[vec]++;
//     }

//     // Find the vector with the highest frequency
//     auto mode = vecs[0];
//     int max_count = 0;
//     for (const auto& [vec, count] : frequency_map) {
//         if (count > max_count) {
//             max_count = count;
//             mode = vec;
//         }
//     }

//     return mode;
// }

// void EEGMotion::run() {
//     // Register signal handler
//     signal(SIGINT, signal_handler);

//     // Open additional shared memory for input
//     fd = open(SHARED_MEMORY_PATH, O_RDWR);
//     if (fd == -1) {
//         perror("Failed to open shared memory for input");
//         return;
//     }

//     // Open additional mutex for synchronization
//     mutex_fd = open(MUTEX_PATH, O_RDWR);
//     if (mutex_fd == -1) {
//         perror("Failed to open mutex for input");
//         close(fd);
//         return;
//     }

//     // Map additional shared memory to process address space
//     addr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//     if (addr == MAP_FAILED) {
//         perror("Failed to map shared memory for input");
//         close(fd);
//         close(mutex_fd);
//         return;
//     }

//     // Open output shared memory
//     output_fd = open(OUTPUT_SHARED_MEMORY_PATH, O_RDWR | O_CREAT, 0666);
//     if (output_fd == -1) {
//         perror("Failed to open output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         return;
//     }

//     if (ftruncate(output_fd, OUTPUT_SHARED_MEMORY_SIZE) == -1) {
//         perror("Failed to set size of output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Open output mutex for synchronization
//     output_mutex_fd = open(OUTPUT_MUTEX_PATH, O_RDWR | O_CREAT, 0666);
//     if (output_mutex_fd == -1) {
//         perror("Failed to open output mutex");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Map output shared memory to process address space
//     output_addr = mmap(NULL, OUTPUT_SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);
//     if (output_addr == MAP_FAILED) {
//         perror("Failed to map output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Ensure output memory alignment
//     if (reinterpret_cast<uintptr_t>(output_addr) % alignof(int) != 0) {
//         std::cerr << "Output shared memory is not properly aligned" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Load the TensorFlow Lite model
//     const char* model_path = "/home/yara/system_motion/models/motion_model_3d_00-07-07_29-06-2024.tflite";
//     auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
//     if (!model) {
//         std::cerr << "Failed to load model from file: " << model_path << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     tflite::ops::builtin::BuiltinOpResolver resolver;
//     std::unique_ptr<tflite::Interpreter> interpreter;
//     tflite::InterpreterBuilder builder(*model, resolver);
//     if (builder(&interpreter) != kTfLiteOk) {
//         std::cerr << "Failed to build interpreter for model" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     if (interpreter->AllocateTensors() != kTfLiteOk) {
//         std::cerr << "Failed to allocate tensors" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     std::vector<std::vector<int>> all_predictions;
//     std::vector<std::vector<int>> mode_vector;
//     int batch_count = 0;

//     while (true) {
//         // Lock mutex
//         if (flock(mutex_fd, LOCK_EX) == -1) {
//             perror("Failed to lock mutex");
//             break;
//         }

//         // Read from shared memory
//         char* data = static_cast<char*>(addr);
//         if (strlen(data) > 0) {
//             // std::cout << "Data read from shared memory: " << std::endl;
//             // std::cout << data << std::endl;

//             std::vector<float> data_vec;
//             std::istringstream ss(data);
//             std::string token;
//             while (std::getline(ss, token, ',')) {
//                 try {
//                     data_vec.push_back(std::stof(token));
//                 } catch (const std::invalid_argument& e) {
//                     std::cerr << "Invalid data encountered in shared memory: " << token << std::endl;
//                     continue;
//                 }
//             }

//             // Normalize the data
//             normalize(data_vec);

//             // Reshape the data into a 2D matrix
//             std::vector<std::vector<float>> reshaped_data = reshape_row(data_vec);

//             // Prepare input tensor
//             float* input = interpreter->typed_input_tensor<float>(0);
//             if (input == nullptr) {
//                 std::cerr << "Failed to get input tensor" << std::endl;
//                 break;
//             }
//             for (size_t i = 0; i < reshaped_data.size(); ++i) {
//                 for (size_t j = 0; j < reshaped_data[i].size(); ++j) {
//                     input[i * reshaped_data[i].size() + j] = reshaped_data[i][j];
//                 }
//             }

//             // Invoke the interpreter
//             if (interpreter->Invoke() != kTfLiteOk) {
//                 std::cerr << "Failed to invoke interpreter" << std::endl;
//                 break;
//             }

//             // Get the output
//             float* output = interpreter->typed_output_tensor<float>(0);
//             if (output == nullptr) {
//                 std::cerr << "Failed to get output tensor" << std::endl;
//                 break;
//             }

//             std::vector<int> prediction(5);
//             for (size_t i = 0; i < 5; ++i) {
//                 prediction[i] = static_cast<int>(output[i]);
//             }

//             // Append prediction to all_predictions
//             all_predictions.push_back(prediction);

//             // Check if we have accumulated 25 samples
//             if (all_predictions.size() == 25) {
//                 batch_count++;
//                 std::vector<int> mode = calculate_mode(all_predictions);

//                 // Store the mode in mode_vector
//                 mode_vector.push_back(mode);

//                 // Print the mode and sample indices
//                 int start_sample = (batch_count - 1) * 25 + 1;
//                 int end_sample = batch_count * 25;
//                 std::cout << "Calculated mode for samples " << start_sample << " to " << end_sample << ": ";
//                 for (int val : mode) {
//                     std::cout << val << " ";
//                 }
//                 std::cout << std::endl;

//                 // Write the mode to shared memory
//                 if (flock(output_mutex_fd, LOCK_EX) == -1) {
//                     perror("Failed to lock output mutex");
//                     break;
//                 }

//                 // Ensure the memory being accessed is valid and within bounds
//                 if (OUTPUT_SHARED_MEMORY_SIZE >= sizeof(int) * 5) {
//                     std::memcpy(output_addr, mode.data(), sizeof(int) * 5);
//                     std::cout << "Successfully copied to output shared memory" << std::endl;
//                 } else {
//                     std::cerr << "Output shared memory size is too small" << std::endl;
//                     break;
//                 }

//                 std::cout << "Written to output shared memory: ";
//                 for (int val : mode) {
//                     std::cout << val << " ";
//                 }
//                 std::cout << std::endl;

//                 if (flock(output_mutex_fd, LOCK_UN) == -1) {
//                     perror("Failed to unlock output mutex");
//                     break;
//                 }

//                 // Clear the all_predictions vector for the next batch
//                 all_predictions.clear();
//             }

//             // Clear the input shared memory to avoid re-processing the same data
//             // std::memset(addr, 0, SHARED_MEMORY_SIZE);
//             // std::cout << "Cleared input shared memory" << std::endl;
//         }

//         // Unlock mutex
//         if (flock(mutex_fd, LOCK_UN) == -1) {
//             perror("Failed to unlock mutex");
//             break;
//         }

//         // Sleep for a short interval before reading again
//         std::this_thread::sleep_for(std::chrono::milliseconds(25));
//        // std::cout << "Loop iteration completed" << std::endl;
//     }

//     // Clean up
//     cleanup();
// }


  
  


// int EEGMotion::output_fd = -1; 
// void * EEGMotion::output_addr = nullptr ;
// int EEGMotion::output_mutex_fd = -1 ; 

// void EEGMotion::cleanup() {
//     // Clean up
//     if (output_addr) munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//     if (output_fd != -1) close(output_fd);
//     if (output_mutex_fd != -1) close(output_mutex_fd);
// }

// void EEGMotion::signalHandler(int signum) {
//     cleanup();
//     exit(signum);
// }


// void EEGMotion::normalize(std::vector<float>& values) {
//     float min_val = *std::min_element(values.begin(), values.end());
//     float max_val = *std::max_element(values.begin(), values.end());
//     float range = max_val - min_val;

//     if (range == 0) {
//         for (auto& value : values) {
//             value = 0.0f;
//         }
//     } else {
//         for (size_t i = 0; i < values.size(); ++i) {
//             values[i] = (values[i] - min_val) / range;
//         }
//     }
// }

// std::vector<std::vector<float>> EEGMotion::reshape_row(const std::vector<float>& row) {
//     std::vector<std::vector<float>> electrode_matrix = {
//         { 0, 0, 0, row[0], 0, 0, row[13], 0, 0, 0 },
//         { 0, row[1], 0, 0, row[2], row[11], 0, 0, row[12], 0 },
//         { 0, 0, row[3], 0, 0, 0, 0, row[10], 0, 0 },
//         { row[4], 0, 0, 0, 0, 0, 0, 0, 0, row[9] },
//         { 0, 0, row[5], 0, 0, 0, 0, row[8], 0, 0 },
//         { 0, 0, 0, 0, row[6], row[7], 0, 0, 0, 0 }
//     };

//     return electrode_matrix;
// }

// void EEGMotion::run() {
//     // Register signal handler
//     signal(SIGINT, signalHandler);

//     // Open shared memory for input
//     int fd = open(SHARED_MEMORY_PATH, O_RDWR);
//     if (fd == -1) {
//         perror("Failed to open shared memory for input");
//         return;
//     }

//     // Open mutex for synchronization
//     int mutex_fd = open(MUTEX_PATH, O_RDWR);
//     if (mutex_fd == -1) {
//         perror("Failed to open mutex for input");
//         close(fd);
//         return;
//     }

//     // Map shared memory to process address space
//     void * addr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//     if (addr == MAP_FAILED) {
//         perror("Failed to map shared memory for input");
//         close(fd);
//         close(mutex_fd);
//         return;
//     }

//     // Open output shared memory
//     output_fd = open(OUTPUT_SHARED_MEMORY_PATH, O_RDWR | O_CREAT, 0666);
//     if (output_fd == -1) {
//         perror("Failed to open output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         return;
//     }

//     if (ftruncate(output_fd, OUTPUT_SHARED_MEMORY_SIZE) == -1) {
//         perror("Failed to set size of output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Open output mutex for synchronization
//     output_mutex_fd = open(OUTPUT_MUTEX_PATH, O_RDWR | O_CREAT, 0666);
//     if (output_mutex_fd == -1) {
//         perror("Failed to open output mutex");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Map output shared memory to process address space
//     output_addr = mmap(NULL, OUTPUT_SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);
//     if (output_addr == MAP_FAILED) {
//         perror("Failed to map output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Ensure output memory alignment
//     if (reinterpret_cast<uintptr_t>(output_addr) % alignof(int) != 0) {
//         std::cerr << "Output shared memory is not properly aligned" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Load the TensorFlow Lite model
//     const char* model_path = "/home/yara/system/models/motion_model_3d_00-07-079-06-2024.tflite";
//     auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
//     if (!model) {
//         std::cerr << "Failed to load model from file: " << model_path << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     tflite::ops::builtin::BuiltinOpResolver resolver;
//     std::unique_ptr<tflite::Interpreter> interpreter;
//     tflite::InterpreterBuilder builder(*model, resolver);
//     if (builder(&interpreter) != kTfLiteOk) {
//         std::cerr << "Failed to build interpreter for model" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     if (interpreter->AllocateTensors() != kTfLiteOk) {
//         std::cerr << "Failed to allocate tensors" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     while (true) {
//         // Lock mutex
//         if (flock(mutex_fd, LOCK_EX) == -1) {
//             perror("Failed to lock mutex");
//             break;
//         }

//         // Read from shared memory
//         char* data = static_cast<char*>(addr);
//         if (strlen(data) > 0) {
//             std::cout << "Data read from shared memory: " << std::endl;
//             std::cout << data << std::endl;

//             std::vector<float> data_vec;
//             std::istringstream ss(data);
//             std::string token;
//             while (std::getline(ss, token, ',')) {
//                 try {
//                     data_vec.push_back(std::stof(token));
//                 } catch (const std::invalid_argument& e) {
//                     std::cerr << "Invalid data encountered in shared memory: " << token << std::endl;
//                     continue;
//                 }
//             }

//             // Normalize the data
//             normalize(data_vec);

//             // Reshape the data into a 2D matrix
//             std::vector<std::vector<float>> reshaped_data = reshape_row(data_vec);

//             // Prepare input tensor
//             float* input = interpreter->typed_input_tensor<float>(0);
//             if (input == nullptr) {
//                 std::cerr << "Failed to get input tensor" << std::endl;
//                 break;
//             }
//             for (size_t i = 0; i < reshaped_data.size(); ++i) {
//                 for (size_t j = 0; j < reshaped_data[i].size(); ++j) {
//                     input[i * reshaped_data[i].size() + j] = reshaped_data[i][j];
//                 }
//             }

//             // Invoke the interpreter
//             if (interpreter->Invoke() != kTfLiteOk) {
//                 std::cerr << "Failed to invoke interpreter" << std::endl;
//                 break;
//             }

//             // Get the output
//             float* output = interpreter->typed_output_tensor<float>(0);
//             if (output == nullptr) {
//                 std::cerr << "Failed to get output tensor" << std::endl;
//                 break;
//             }

//             std::vector<int> prediction(5);
//             for (size_t i = 0; i < 5; ++i) {
//                 prediction[i] = static_cast<int>(output[i]);
//             }

//             // Write the output to shared memory
//             if (flock(output_mutex_fd, LOCK_EX) == -1) {
//                 perror("Failed to lock output mutex");
//                 break;
//             }

//             // Ensure the memory being accessed is valid and within bounds
//             if (OUTPUT_SHARED_MEMORY_SIZE >= sizeof(int) * 5) {
//                 std::memcpy(output_addr, prediction.data(), sizeof(int) * 5);
//                 std::cout << "Successfully copied to output shared memory" << std::endl;
//             } else {
//                 std::cerr << "Output shared memory size is too small" << std::endl;
//                 break;
//             }

//             std::cout << "Written to output shared memory: ";
//             for (int val : prediction) {
//                 std::cout << val << " ";
//             }
//             std::cout << std::endl;

//             if (flock(output_mutex_fd, LOCK_UN) == -1) {
//                 perror("Failed to unlock output mutex");
//                 break;
//             }

//             // Clear the input shared memory to avoid re-processing the same data
//             std::memset(addr, 0, SHARED_MEMORY_SIZE);
//             std::cout << "Cleared input shared memory" << std::endl;
//         }

//         // Unlock mutex
//         if (flock(mutex_fd, LOCK_UN) == -1) {
//             perror("Failed to unlock mutex");
//             break;
//         }

//         // Sleep for a short interval before reading again
//         std::this_thread::sleep_for(std::chrono::milliseconds(25));
//         std::cout << "Loop iteration completed" << std::endl;
//     }
// }



// #include "eeg_motion.h"
// #include <iostream>
// #include <sys/mman.h>
// #include <sys/file.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <cstring>
// #include <sstream>
// #include <vector>
// #include <algorithm>
// #include <chrono>
// #include <thread>
// #include <csignal>
// #include <tensorflow/lite/interpreter.h>
// #include <tensorflow/lite/kernels/register.h>
// #include <tensorflow/lite/model.h>

// #define SHARED_MEMORY_PATH "/dev/shm/csv_to_cpp_shm"
// #define SHARED_MEMORY_SIZE 600
// #define MUTEX_PATH "/home/yara/system_motion/src/csv_to_cpp_mutex"

// // Output shared memory
// #define OUTPUT_SHARED_MEMORY_PATH_MOTION "/dev/shm/cpp_to_cpp_shm"
// #define OUTPUT_SHARED_MEMORY_SIZE_MOTION sizeof(int) * 5  // 5 integers
// #define OUTPUT_MUTEX_PATH_MOTION "/home/yara/system_motion/src/motion_output_mutex"

// int fd_motion = -1;
// int mutex_fd_motion = -1;
// void* addr_motion = MAP_FAILED;
// int output_fd_motion = -1;
// int output_mutex_fd_motion = -1;
// void* output_addr_motion = MAP_FAILED;

// void cleanup_motion() {
//     // Clean up output shared memory and mutex
//     if (output_addr_motion != MAP_FAILED) munmap(output_addr_motion, OUTPUT_SHARED_MEMORY_SIZE_MOTION);
//     if (output_fd_motion != -1) close(output_fd_motion);
//     if (output_mutex_fd_motion != -1) close(output_mutex_fd_motion);

//     // Unlink output shared memory and mutex files
//     shm_unlink(OUTPUT_SHARED_MEMORY_PATH_MOTION);
//     unlink(OUTPUT_MUTEX_PATH_MOTION);
// }

// void signal_handler_motion(int signal) {
//     std::cout << "Interrupt signal (" << signal << ") received.\n";
//     cleanup_motion();
//     exit(signal);
// }

// void EEGMotion::normalize(std::vector<float>& values) {
//     float min_val = *std::min_element(values.begin(), values.end());
//     float max_val = *std::max_element(values.begin(), values.end());
//     float range = max_val - min_val;

//     if (range == 0) {
//         for (auto& value : values) {
//             value = 0.0f;
//         }
//     } else {
//         for (size_t i = 0; i < values.size(); ++i) {
//             values[i] = (values[i] - min_val) / range;
//         }
//     }
// }

// std::vector<std::vector<float>> EEGMotion::reshape_row(const std::vector<float>& row) {
//     std::vector<std::vector<float>> electrode_matrix = {
//         { 0, 0, 0, row[0], 0, 0, row[13], 0, 0, 0 },
//         { 0, row[1], 0, 0, row[2], row[11], 0, 0, row[12], 0 },
//         { 0, 0, row[3], 0, 0, 0, 0, row[10], 0, 0 },
//         { row[4], 0, 0, 0, 0, 0, 0, 0, 0, row[9] },
//         { 0, 0, row[5], 0, 0, 0, 0, row[8], 0, 0 },
//         { 0, 0, 0, 0, row[6], row[7], 0, 0, 0, 0 }
//     };

//     return electrode_matrix;
// }

// void EEGMotion::run() {
//     // Register signal handler
//     signal(SIGINT, signal_handler_motion);

//     // Open shared memory for input
//     fd_motion = open(SHARED_MEMORY_PATH, O_RDWR);
//     if (fd_motion == -1) {
//         perror("Failed to open shared memory for input");
//         return;
//     }

//     // Open mutex for synchronization
//     mutex_fd_motion = open(MUTEX_PATH, O_RDWR);
//     if (mutex_fd_motion == -1) {
//         perror("Failed to open mutex for input");
//         close(fd_motion);
//         return;
//     }

//     // Map shared memory to process address space
//     addr_motion = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_motion, 0);
//     if (addr_motion == MAP_FAILED) {
//         perror("Failed to map shared memory for input");
//         close(fd_motion);
//         close(mutex_fd_motion);
//         return;
//     }

//     // Open output shared memory
//     output_fd_motion = open(OUTPUT_SHARED_MEMORY_PATH_MOTION, O_RDWR | O_CREAT, 0666);
//     if (output_fd_motion == -1) {
//         perror("Failed to open output shared memory");
//         munmap(addr_motion, SHARED_MEMORY_SIZE);
//         close(fd_motion);
//         close(mutex_fd_motion);
//         return;
//     }

//     if (ftruncate(output_fd_motion, OUTPUT_SHARED_MEMORY_SIZE_MOTION) == -1) {
//         perror("Failed to set size of output shared memory");
//         munmap(addr_motion, SHARED_MEMORY_SIZE);
//         close(fd_motion);
//         close(mutex_fd_motion);
//         close(output_fd_motion);
//         return;
//     }

//     // Open output mutex for synchronization
//     output_mutex_fd_motion = open(OUTPUT_MUTEX_PATH_MOTION, O_RDWR | O_CREAT, 0666);
//     if (output_mutex_fd_motion == -1) {
//         perror("Failed to open output mutex");
//         munmap(addr_motion, SHARED_MEMORY_SIZE);
//         close(fd_motion);
//         close(mutex_fd_motion);
//         close(output_fd_motion);
//         return;
//     }

//     // Map output shared memory to process address space
//     output_addr_motion = mmap(NULL, OUTPUT_SHARED_MEMORY_SIZE_MOTION, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd_motion, 0);
//     if (output_addr_motion == MAP_FAILED) {
//         perror("Failed to map output shared memory");
//         munmap(addr_motion, SHARED_MEMORY_SIZE);
//         close(fd_motion);
//         close(mutex_fd_motion);
//         close(output_fd_motion);
//         close(output_mutex_fd_motion);
//         return;
//     }

//     // Ensure output memory alignment
//     if (reinterpret_cast<uintptr_t>(output_addr_motion) % alignof(int) != 0) {
//         std::cerr << "Output shared memory is not properly aligned" << std::endl;
//         munmap(addr_motion, SHARED_MEMORY_SIZE);
//         close(fd_motion);
//         close(mutex_fd_motion);
//         munmap(output_addr_motion, OUTPUT_SHARED_MEMORY_SIZE_MOTION);
//         close(output_fd_motion);
//         close(output_mutex_fd_motion);
//         return;
//     }

//     // Load the TensorFlow Lite model
//     const char* model_path = "/home/yara/system_motion/models/motion_model_3d_00-07-079-06-2024.tflite";
//     auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
//     if (!model) {
//         std::cerr << "Failed to load model from file: " << model_path << std::endl;
//         munmap(addr_motion, SHARED_MEMORY_SIZE);
//         close(fd_motion);
//         close(mutex_fd_motion);
//         munmap(output_addr_motion, OUTPUT_SHARED_MEMORY_SIZE_MOTION);
//         close(output_fd_motion);
//         close(output_mutex_fd_motion);
//         return;
//     }

//     tflite::ops::builtin::BuiltinOpResolver resolver;
//     std::unique_ptr<tflite::Interpreter> interpreter;
//     tflite::InterpreterBuilder builder(*model, resolver);
//     if (builder(&interpreter) != kTfLiteOk) {
//         std::cerr << "Failed to build interpreter for model" << std::endl;
//         munmap(addr_motion, SHARED_MEMORY_SIZE);
//         close(fd_motion);
//         close(mutex_fd_motion);
//         munmap(output_addr_motion, OUTPUT_SHARED_MEMORY_SIZE_MOTION);
//         close(output_fd_motion);
//         close(output_mutex_fd_motion);
//         return;
//     }

//     if (interpreter->AllocateTensors() != kTfLiteOk) {
//         std::cerr << "Failed to allocate tensors" << std::endl;
//         munmap(addr_motion, SHARED_MEMORY_SIZE);
//         close(fd_motion);
//         close(mutex_fd_motion);
//         munmap(output_addr_motion, OUTPUT_SHARED_MEMORY_SIZE_MOTION);
//         close(output_fd_motion);
//         close(output_mutex_fd_motion);
//         return;
//     }

//     while (true) {
//         // Lock mutex
//         if (flock(mutex_fd_motion, LOCK_EX) == -1) {
//             perror("Failed to lock mutex");
//             break;
//         }

//         // Read from shared memory
//         char* data = static_cast<char*>(addr_motion);
//         if (strlen(data) > 0) {
//             std::cout << "Data read from shared memory: " << std::endl;
//             std::cout << data << std::endl;

//             std::vector<float> data_vec;
//             std::istringstream ss(data);
//             std::string token;
//             while (std::getline(ss, token, ',')) {
//                 try {
//                     data_vec.push_back(std::stof(token));
//                 } catch (const std::invalid_argument& e) {
//                     std::cerr << "Invalid data encountered in shared memory: " << token << std::endl;
//                     continue;
//                 }
//             }

//             // Normalize the data
//             normalize(data_vec);

//             // Reshape the data into a 2D matrix
//             std::vector<std::vector<float>> reshaped_data = reshape_row(data_vec);

//             // Prepare input tensor
//             float* input = interpreter->typed_input_tensor<float>(0);
//             if (input == nullptr) {
//                 std::cerr << "Failed to get input tensor" << std::endl;
//                 break;
//             }
//             for (size_t i = 0; i < reshaped_data.size(); ++i) {
//                 for (size_t j = 0; j < reshaped_data[i].size(); ++j) {
//                     input[i * reshaped_data[i].size() + j] = reshaped_data[i][j];
//                 }
//             }

//             // Invoke the interpreter
//             if (interpreter->Invoke() != kTfLiteOk) {
//                 std::cerr << "Failed to invoke interpreter" << std::endl;
//                 break;
//             }

//             // Get the output
//             float* output = interpreter->typed_output_tensor<float>(0);
//             if (output == nullptr) {
//                 std::cerr << "Failed to get output tensor" << std::endl;
//                 break;
//             }

//             std::vector<int> prediction(5);
//             for (size_t i = 0; i < 5; ++i) {
//                 prediction[i] = static_cast<int>(output[i]);
//             }

//             // Write the output to shared memory
//             if (flock(output_mutex_fd_motion, LOCK_EX) == -1) {
//                 perror("Failed to lock output mutex");
//                 break;
//             }

//             // Ensure the memory being accessed is valid and within bounds
//             if (OUTPUT_SHARED_MEMORY_SIZE_MOTION >= sizeof(int) * 5) {
//                 std::memcpy(output_addr_motion, prediction.data(), sizeof(int) * 5);
//                 std::cout << "Successfully copied to output shared memory" << std::endl;
//             } else {
//                 std::cerr << "Output shared memory size is too small" << std::endl;
//                 break;
//             }

//             std::cout << "Written to output shared memory: ";
//             for (int val : prediction) {
//                 std::cout << val << " ";
//             }
//             std::cout << std::endl;

//             if (flock(output_mutex_fd_motion, LOCK_UN) == -1) {
//                 perror("Failed to unlock output mutex");
//                 break;
//             }

//             // Clear the input shared memory to avoid re-processing the same data
//             std::memset(addr_motion, 0, SHARED_MEMORY_SIZE);
//             std::cout << "Cleared input shared memory" << std::endl;
//         }

//         // Unlock mutex
//         if (flock(mutex_fd_motion, LOCK_UN) == -1) {
//             perror("Failed to unlock mutex");
//             break;
//         }

//         // Sleep for a short interval before reading again
//         std::this_thread::sleep_for(std::chrono::milliseconds(25));
//         std::cout << "Loop iteration completed" << std::endl;
//     }

//     // Clean up
//     cleanup_motion();
// }






// // #include <iostream>
// // #include <sys/mman.h>
// // #include <sys/file.h>
// // #include <fcntl.h>
// // #include <unistd.h>
// // #include <cstring>
// // #include <sstream>
// // #include <vector>
// // #include <algorithm> // for std::min_element and std::max_element
// // #include <chrono> // for std::chrono
// // #include <thread> // for std::this_thread
// // #include <tensorflow/lite/interpreter.h>
// // #include <tensorflow/lite/kernels/register.h>
// // #include <tensorflow/lite/model.h>

// // #define SHARED_MEMORY_PATH "/dev/shm/csv_to_cpp_shm"
// // #define SHARED_MEMORY_SIZE 600
// // #define MUTEX_PATH "/home/yara/system_motion/csv_to_cpp_mutex"

// // // Output shared memory
// // #define OUTPUT_SHARED_MEMORY_PATH "/dev/shm/cpp_to_cpp_shm"
// // #define OUTPUT_SHARED_MEMORY_SIZE sizeof(int) * 5  // 5 integers
// // #define OUTPUT_MUTEX_PATH "/home/yara/system_motion/cpp_to_cpp_mutex"

// // void normalize(std::vector<float>& values);
// // std::vector<std::vector<float>> reshape_row(const std::vector<float>& row);

// // int main() {
// //     // Open shared memory for input
// //     int fd = open(SHARED_MEMORY_PATH, O_RDWR);
// //     if (fd == -1) {
// //         perror("Failed to open shared memory for input");
// //         return 1;
// //     }

// //     // Open mutex for synchronization
// //     int mutex_fd = open(MUTEX_PATH, O_RDWR);
// //     if (mutex_fd == -1) {
// //         perror("Failed to open mutex");
// //         close(fd);
// //         return 1;
// //     }

// //     // Map shared memory to process address space
// //     void* addr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
// //     if (addr == MAP_FAILED) {
// //         perror("Failed to map shared memory for input");
// //         close(fd);
// //         close(mutex_fd);
// //         return 1;
// //     }

// //     // Open output shared memory
// //     int output_fd = open(OUTPUT_SHARED_MEMORY_PATH, O_RDWR | O_CREAT, 0666);
// //     if (output_fd == -1) {
// //         perror("Failed to open output shared memory");
// //         munmap(addr, SHARED_MEMORY_SIZE);
// //         close(fd);
// //         close(mutex_fd);
// //         return 1;
// //     }

// //     if (ftruncate(output_fd, OUTPUT_SHARED_MEMORY_SIZE) == -1) {
// //         perror("Failed to set size of output shared memory");
// //         munmap(addr, SHARED_MEMORY_SIZE);
// //         close(fd);
// //         close(mutex_fd);
// //         close(output_fd);
// //         return 1;
// //     }

// //     // Open output mutex for synchronization
// //     int output_mutex_fd = open(OUTPUT_MUTEX_PATH, O_RDWR | O_CREAT, 0666);
// //     if (output_mutex_fd == -1) {
// //         perror("Failed to open output mutex");
// //         munmap(addr, SHARED_MEMORY_SIZE);
// //         close(fd);
// //         close(mutex_fd);
// //         close(output_fd);
// //         return 1;
// //     }

// //     // Map output shared memory to process address space
// //     void* output_addr = mmap(NULL, OUTPUT_SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);
// //     if (output_addr == MAP_FAILED) {
// //         perror("Failed to map output shared memory");
// //         munmap(addr, SHARED_MEMORY_SIZE);
// //         close(fd);
// //         close(mutex_fd);
// //         close(output_fd);
// //         close(output_mutex_fd);
// //         return 1;
// //     }

// //     // Ensure output memory alignment
// //     if (reinterpret_cast<uintptr_t>(output_addr) % alignof(int) != 0) {
// //         std::cerr << "Output shared memory is not properly aligned" << std::endl;
// //         munmap(addr, SHARED_MEMORY_SIZE);
// //         close(fd);
// //         close(mutex_fd);
// //         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
// //         close(output_fd);
// //         close(output_mutex_fd);
// //         return 1;
// //     }

// //     // Load the TensorFlow Lite model
// //     const char* model_path = "/home/yara/system_motion/data/motion_model_3d_00-07-079-06-2024.tflite";
// //     auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
// //     if (!model) {
// //         std::cerr << "Failed to load model from file: " << model_path << std::endl;
// //         munmap(addr, SHARED_MEMORY_SIZE);
// //         close(fd);
// //         close(mutex_fd);
// //         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
// //         close(output_fd);
// //         close(output_mutex_fd);
// //         return 1;
// //     }

// //     tflite::ops::builtin::BuiltinOpResolver resolver;
// //     std::unique_ptr<tflite::Interpreter> interpreter;
// //     tflite::InterpreterBuilder builder(*model, resolver);
// //     if (builder(&interpreter) != kTfLiteOk) {
// //         std::cerr << "Failed to build interpreter for model" << std::endl;
// //         munmap(addr, SHARED_MEMORY_SIZE);
// //         close(fd);
// //         close(mutex_fd);
// //         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
// //         close(output_fd);
// //         close(output_mutex_fd);
// //         return 1;
// //     }

// //     if (interpreter->AllocateTensors() != kTfLiteOk) {
// //         std::cerr << "Failed to allocate tensors" << std::endl;
// //         munmap(addr, SHARED_MEMORY_SIZE);
// //         close(fd);
// //         close(mutex_fd);
// //         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
// //         close(output_fd);
// //         close(output_mutex_fd);
// //         return 1;
// //     }

// //     while (true) {
// //         // Lock mutex
// //         if (flock(mutex_fd, LOCK_EX) == -1) {
// //             perror("Failed to lock mutex");
// //             break;
// //         }

// //         // Read from shared memory
// //         char* data = static_cast<char*>(addr);
// //         if (strlen(data) > 0) {
// //             std::cout << "Data read from shared memory: " << std::endl;
// //             std::cout << data << std::endl;

// //             std::vector<float> data_vec;
// //             std::istringstream ss(data);
// //             std::string token;
// //             while (std::getline(ss, token, ',')) {
// //                 try {
// //                     data_vec.push_back(std::stof(token));
// //                 } catch (const std::invalid_argument& e) {
// //                     std::cerr << "Invalid data encountered in shared memory: " << token << std::endl;
// //                     continue;
// //                 }
// //             }

// //             // Normalize the data
// //             normalize(data_vec);

// //             // Reshape the data into a 2D matrix
// //             std::vector<std::vector<float>> reshaped_data = reshape_row(data_vec);

// //             // Prepare input tensor
// //             float* input = interpreter->typed_input_tensor<float>(0);
// //             if (input == nullptr) {
// //                 std::cerr << "Failed to get input tensor" << std::endl;
// //                 break;
// //             }
// //             for (size_t i = 0; i < reshaped_data.size(); ++i) {
// //                 for (size_t j = 0; j < reshaped_data[i].size(); ++j) {
// //                     input[i * reshaped_data[i].size() + j] = reshaped_data[i][j];
// //                 }
// //             }

// //             // Invoke the interpreter
// //             if (interpreter->Invoke() != kTfLiteOk) {
// //                 std::cerr << "Failed to invoke interpreter" << std::endl;
// //                 break;
// //             }

// //             // Get the output
// //             float* output = interpreter->typed_output_tensor<float>(0);
// //             if (output == nullptr) {
// //                 std::cerr << "Failed to get output tensor" << std::endl;
// //                 break;
// //             }

// //             std::vector<int> prediction(5);
// //             for (size_t i = 0; i < 5; ++i) {
// //                 prediction[i] = static_cast<int>(output[i]);
// //             }

// //             // Write the output to shared memory
// //             if (flock(output_mutex_fd, LOCK_EX) == -1) {
// //                 perror("Failed to lock output mutex");
// //                 break;
// //             }

// //             // Ensure the memory being accessed is valid and within bounds
// //             if (OUTPUT_SHARED_MEMORY_SIZE >= sizeof(int) * 5) {
// //                 std::memcpy(output_addr, prediction.data(), sizeof(int) * 5);
// //                 std::cout << "Successfully copied to output shared memory" << std::endl;
// //             } else {
// //                 std::cerr << "Output shared memory size is too small" << std::endl;
// //                 break;
// //             }

// //             std::cout << "Written to output shared memory: ";
// //             for (int val : prediction) {
// //                 std::cout << val << " ";
// //             }
// //             std::cout << std::endl;

// //             if (flock(output_mutex_fd, LOCK_UN) == -1) {
// //                 perror("Failed to unlock output mutex");
// //                 break;
// //             }

// //             // Clear the input shared memory to avoid re-processing the same data
// //             std::memset(addr, 0, SHARED_MEMORY_SIZE);
// //             std::cout << "Cleared input shared memory" << std::endl;
// //         }

// //         // Unlock mutex
// //         if (flock(mutex_fd, LOCK_UN) == -1) {
// //             perror("Failed to unlock mutex");
// //             break;
// //         }

// //         // Sleep for a short interval before reading again
// //         std::this_thread::sleep_for(std::chrono::milliseconds(25));
// //         std::cout << "Loop iteration completed" << std::endl;
// //     }

// //     // Clean up
// //     munmap(addr, SHARED_MEMORY_SIZE);
// //     close(fd);
// //     close(mutex_fd);
// //     munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
// //     close(output_fd);
// //     close(output_mutex_fd);

// //     return 0;
// // }

// // void normalize(std::vector<float>& values) {
// //     float min_val = *std::min_element(values.begin(), values.end());
// //     float max_val = *std::max_element(values.begin(), values.end());
// //     float range = max_val - min_val;

// //     if (range == 0) {
// //         for (auto& value : values) {
// //             value = 0.0f;
// //         }
// //     } else {
// //         for (size_t i = 0; i < values.size(); ++i) {
// //             values[i] = (values[i] - min_val) / range;
// //         }
// //     }
// // }

// // std::vector<std::vector<float>> reshape_row(const std::vector<float>& row) {
// //     std::vector<std::vector<float>> electrode_matrix = {
// //         { 0, 0, 0, row[0], 0, 0, row[13], 0, 0, 0 },
// //         { 0, row[1], 0, 0, row[2], row[11], 0, 0, row[12], 0 },
// //         { 0, 0, row[3], 0, 0, 0, 0, row[10], 0, 0 },
// //         { row[4], 0, 0, 0, 0, 0, 0, 0, 0, row[9] },
// //         { 0, 0, row[5], 0, 0, 0, 0, row[8], 0, 0 },
// //         { 0, 0, 0, 0, row[6], row[7], 0, 0, 0, 0 }
// //     };

// //     return electrode_matrix;
// // }
