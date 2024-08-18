#include "eeg_stress.h"
#include <iostream>
#include <sys/mman.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <csignal>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>

#define SHARED_MEMORY_PATH_2 "/dev/shm/stress_csv_to_cpp_shm"
#define SHARED_MEMORY_SIZE_2 600
#define MUTEX_PATH_2 "/home/yara/system_motion/src/stress_csv_to_cpp_mutex"

// Output shared memory
#define OUTPUT_SHARED_MEMORY_PATH "/dev/shm/stress_out_to_cpp_shm"
#define OUTPUT_SHARED_MEMORY_SIZE sizeof(int) * 1
#define OUTPUT_MUTEX_PATH "/home/yara/system_motion/src/stress_out_to_cpp_mutex"

static int fd_stress = -1;
static int mutex_fd_stress = -1;
static void* addr_stress = MAP_FAILED;
static int output_fd_stress = -1;
static int output_mutex_fd_stress = -1;
static void* output_addr_stress = MAP_FAILED;

// static void cleanup_stress() {
//     if (addr_stress != MAP_FAILED) munmap(addr_stress, SHARED_MEMORY_SIZE_2);
//     if (fd_stress != -1) close(fd_stress);
//     if (mutex_fd_stress != -1) close(mutex_fd_stress);
//     if (output_addr_stress != MAP_FAILED) munmap(output_addr_stress, OUTPUT_SHARED_MEMORY_SIZE);
//     if (output_fd_stress != -1) close(output_fd_stress);
//     if (output_mutex_fd_stress != -1) close(output_mutex_fd_stress);

//     // Unlink shared memory and mutex files
//     shm_unlink(SHARED_MEMORY_PATH_2);
//     unlink(MUTEX_PATH_2);
//     shm_unlink(OUTPUT_SHARED_MEMORY_PATH);
//     unlink(OUTPUT_MUTEX_PATH);
// }
static void cleanup_stress() {
    // Clean up output shared memory and mutex
    if (output_addr_stress != MAP_FAILED) munmap(output_addr_stress, OUTPUT_SHARED_MEMORY_SIZE);
    if (output_fd_stress != -1) close(output_fd_stress);
    if (output_mutex_fd_stress != -1) close(output_mutex_fd_stress);

    // Unlink output shared memory and mutex files
    shm_unlink(OUTPUT_SHARED_MEMORY_PATH);
    unlink(OUTPUT_MUTEX_PATH);
}

static void signal_handler_stress(int signal) {
    std::cout << "Interrupt signal (" << signal << ") received.\n";
    cleanup_stress();
    exit(signal);
}

