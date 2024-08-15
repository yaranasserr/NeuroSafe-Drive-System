# Remote Physiological Measurement with PhysFormer

## Overview

This project utilizes **PhysFormer**, a deep learning-based transformer architecture designed for remote photoplethysmography (rPPG) measurement. PhysFormer is particularly suited for non-contact detection of physiological signals such as heart rate (HR) from facial videos. By leveraging a transformer network, PhysFormer is able to capture long-range spatio-temporal information, which significantly enhances the accuracy of rPPG measurement even under challenging conditions like head movements and varying illumination.

## PhysFormer in My Work

In my work, I implemented the PhysFormer model to measure physiological signals (e.g., heart rate) from facial video data. The transformer architecture allowed for the effective extraction of subtle rPPG features by learning long-range dependencies across frames. I used this model as part of a larger pipeline to improve the non-invasive measurement of vital signs.

The main contributions of PhysFormer in this project include:
- **Global Spatio-temporal Attention**: Adaptively aggregates local and global spatio-temporal features.
- **Temporal Difference Transformer**: Enhances quasi-periodic rPPG features and reduces noise.
- **No Pretraining Required**: Can be trained from scratch on rPPG datasets without the need for large-scale dataset pretraining.

## Features

- **End-to-End Transformer Model**: An end-to-end transformer architecture specifically for remote physiological measurement.
- **Temporal Difference Multi-head Self-Attention (TD-MHSA)**: Enhances subtle temporal rPPG signals by focusing on fine-grained local temporal differences.
- **Spatio-temporal Feed-forward (ST-FF)**: Refines local inconsistencies and provides richer feature representation.

## Installation

To run the PhysFormer model in your own projects, first clone the original repository from [here](https://github.com/ZitongYu/PhysFormer).

```bash
git clone https://github.com/ZitongYu/PhysFormer.git
cd PhysFormer
```

Then, install the required dependencies:

```bash
pip install -r requirements.txt
```

## Usage

You can train the PhysFormer model on your own rPPG datasets by using the following command:

```bash
python train.py --config configs/physformer_config.yaml
```

To evaluate the model on your test data, use:

```bash
python test.py --config configs/physformer_config.yaml
```

## Dataset

The PhysFormer model can be trained and evaluated on standard rPPG datasets such as VIPL-HR, MAHNOB-HCI, and MMSE-HR. Ensure that your dataset is preprocessed according to the instructions in the PhysFormer repository.

## Acknowledgment

This work is based on the PhysFormer model, as presented in the paper:

> **PhysFormer: Facial Video-based Physiological Measurement with Temporal Difference Transformer**  
> *Zitong Yu, Yuming Shen, Jingang Shi, Hengshuang Zhao, Philip Torr, Guoying Zhao*  
> [Paper Link](https://github.com/ZitongYu/PhysFormer)
