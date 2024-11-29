import cv2
import numpy as np
import tensorflow as tf
import threading
import torch
from TorchLossComputer import TorchLossComputer

# Function to preprocess each frame for the TensorFlow Lite model
def preprocess_frame(frame):
    processed_frame = cv2.resize(frame, (128, 128))  # Resize frame to match TensorFlow model input size
    processed_frame = cv2.cvtColor(processed_frame, cv2.COLOR_BGR2GRAY)  # Convert to grayscale
    processed_frame = np.expand_dims(processed_frame, axis=-1)  # Add channel dimension
    return processed_frame

# Function to perform TensorFlow Lite model inference for both rPPG and HR
def calculate_rPPG(rppg_interpreter, frames, result):
    processed_frames = np.array([preprocess_frame(f) for f in frames])  # (160, 128, 128, 1)
    processed_frames = np.repeat(processed_frames[..., np.newaxis], 3, axis=-1)  # (160, 128, 128, 1, 3)
    processed_frames = processed_frames.squeeze(-2)  # (160, 128, 128, 3)
    input_data = processed_frames.transpose((3, 0, 1, 2))  # (3, 160, 128, 128)
    input_data = input_data.reshape((1, 3, 160, 128, 128))  # (1, 3, 160, 128, 128)
    # Convert to float32 and normalize
    input_data = input_data.astype('float32') / 255.0
    rppg_input_details = rppg_interpreter.get_input_details()
    rppg_output_details = rppg_interpreter.get_output_details()
    rppg_interpreter.set_tensor(rppg_input_details[0]['index'], input_data)
    rppg_interpreter.invoke()
    rppg_output = rppg_interpreter.get_tensor(rppg_output_details[0]['index'])
    # Convert rppg_output to a torch tensor
    rppg_output = torch.from_numpy(rppg_output.flatten()).float()
    HR_predicted = TorchLossComputer.cross_entropy_power_spectrum_forward_pred(rppg_output, 30) + 40
    result['rPPG'] = rppg_output.mean().item()
    result['HR'] = HR_predicted.item()

# Function to perform TensorFlow Lite model inference
def Prediction_from_rPPG(interpreter, frames, result):
    processed_frames = np.array([preprocess_frame(f) for f in frames])
    input_data = processed_frames.mean(axis=(1, 2)).reshape((1, 160, 1)).astype('float32')
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()
    result['tf_prediction'] = interpreter.get_tensor(output_details[0]['index'])

# Main function to run the application
def main():
    # Load the rPPG TFLite model
    rppg_interpreter = tf.lite.Interpreter(model_path='/home/yara/Desktop/GP/71-20241105T115730Z-001/71/model30.tflite')
    rppg_interpreter.allocate_tensors()
    # Load the TFLite model
    interpreter = tf.lite.Interpreter(model_path='/home/yara/Desktop/GP/71-20241105T115730Z-001/71/Prediction.tflite')
    interpreter.allocate_tensors()
    cap = cv2.VideoCapture(0)
    # Set higher camera resolution for better quality
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)  # Full HD width
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)  # Full HD height
    frame_rate = 30  # Assuming a frame rate of 30 FPS
    clip_size = 160
    frames = []
    result = {'rPPG': 0, 'HR': 0, 'tf_prediction': 0}
    # Set the window size
    cv2.namedWindow('Webcam', cv2.WINDOW_NORMAL)
    cv2.resizeWindow('Webcam', 1280, 720)  # Adjust width and height as needed
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        # Keep the original resolution for displaying
        display_frame = frame.copy()
        # Resize frame for processing
        processed_frame = cv2.resize(frame, (128, 128))
        frames.append(processed_frame)
        if len(frames) == clip_size:
            # Create and start threads for PyTorch and TensorFlow calculations
            rppg_thread = threading.Thread(target=calculate_rPPG, args=(rppg_interpreter, np.array(frames), result))
            prediction_thread = threading.Thread(target=Prediction_from_rPPG, args=(interpreter, frames, result))
            rppg_thread.start()
            prediction_thread.start()
            rppg_thread.join()
            prediction_thread.join()
            frames = []  # Reset frames after processing
        # Check the shape and type of tf_prediction
        if isinstance(result['tf_prediction'], np.ndarray):
            prediction_text = f"Prediction: {result['tf_prediction'][0][0]}" if result['tf_prediction'].size > 0 else "Prediction: N/A"
        else:
            prediction_text = f"Prediction: {result['tf_prediction']}"
        # Display HR and TensorFlow model prediction on the frame
        cv2.putText(display_frame, f"HR: {result['HR']:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
        cv2.putText(display_frame, prediction_text, (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
        cv2.imshow('Webcam', display_frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
