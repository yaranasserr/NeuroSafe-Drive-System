#include <thread>
#include "eeg_motion.h"
#include "eeg_stress.h"
// #include "can_handler.h"
#include "ppg_stress.h"

int main() {
    EEGMotion eeg_motion;
    EEGStress eeg_stress;
    // // CanHandler can_handler;
    // PPGStress ppg_stress;

    // Create and start threads for EEGMotion, EEGStress, CanHandler, and PPGStress
    std::thread eeg_motion_thread(&EEGMotion::run, &eeg_motion);
    std::thread eeg_stress_thread(&EEGStress::run, &eeg_stress);
//    // std::thread can_handler_thread(&CanHandler::run, &can_handler);
//     std::thread ppg_stress_thread(&PPGStress::run, &ppg_stress);

    // Wait for all threads to finish
    eeg_motion_thread.join();
    eeg_stress_thread.join();
//    // can_handler_thread.join();
//     ppg_stress_thread.join();

    return 0;
}

// #include "eeg_stress.h"
// // #include "eeg_motion.h"
// int main() {
//     EEGStress eegStress;
//     eegStress.run();
//     // EEGMotion eegMotion;
//     // eegMotion.run();

//     // PPGStress ppgStress;
//     // ppgStress.run();

//     return 0;
// }


