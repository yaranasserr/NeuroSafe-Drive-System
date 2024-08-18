#ifndef EEG_STRESS_H
#define EEG_STRESS_H

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
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>

class EEGStress {
public:
    void run();

private:
    void normalize(std::vector<float>& values);
    std::vector<std::vector<float>> reshape_row(const std::vector<float>& row);
};

#endif // EEG_STRESS_H
