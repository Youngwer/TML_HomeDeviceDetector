# TML_HomeDeviceDetector

# Home Device Detector

This project implements an embedded sound recognition system that assists Deaf and hard of hearing (DHH) individuals by detecting and classifying critical household sounds (smoke alarms and doorbells) and providing visual alerts through LEDs.

## Project Overview

The Home Device Detector addresses accessibility barriers for DHH individuals in smart homes by developing a TinyML-based embedded system that can:

- Detect critical household sound events in real-time
- Classify between normal ambient sounds, doorbell rings, and smoke alarms
- Provide immediate visual feedback through color-coded LED indicators
- Process audio locally for enhanced privacy and reduced latency

<img src="/img/device_photo.jpg" alt="Home Device Detector" style="zoom:25%;" />

### Features

- Real-time detection of 3 sound categories:
  - Normal environmental sounds
  - Doorbell rings
  - Smoke alarms
- Visual feedback through LED indicators:
  - Green for normal environment
  - Yellow for doorbell alerts
  - Red for smoke alarm detection
- Fast response time (within 2 seconds)
- Compact 3D-printed enclosure for easy placement
- Serial monitoring interface for debugging

## Hardware Requirements

- Arduino Nano 33 BLE Sense
- 3× LEDs (Green, Yellow, Red)
- 3× 220Ω resistors
- Breadboard/PCB for circuit connections
- 3D-printed enclosure (optional)
- USB cable for power and programming

## Software Dependencies

- Arduino IDE
- [PDM library](https://www.arduino.cc/en/Reference/PDM) (for microphone access)
- [Arduino_LSM9DS1 library](https://www.arduino.cc/en/Reference/ArduinoLSM9DS1)
- Edge Impulse exported model library for Home Device Detector

## Installation

1. Clone this repository:

   ```
   git clone https://github.com/Youngwer/TML_HomeDeviceDetector.git
   ```

2. Open the Arduino sketch (`home_device_detector.ino`) in the Arduino IDE.

3. Install the required libraries through the Arduino Library Manager:

   - PDM
   - Arduino_LSM9DS1

4. The Edge Impulse model is already included in the repository as the `HomeDeviceDetector_inferencing` library. If you want to modify or retrain the model:

   - Create an account on [Edge Impulse](https://studio.edgeimpulse.com/)
   - Follow the instructions in the report to recreate the model
   - Export the model as an Arduino library
   - Replace the existing model files in the repository

5. Connect the hardware components according to the circuit diagram:

   - Connect the Green LED to pin D2 through a 220Ω resistor
   - Connect the Yellow LED to pin D3 through a 220Ω resistor
   - Connect the Red LED to pin D4 through a 220Ω resistor

6. Upload the sketch to your Arduino Nano 33 BLE Sense.

## Usage

Once the system is powered on and initialized:

1. The device will continuously monitor ambient sounds through the onboard microphone.
2. When a sound event is detected, the corresponding LED will illuminate:
   - Green LED: Normal environmental sounds
   - Yellow LED: Doorbell detection (with distinctive blinking pattern)
   - Red LED: Smoke alarm detection (with rapid blinking pattern)
3. Connect to the serial monitor (115200 baud) to view detailed classification results and debugging information.

## System Architecture

The system is organized into several key components:

- **home_device_detector.ino** - Main program with setup and loop functions
- **audio_processing.h** - Audio capture and processing functions
- **inference.h** - ML model inference handling
- **led_control.h** - LED feedback control functions

### Data Processing Pipeline

1. Continuous audio sampling through the PDM microphone
2. Pre-processing of audio data (windowing, normalization)
3. Feature extraction using Mel-Frequency Energy (MFE) spectrograms
4. Inference using the quantized TensorFlow Lite model
5. Classification of sound events based on confidence thresholds
6. Visual feedback through LED indicators

## Model Training

The model was trained using Edge Impulse with the following workflow:

1. Data collection from multiple sources:
   - Online audio repositories for doorbell and smoke alarm sounds
   - Smartphone recordings for ambient environmental sounds

2. Dataset preparation with balanced class distribution:
   - Smoke alarm: 1 minute 42 seconds
   - Doorbell sounds: 1 minute 50 seconds
   - Normal environmental audio: 1 minute 56 seconds
   - 80:20 train/test split

3. Feature extraction using MFE spectrograms:
   - Window size: 1000ms
   - Window increase: 500ms
   - Sampling rate: 16kHz
   - 48 filters and 2048 FFT length

4. Neural network architecture:
   - Convolutional layers with optimized filter configurations
   - Dropout regularization to prevent overfitting
   - Dense layers for final classification

5. Model optimization and quantization:
   - Final accuracy: 94.9% (training), 90.32% (testing)
   - Model size optimized for Arduino deployment

## Calibration and Tuning

The system uses a confidence threshold parameter that can be adjusted in the code:

```cpp
#define CONFIDENCE_THRESHOLD 0.80f
```

This threshold determines how confident the model must be to trigger an alert. You can adjust this value (between 0.0 and 1.0) based on your specific environment to balance between sensitivity and false positives.

## Performance

The system:

- Samples audio continuously at 16kHz
- Processes audio in 1-second windows
- Delivers visual alerts within 2 seconds of sound detection
- Achieves over 90% accuracy in sound classification
- Uses approximately 65% of the Arduino's available memory

## Enclosure Design

The 3D-printed enclosure was designed using Fusion 360 and includes:

- Mounting points for the Arduino Nano 33 BLE Sense
- Openings for the USB connection
- Optimal positioning for the onboard microphone
- Diffusion windows for the LED indicators

STL files for the enclosure are included in the `/enclosure` folder.

## Troubleshooting

- If the LEDs don't illuminate, check your connections and resistor values
- If certain sounds aren't recognized correctly, try adjusting the confidence threshold
- If the device becomes unresponsive, check the serial monitor for memory-related errors
- Ensure the microphone opening in the enclosure isn't obstructed

## Future Improvements

- Add support for more household sound categories (microwave, kettle, etc.)
- Implement haptic feedback through vibration motors
- Add Bluetooth connectivity for smartphone notifications
- Develop adaptive audio filtering for noisy environments
- Include a battery power option for portable use

## Acknowledgments

- UCL Centre for Advanced Spatial Analysis (CASA)
- Edge Impulse for the embedded machine learning platform
- Arduino community for libraries and support
- Online sound repositories (Pixabay, Uppbeat) for training data

## Author

Wenhao Yang - [GitHub](https://github.com/Youngwer)

------

Project created as part of the CASA0018 - Deep Learning for Sensor Networks course at University College London.