# BCI Project

This project involves two main applications utilizing Brain-Computer Interface (BCI) technology:

1. **Motion Control**
2. **Stress Detection**

## Applications Overview

### 1. Motion Control
This application is designed to interpret brain signals to control motion. The data source for this application is based on the paper *"Converting your thoughts to texts: Enabling brain typing via deep feature learning of EEG signals."* The corresponding dataset is available in the following GitHub repository: [Brain Typing GitHub Repository](https://github.com/xiangzhang1015/Brain_typing?tab=readme-ov-file).

In this application, the "center" label was not used. Additionally, data augmentation was applied to the original data to enhance the model's robustness. The following functions were used for augmentation:

```python
def add_noise(eeg_data, noise_level=0.05):
    noise = np.random.normal(0, noise_level, eeg_data.shape)
    noisy_data = eeg_data + noise
    return noisy_data

def add_baseline_drift(eeg_data, drift_level=0.5):
    drift = np.random.normal(0, drift_level, eeg_data.shape)
    drifted_data = eeg_data + drift
    return drifted_data
```
#### Labels for Motion Control:
The following actions are identified in the motion control application:

| Action ID | Action Name   |
|-----------|---------------|
| 1         | up            |
| 2         | down          |
| 3         | left          |
| 4         | right         |
| 5         | middle        |
| 6         | eye close     |

You can explore the implementation details in the following notebook: [Motion Control Notebook](https://github.com/NeuroSafe-Drive/NeuroSafe-Drive-System/blob/Brain-computer-Interface/motion.ipynb).

### 2. Stress Detection
The stress detection application analyzes brain signals to determine the stress levels of the user. The data for this application comes from the *STEW: Simultaneous Task EEG Workload Dataset,* which can be accessed through the following link: [STEW Dataset on IEEE DataPort](https://ieee-dataport.org/open-access/stew-simultaneous-task-eeg-workload-dataset).

#### Labels for Stress Detection:
The labels for the stress detection application are as follows:

- `0`: Unstressed
- `1`: Stressed

You can explore the implementation details in the following notebook: [Stress Detection Notebook](https://github.com/NeuroSafe-Drive/NeuroSafe-Drive-System/blob/Brain-computer-Interface/stress.ipynb).

## Data Sources and Preprocessing

Both datasets used in this project are preprocessed and provided in this [Google Drive link](https://drive.google.com/drive/folders/1d4sSMIGzQq_d-SoNrowMDIJ8cjvrVDNM).


