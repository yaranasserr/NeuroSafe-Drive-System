#ifndef PPG_STRESS_H
#define PPG_STRESS_H

#include <iostream>
#include <sys/mman.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>

// #define SHARED_MEMORY_PATH "/dev/shm/ppg_to_cpp_shm"
// #define SHARED_MEMORY_SIZE 400
// #define MUTEX_PATH "/home/yara/system_motion/ppg_to_cpp_mutex"

struct DataRow {
    std::vector<float> features;
    int label;
};

class PPGStress {
public:
    PPGStress();
    ~PPGStress();
    bool initializeSharedMemory();
    bool loadModel(const char* model_path);
    void run();

private:
    void processBatch();
    void readDataFromSharedMemory();

    int fd;
    int mutex_fd;
    void* addr;
    std::vector<DataRow> data_vec;
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
    int batch_start_index;
};

#endif // PPG_STRESS_H
