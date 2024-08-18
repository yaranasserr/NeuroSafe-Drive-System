import csv
import os
import mmap
import fcntl
import time
import signal
import sys

# Use the full paths for shared memory and mutex
SHARED_MEMORY_PATH = "/dev/shm/ppg_to_cpp_shm"
SHARED_MEMORY_SIZE = 4 * 100 # Adjusted size
MUTEX_NAME = "/home/yara/system_motion/ppg_to_cpp_mutex"
CSV_FILE_PATH = "/home/yara/system_motion/data/filtered_combined_subjects_98_per.csv"

mm = None
mutex_fd = None

def create_shared_memory():
    global mm, mutex_fd

    # Ensure the directory exists
    os.makedirs(os.path.dirname(SHARED_MEMORY_PATH), exist_ok=True)
    print(f"Creating shared memory at {SHARED_MEMORY_PATH}")

    # Create shared memory
    fd = os.open(SHARED_MEMORY_PATH, os.O_CREAT | os.O_TRUNC | os.O_RDWR)
    os.write(fd, b'\x00' * SHARED_MEMORY_SIZE)
    mm = mmap.mmap(fd, SHARED_MEMORY_SIZE, access=mmap.ACCESS_WRITE)
    print("Shared memory created and initialized")

    print(f"Creating mutex at {MUTEX_NAME}")
    # Create mutex
    mutex_fd = open(MUTEX_NAME, 'w')
    print("Mutex created")

    return mm, mutex_fd

def cleanup(signal_received=None, frame=None):
    global mm, mutex_fd
    print("Cleaning up...")
    if mm:
        mm.close()
        os.remove(SHARED_MEMORY_PATH)
        print("Shared memory removed")
    if mutex_fd:
        mutex_fd.close()
        os.remove(MUTEX_NAME)
        print("Mutex removed")
    sys.exit(0)

def read_csv(file_path, mm, mutex_fd):
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
            mm.seek(0)

            # Lock mutex
            print("Locking mutex")
            fcntl.flock(mutex_fd, fcntl.LOCK_EX)

            mm.write(line.encode('utf-8'))
            mm.flush()
            print(f"Written to shared memory: {line}")

            # Unlock mutex
            print("Unlocking mutex")
            fcntl.flock(mutex_fd, fcntl.LOCK_UN)

            time.sleep(0.01)  # Small delay to mimic real-time

    print("Closing shared memory and mutex")
    cleanup()

if __name__ == "__main__":
    # signal.signal(signal.SIGINT, cleanup)
    mm, mutex_fd = create_shared_memory()
    read_csv(CSV_FILE_PATH, mm, mutex_fd)
    print("Script completed")
# import csv
# import os
# import mmap
# import fcntl
# import time
# import signal
# import sys

# # Use the full paths for shared memory and mutex
# SHARED_MEMORY_PATH = "/dev/shm/csv_to_cpp_shm"
# SHARED_MEMORY_SIZE = 600
# MUTEX_NAME = "/home/yara/system_motion/csv_to_cpp_mutex"
# CSV_FILE_PATH = "/home/yara/system_motion/data/unlabelled_motion_data.csv"

# mm = None
# mutex_fd = None

# def create_shared_memory():
#     global mm, mutex_fd

#     # Ensure the directory exists
#     os.makedirs(os.path.dirname(SHARED_MEMORY_PATH), exist_ok=True)
#     print(f"Creating shared memory at {SHARED_MEMORY_PATH}")

#     # Create shared memory
#     fd = os.open(SHARED_MEMORY_PATH, os.O_CREAT | os.O_TRUNC | os.O_RDWR)
#     os.write(fd, b'\x00' * SHARED_MEMORY_SIZE)
#     mm = mmap.mmap(fd, SHARED_MEMORY_SIZE, access=mmap.ACCESS_WRITE)
#     print("Shared memory created and initialized")

#     print(f"Creating mutex at {MUTEX_NAME}")
#     # Create mutex
#     mutex_fd = open(MUTEX_NAME, 'w')
#     print("Mutex created")

#     return mm, mutex_fd

# def cleanup(signal_received=None, frame=None):
#     global mm, mutex_fd
#     print("Cleaning up...")
#     if mm:
#         mm.close()
#         os.remove(SHARED_MEMORY_PATH)
#         print("Shared memory removed")
#     if mutex_fd:
#         mutex_fd.close()
#         os.remove(MUTEX_NAME)
#         print("Mutex removed")
#     sys.exit(0)

# def read_csv(file_path, mm, mutex_fd):
#     # Check if the CSV file exists
#     if not os.path.exists(file_path):
#         print(f"CSV file not found at {file_path}")
#         return

#     print(f"Reading CSV file from {file_path}")
#     with open(file_path, mode='r') as file:
#         reader = csv.reader(file)
#         for row in reader:
#             line = ','.join(row)
#             print(f"Read row: {line}")
#             mm.seek(0)

#             # Lock mutex
#             print("Locking mutex")
#             fcntl.flock(mutex_fd, fcntl.LOCK_EX)

#             mm.write(line.encode('utf-8'))
#             mm.flush()
#             print(f"Written to shared memory: {line}")

#             # Unlock mutex
#             print("Unlocking mutex")
#             fcntl.flock(mutex_fd, fcntl.LOCK_UN)

#             time.sleep(0.01)  # Small delay to mimic real-time

#     print("Closing shared memory and mutex")
#     cleanup()

# if __name__ == "__main__":
#     signal.signal(signal.SIGINT, cleanup)
#     mm, mutex_fd = create_shared_memory()
#     read_csv(CSV_FILE_PATH, mm, mutex_fd)
#     print("Script completed")
