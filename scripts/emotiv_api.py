import csv
import os
import mmap
import fcntl
import time
import signal
import sys

# Use the full paths for shared memory and mutex
SHARED_MEMORY_PATH = "/dev/shm/csv_to_cpp_shm"
SHARED_MEMORY_SIZE = 600  # Adjusted size
MUTEX_NAME = "/home/yara/system_motion/src/csv_to_cpp_mutex"

SHARED_MEMORY_PATH_2 = "/dev/shm/stress_csv_to_cpp_shm"
SHARED_MEMORY_SIZE_2 = 600  # Adjusted size
MUTEX_NAME_2 = "/home/yara/system_motion/src/stress_csv_to_cpp_mutex"

CSV_FILE_PATH = "/home/yara/system_motion/data/wheelchair_subset.csv"

mm = None
mutex_fd = None
mm2 = None
mutex_fd2 = None

def create_shared_memory(shared_memory_path, shared_memory_size, mutex_name):
    # Ensure the directory exists
    os.makedirs(os.path.dirname(shared_memory_path), exist_ok=True)
    print(f"Creating shared memory at {shared_memory_path}")

    # Create shared memory
    fd = os.open(shared_memory_path, os.O_CREAT | os.O_TRUNC | os.O_RDWR)
    os.write(fd, b'\x00' * shared_memory_size)
    mm = mmap.mmap(fd, shared_memory_size, access=mmap.ACCESS_WRITE)
    print("Shared memory created and initialized")

    print(f"Creating mutex at {mutex_name}")
    # Create mutex
    mutex_fd = open(mutex_name, 'w')
    print("Mutex created")

    return mm, mutex_fd

def cleanup(signal_received=None, frame=None):
    global mm, mutex_fd, mm2, mutex_fd2
    print("Cleaning up...")
    if mm:
        mm.close()
        os.remove(SHARED_MEMORY_PATH)
        print("Shared memory removed")
    if mutex_fd:
        mutex_fd.close()
        os.remove(MUTEX_NAME)
        print("Mutex removed")
    if mm2:
        mm2.close()
        os.remove(SHARED_MEMORY_PATH_2)
        print("Shared memory 2 removed")
    if mutex_fd2:
        mutex_fd2.close()
        os.remove(MUTEX_NAME_2)
        print("Mutex 2 removed")
    sys.exit(0)

def read_csv(file_path, mm, mutex_fd, mm2, mutex_fd2):
    # Check if the CSV file exists
    if not os.path.exists(file_path):
        print(f"CSV file not found at {file_path}")
        return

    print(f"Reading CSV file from {file_path}")
    with open(file_path, mode='r') as file:
        reader = csv.reader(file)
        for row in reader:
            line = ','.join(row)
            print(f"Read row: {line}")
            time.sleep(0.5) 

            # Write to shared memory 1
            mm.seek(0)
            print("Locking mutex")
            fcntl.flock(mutex_fd, fcntl.LOCK_EX)
            mm.write(line.encode('utf-8'))
            mm.flush()
            print(f"Written to shared memory: {line}")
            print("Unlocking mutex")
            fcntl.flock(mutex_fd, fcntl.LOCK_UN)

            # Write to shared memory 2
            mm2.seek(0)
            print("Locking mutex 2")
            fcntl.flock(mutex_fd2, fcntl.LOCK_EX)
            mm2.write(line.encode('utf-8'))
            mm2.flush()
            print(f"Written to shared memory 2: {line}")
            print("Unlocking mutex 2")
            fcntl.flock(mutex_fd2, fcntl.LOCK_UN)

            time.sleep(0.01)  # Small delay to mimic real-time

    print("Closing shared memory and mutex")
    cleanup()

if __name__ == "__main__":
    signal.signal(signal.SIGINT, cleanup)
    mm, mutex_fd = create_shared_memory(SHARED_MEMORY_PATH, SHARED_MEMORY_SIZE, MUTEX_NAME)
    mm2, mutex_fd2 = create_shared_memory(SHARED_MEMORY_PATH_2, SHARED_MEMORY_SIZE_2, MUTEX_NAME_2)
    read_csv(CSV_FILE_PATH, mm, mutex_fd, mm2, mutex_fd2)
    print("Script completed")