void EEGStress::normalize(std::vector<float>& values) {
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

std::vector<std::vector<float>> EEGStress::reshape_row(const std::vector<float>& row) {
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

void EEGStress::run() {
    // Register signal handler
    signal(SIGINT, signal_handler_stress);

    // Open additional shared memory for input
    fd_stress = open(SHARED_MEMORY_PATH_2, O_RDWR);
    if (fd_stress == -1) {
        perror("Failed to open shared memory for input");
        return;
    }

    // Open additional mutex for synchronization
    mutex_fd_stress = open(MUTEX_PATH_2, O_RDWR);
    if (mutex_fd_stress == -1) {
        perror("Failed to open mutex for input");
        close(fd_stress);
        return;
    }

    // Map additional shared memory to process address space
    addr_stress = mmap(NULL, SHARED_MEMORY_SIZE_2, PROT_READ | PROT_WRITE, MAP_SHARED, fd_stress, 0);
    if (addr_stress == MAP_FAILED) {
        perror("Failed to map shared memory for input");
        close(fd_stress);
        close(mutex_fd_stress);
        return;
    }

    // Open output shared memory
    output_fd_stress = open(OUTPUT_SHARED_MEMORY_PATH, O_RDWR | O_CREAT, 0666);
    if (output_fd_stress == -1) {
        perror("Failed to open output shared memory");
        munmap(addr_stress, SHARED_MEMORY_SIZE_2);
        close(fd_stress);
        close(mutex_fd_stress);
        return;
    }

    if (ftruncate(output_fd_stress, OUTPUT_SHARED_MEMORY_SIZE) == -1) {
        perror("Failed to set size of output shared memory");
        munmap(addr_stress, SHARED_MEMORY_SIZE_2);
        close(fd_stress);
        close(mutex_fd_stress);
        close(output_fd_stress);
        return;
    }

    // Open output mutex for synchronization
    output_mutex_fd_stress = open(OUTPUT_MUTEX_PATH, O_RDWR | O_CREAT, 0666);
    if (output_mutex_fd_stress == -1) {
        perror("Failed to open output mutex");
        munmap(addr_stress, SHARED_MEMORY_SIZE_2);
        close(fd_stress);
        close(mutex_fd_stress);
        close(output_fd_stress);
        return;
    }

    // Map output shared memory to process address space
    output_addr_stress = mmap(NULL, OUTPUT_SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd_stress, 0);
    if (output_addr_stress == MAP_FAILED) {
        perror("Failed to map output shared memory");
        munmap(addr_stress, SHARED_MEMORY_SIZE_2);
        close(fd_stress);
        close(mutex_fd_stress);
        close(output_fd_stress);
        close(output_mutex_fd_stress);
        return;
    }

    // Ensure output memory alignment
    if (reinterpret_cast<uintptr_t>(output_addr_stress) % alignof(int) != 0) {
        std::cerr << "Output shared memory is not properly aligned" << std::endl;
        munmap(addr_stress, SHARED_MEMORY_SIZE_2);
        close(fd_stress);
        close(mutex_fd_stress);
        munmap(output_addr_stress, OUTPUT_SHARED_MEMORY_SIZE);
        close(output_fd_stress);
        close(output_mutex_fd_stress);
        return;
    }

    // Load the TensorFlow Lite model
    const char* model_path = "/home/yara/system_motion/models/all_stress_model_3d_17-38-15_25-06-2024.tflite";
    auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
    if (!model) {
        std::cerr << "Failed to load model from file: " << model_path << std::endl;
        munmap(addr_stress, SHARED_MEMORY_SIZE_2);
        close(fd_stress);
        close(mutex_fd_stress);
        munmap(output_addr_stress, OUTPUT_SHARED_MEMORY_SIZE);
        close(output_fd_stress);
        close(output_mutex_fd_stress);
        return;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::InterpreterBuilder builder(*model, resolver);
    if (builder(&interpreter) != kTfLiteOk) {
        std::cerr << "Failed to build interpreter for model" << std::endl;
        munmap(addr_stress, SHARED_MEMORY_SIZE_2);
        close(fd_stress);
        close(mutex_fd_stress);
        munmap(output_addr_stress, OUTPUT_SHARED_MEMORY_SIZE);
        close(output_fd_stress);
        close(output_mutex_fd_stress);
        return;
    }

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        std::cerr << "Failed to allocate tensors" << std::endl;
        munmap(addr_stress, SHARED_MEMORY_SIZE_2);
        close(fd_stress);
        close(mutex_fd_stress);
        munmap(output_addr_stress, OUTPUT_SHARED_MEMORY_SIZE);
        close(output_fd_stress);
        close(output_mutex_fd_stress);
        return;
    }

    while (true) {
        // Lock mutex
        if (flock(mutex_fd_stress, LOCK_EX) == -1) {
            perror("Failed to lock mutex");
            break;
        }

        // Read from shared memory
        char* data = static_cast<char*>(addr_stress);
        if (strlen(data) > 0) {
          //std::cout << "Data read from shared memory: " << std::endl;
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

            std::vector<int> prediction(1);
            for (size_t i = 0; i < 1; ++i) {
                prediction[i] = static_cast<int>(output[i]);
            }

            // Write the output to shared memory
            if (flock(output_mutex_fd_stress, LOCK_EX) == -1) {
                perror("Failed to lock output mutex");
                break;
            }

            // Ensure the memory being accessed is valid and within bounds
            if (OUTPUT_SHARED_MEMORY_SIZE >= sizeof(int) * 1) {
                std::memcpy(output_addr_stress, prediction.data(), sizeof(int) * 1);
                std::cout << "Successfully copied to output shared memory" << std::endl;
            } else {
                std::cerr << "Output shared memory size is too small" << std::endl;
                break;
            }

            std::cout << "Written to output shared memory: ";
            for (int val : prediction) {
                std::cout << val << " ";
            }
            std::cout << std::endl;

            if (flock(output_mutex_fd_stress, LOCK_UN) == -1) {
                perror("Failed to unlock output mutex");
                break;
            }

            // Clear the input shared memory to avoid re-processing the same data
            std::memset(addr_stress, 0, SHARED_MEMORY_SIZE_2);
            std::cout << "Cleared input shared memory" << std::endl;
        }

        // Unlock mutex
        if (flock(mutex_fd_stress, LOCK_UN) == -1) {
            perror("Failed to unlock mutex");
            break;
        }

        // Sleep for a short interval before reading again
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      //  std::cout << "Loop iteration completed" << std::endl;
    }

    // Clean up
    cleanup_stress();
}



// output mutex is cleaned
// #include "eeg_stress.h"

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

// #define SHARED_MEMORY_PATH_2 "/dev/shm/stress_csv_to_cpp_shm"
// #define SHARED_MEMORY_SIZE_2 600
// #define MUTEX_PATH_2 "/home/yara/system_motion/src/stress_csv_to_cpp_mutex"

// // Output shared memory
// #define OUTPUT_SHARED_MEMORY_PATH "/dev/shm/stress_out_to_cpp_shm"
// #define OUTPUT_SHARED_MEMORY_SIZE sizeof(int) * 1
// #define OUTPUT_MUTEX_PATH "/home/yara/system_motion/src/stress_out_to_cpp_mutex"

// int fd = -1;
// int mutex_fd = -1;
// void* addr = MAP_FAILED;
// int output_fd = -1;
// int output_mutex_fd = -1;
// void* output_addr = MAP_FAILED;

// void cleanup() {
//     if (addr != MAP_FAILED) munmap(addr, SHARED_MEMORY_SIZE_2);
//     if (fd != -1) close(fd);
//     if (mutex_fd != -1) close(mutex_fd);
//     if (output_addr != MAP_FAILED) munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//     if (output_fd != -1) close(output_fd);
//     if (output_mutex_fd != -1) close(output_mutex_fd);

//     // Unlink shared memory and mutex files
//     shm_unlink(SHARED_MEMORY_PATH_2);
//     unlink(MUTEX_PATH_2);
//     shm_unlink(OUTPUT_SHARED_MEMORY_PATH);
//     unlink(OUTPUT_MUTEX_PATH);
// }

// void signal_handler(int signal) {
//     std::cout << "Interrupt signal (" << signal << ") received.\n";
//     cleanup();
//     exit(signal);
// }

// void EEGStress::normalize(std::vector<float>& values) {
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

// std::vector<std::vector<float>> EEGStress::reshape_row(const std::vector<float>& row) {
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

// void EEGStress::run() {
//     // Register signal handler
//     signal(SIGINT, signal_handler);

//     // Open additional shared memory for input
//     fd = open(SHARED_MEMORY_PATH_2, O_RDWR);
//     if (fd == -1) {
//         perror("Failed to open shared memory for input");
//         return;
//     }

//     // Open additional mutex for synchronization
//     mutex_fd = open(MUTEX_PATH_2, O_RDWR);
//     if (mutex_fd == -1) {
//         perror("Failed to open mutex for input");
//         close(fd);
//         return;
//     }

//     // Map additional shared memory to process address space
//     addr = mmap(NULL, SHARED_MEMORY_SIZE_2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         return;
//     }

//     if (ftruncate(output_fd, OUTPUT_SHARED_MEMORY_SIZE) == -1) {
//         perror("Failed to set size of output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Open output mutex for synchronization
//     output_mutex_fd = open(OUTPUT_MUTEX_PATH, O_RDWR | O_CREAT, 0666);
//     if (output_mutex_fd == -1) {
//         perror("Failed to open output mutex");
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Map output shared memory to process address space
//     output_addr = mmap(NULL, OUTPUT_SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);
//     if (output_addr == MAP_FAILED) {
//         perror("Failed to map output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Ensure output memory alignment
//     if (reinterpret_cast<uintptr_t>(output_addr) % alignof(int) != 0) {
//         std::cerr << "Output shared memory is not properly aligned" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Load the TensorFlow Lite model
//     const char* model_path = "/home/yara/system_motion/models/all_stress_model_3d_17-38-15_25-06-2024.tflite";
//     auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
//     if (!model) {
//         std::cerr << "Failed to load model from file: " << model_path << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE_2);
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
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     if (interpreter->AllocateTensors() != kTfLiteOk) {
//         std::cerr << "Failed to allocate tensors" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE_2);
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

//             std::vector<int> prediction(1);
//             for (size_t i = 0; i < 1; ++i) {
//                 prediction[i] = static_cast<int>(output[i]);
//             }

//             // Write the output to shared memory
//             if (flock(output_mutex_fd, LOCK_EX) == -1) {
//                 perror("Failed to lock output mutex");
//                 break;
//             }

//             // Ensure the memory being accessed is valid and within bounds
//             if (OUTPUT_SHARED_MEMORY_SIZE >= sizeof(int) * 1) {
//                 std::memcpy(output_addr, prediction.data(), sizeof(int) * 1);
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
//             std::memset(addr, 0, SHARED_MEMORY_SIZE_2);
//             std::cout << "Cleared input shared memory" << std::endl;
//         }

//         // Unlock mutex
//         if (flock(mutex_fd, LOCK_UN) == -1) {
//             perror("Failed to unlock mutex");
//             break;
//         }

//         // Sleep for a short interval before reading again
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         std::cout << "Loop iteration completed" << std::endl;
//     }

//     // Clean up
//     cleanup();
// }





// #include "eeg_stress.h"

// #define SHARED_MEMORY_PATH_2 "/dev/shm/stress_csv_to_cpp_shm"
// #define SHARED_MEMORY_SIZE_2 600
// #define MUTEX_PATH_2 "/home/yara/system_motion/src/stress_csv_to_cpp_mutex"

// // Output shared memory
// #define OUTPUT_SHARED_MEMORY_PATH "/dev/shm/stress_out_to_cpp_shm"
// #define OUTPUT_SHARED_MEMORY_SIZE sizeof(int) * 1
// #define OUTPUT_MUTEX_PATH "/home/yara/system_motion/src/stress_out_to_cpp_mutex"

// void EEGStress::run() {
//     // Open additional shared memory for input
//     int fd = open(SHARED_MEMORY_PATH_2, O_RDWR);
//     if (fd == -1) {
//         perror("Failed to open shared memory for input"); 
//         return;
//     }

//     // Open additional mutex for synchronization
//     int mutex_fd = open(MUTEX_PATH_2, O_RDWR);
//     if (mutex_fd == -1) {
//         perror("Failed to open mutex for input");
//         close(fd);
//         return;
//     }

//     // Map additional shared memory to process address space
//     void* addr = mmap(NULL, SHARED_MEMORY_SIZE_2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//     if (addr == MAP_FAILED) {
//         perror("Failed to map shared memory for input");
//         close(fd);
//         close(mutex_fd);
//         return;
//     }

//     // Open output shared memory
//     int output_fd = open(OUTPUT_SHARED_MEMORY_PATH, O_RDWR | O_CREAT, 0666);
//     if (output_fd == -1) {
//         perror("Failed to open output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         return;
//     }

//     if (ftruncate(output_fd, OUTPUT_SHARED_MEMORY_SIZE) == -1) {
//         perror("Failed to set size of output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Open output mutex for synchronization
//     int output_mutex_fd = open(OUTPUT_MUTEX_PATH, O_RDWR | O_CREAT, 0666);
//     if (output_mutex_fd == -1) {
//         perror("Failed to open output mutex");
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         return;
//     }

//     // Map output shared memory to process address space
//     void* output_addr = mmap(NULL, OUTPUT_SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);
//     if (output_addr == MAP_FAILED) {
//         perror("Failed to map output shared memory");
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Ensure output memory alignment
//     if (reinterpret_cast<uintptr_t>(output_addr) % alignof(int) != 0) {
//         std::cerr << "Output shared memory is not properly aligned" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     // Load the TensorFlow Lite model
//     const char* model_path = "/home/yara/system_motion/models/all_stress_model_3d_17-38-15_25-06-2024.tflite";
//     auto model = tflite::FlatBufferModel::BuildFromFile(model_path);
//     if (!model) {
//         std::cerr << "Failed to load model from file: " << model_path << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE_2);
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
//         munmap(addr, SHARED_MEMORY_SIZE_2);
//         close(fd);
//         close(mutex_fd);
//         munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//         close(output_fd);
//         close(output_mutex_fd);
//         return;
//     }

//     if (interpreter->AllocateTensors() != kTfLiteOk) {
//         std::cerr << "Failed to allocate tensors" << std::endl;
//         munmap(addr, SHARED_MEMORY_SIZE_2);
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

//             std::vector<int> prediction(1);
//             for (size_t i = 0; i < 2; ++i) {
//                 prediction[i] = static_cast<int>(output[i]);
//             }

//             // Write the output to shared memory
//             if (flock(output_mutex_fd, LOCK_EX) == -1) {
//                 perror("Failed to lock output mutex");
//                 break;
//             }

//             // Ensure the memory being accessed is valid and within bounds
//             if (OUTPUT_SHARED_MEMORY_SIZE >= sizeof(int) * 1) {
//                 std::memcpy(output_addr, prediction.data(), sizeof(int) * 1);
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
//             std::memset(addr, 0, SHARED_MEMORY_SIZE_2);
//             std::cout << "Cleared input shared memory" << std::endl;
//         }

//         // Unlock mutex
//         if (flock(mutex_fd, LOCK_UN) == -1) {
//             perror("Failed to unlock mutex");
//             break;
//         }

//         // Sleep for a short interval before reading again
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         std::cout << "Loop iteration completed" << std::endl;
//     }

//     // Clean up
//     munmap(addr, SHARED_MEMORY_SIZE_2);
//     close(fd);
//     close(mutex_fd);
//     munmap(output_addr, OUTPUT_SHARED_MEMORY_SIZE);
//     close(output_fd);
//     close(output_mutex_fd);
// }

// void EEGStress::normalize(std::vector<float>& values) {
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

// std::vector<std::vector<float>> EEGStress::reshape_row(const std::vector<float>& row) {
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
