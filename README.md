# NeuroSafe Drive

## Introduction

NeuroSafe Drive integrates advanced technologies to address critical challenges in driver safety and mobility. Our dual-focused project encompasses a brain-computer interface (BCI) for wheelchair control and a hybrid BCI and PPG system for stress detection.

A Brain-Computer Interface (BCI) enables direct communication between the brain and external devices by translating brain activity into commands. Using EEG (electroencephalography) sensors, our BCI system reads brain signals and translates them into control commands for a wheelchair. This technology provides individuals with severe mobility impairments—such as those with spinal cord injuries or severe neurological disorders—with greater independence and mobility.

Our research highlighted that many disabilities result from accidents, often influenced by the driver’s health and cognitive state. To address this, we developed a system that continuously monitors and assesses a driver’s health and mental condition. This includes stress detection using EEG, heart monitoring with PPG, and tracking respiration rate, blood oxygen levels, and heart rate. Real-time data is displayed on a mobile app, which also provides alerts for any detected abnormalities.

We have successfully expanded BCI technology from its traditional use in assistive devices to practical applications in the automotive industry, using it as ground truth for health monitoring and mental state detection. Our system is designed to be sensor-free for enhanced driver comfort, utilizing remote PPG (rPPG) through advanced camera and computer vision techniques. rPPG is a non-contact method that measures heart rate using a camera to detect blood flow changes in the skin.

## Project Milestones

- **BCI for Disabled Individuals**: Developed a BCI system for wheelchair control to enhance the mobility and independence of individuals with disabilities.
- **BCI for Mental Health Monitoring**: Expanded the use of BCI technology to monitor and assess mental health conditions.
- **Integration of PPG Signals**: Incorporated PPG signals to monitor heart-related metrics and improve overall health monitoring.
- **EEG and PPG for Stress Detection**: Combined EEG and PPG signals for accurate stress detection, achieving a classification accuracy of 99%.
- **Sensor-Free System with rPPG**: Developed a sensor-free system relying on rPPG, using BCI as ground truth for automotive applications to enhance driver comfort. Our BCI model for wheelchair control, with 5 classes (forward, backward, right, left, stop), achieved a classification accuracy of 92%.

## Key Components

- **Emotiv Headset**: For high-resolution EEG data acquisition.
- **Max30102 Sensor**: For accurate PPG signal measurement.
- **Raspberry Pi**: For real-time data processing and system integration.
- **BCI Technology**: For precise wheelchair control and integration into automotive applications.
- **Custom ARM Drivers and STM Microcontrollers**: For robust wheelchair control.
- **CAN Communication Protocols**: For efficient data exchange and control.

## Project Achievements

- Engineered a wheelchair control system utilizing BCI technology, effectively emulating the vehicle control experience for enhanced user autonomy and functionality.
- Advanced BCI technology from assistive devices to practical automotive applications, expanding its use beyond traditional contexts.
- Developed a robust software system using shared memory and mutex synchronization on Raspberry Pi for efficient data management and inter-node communication, utilizing inter-process communication (IPC) mechanisms.
- Created a real-time API for EEG data acquisition from the BCI headset.
- Optimized system performance by converting deep learning models from Python to TensorFlow Lite C++ for real-time execution on edge devices.
- Integrated embedded systems with custom ARM drivers and STM microcontrollers, leveraging CAN protocols for precise and responsive wheelchair control.

## How the System Works
- ![NeuroSafe Drive system](https://github.com/user-attachments/assets/53662205-fd2e-4c3e-8318-f3b47e58a36b)


The NeuroSafe Drive system integrates multiple components to ensure effective real-time data processing, model prediction, and communication, all executed on a Raspberry Pi 5 platform. The system is divided into key sections:

### 1. **BCI (Brain-Computer Interface)**
   - **Function:** Acquires EEG (Electroencephalography) data for classification.
   - **Process:** The headset captures EEG data, which is then classified by AI models optimized and converted from TensorFlow to TensorFlow Lite. The classified data is transmitted as control commands to a wheelchair or used for stress detection.

### 2. **PPG (Photoplethysmography) Sensor**
   - **Function:** Monitors physiological signals, including heart rate, respiration rate, and blood oxygen levels, for stress detection.
   - **Process:** PPG data, along with EEG data, is processed for stress detection. The results, including heart rate, respiration rate, blood oxygen levels, and stress status, are sent over WebSocket to a mobile app for real-time monitoring and alerts.

### 3. **Wheelchair Control**
   - **Function:** Receives control commands based on classified EEG data.
   - **Process:** The control commands derived from the classified EEG data are sent to the wheelchair, enabling individuals with severe mobility impairments to control their movement.

### 4. **Inter-Process Communication (IPC)**
   - **Shared Memory:** Facilitates concurrent access to a designated memory region by multiple processes, enabling swift data exchange without the overhead of data duplication.
   - **Mutex (Mutual Exclusion):** Ensures exclusive access to shared resources, preventing simultaneous data access that could lead to conflicts and compromise system stability.

### 5. **System Integration on Raspberry Pi**
   - **Data Flow:** 
     - **Data Acquisition from Headset:** A Python script interfaces with the headset's API to capture raw EEG data streams, saving them into a CSV file formatted with 14 columns of floating-point data.
     - **Initial Processing and Prediction:**
       - The Python script reads data sequentially from the CSV file, processing each line and transmitting it to the first C++ script via a named pipe.
       - The first C++ script receives the data, processes it through a TensorFlow Lite model, and sends the prediction output to the CAN script via shared memory or another named pipe.
     - **CAN Communication:**
       - The CAN script receives predictions from the first C++ script and sends a serialized list of 5 uint8 elements to the STM over the CAN bus.
       - It also captures real-time sensor data from the STM and sends it to the second C++ script.
     - **Final Processing and Output:**
       - The second C++ script receives sensor data, processes it through another TensorFlow Lite model, and outputs the final prediction.

### 6. **Health Monitoring Data**
   - **WebSocket Communication:** 
     - Health status data, including stress detection, heart rate, respiration rate, and blood oxygen levels, is transmitted over WebSocket to a mobile app.
     - The mobile app displays real-time data and provides alerts for any detected abnormalities, contributing to driver safety and well-being.



## How to Run the Project
### Instructions to Run the Project with TensorFlow Lite

1. **Install TensorFlow Lite:**
   ``
   pip install tflite-runtime
``
2. **Build the Project with CMake:**
``mkdir build
cd build
cmake .. ``

3. **Compile the Project:**
`` make ``

## Datasets used 

[Access the datasets used ](https://drive.google.com/drive/folders/1in0-NH6YI6k8q6PYpGbGeM-kdwV5GCh_)

## Resources

[Access the book ](https://drive.google.com/drive/folders/1SQYw4Wmmh6DEhTUPfxT3nn4LdtVrDM_i)




## Acknowledgments

This project was developed under the mentorship program of Valeo Automotive, whose guidance and support were invaluable. We also extend our gratitude to the Academy for Scientific Research and Technology for funding this project.

## Bibliography

The NeuroSafe Drive project was carried out by a dedicated team of students from the Electronics and Communication Department at Alexandria University, Egypt. The team specializes in AI, Embedded Systems, Software, and Robotics.

