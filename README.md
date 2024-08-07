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

## Repository Structure

*(Structure details go here)*

## How to Run the Project

*(Instructions on setting up and running the project go here)*

## Resources

[Access the book and demo video](https://drive.google.com/drive/folders/1SQYw4Wmmh6DEhTUPfxT3nn4LdtVrDM_i)

