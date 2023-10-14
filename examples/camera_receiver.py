#!/usr/bin/env python3

import numpy as np
import cv2
import socket
import time
import os
import threading
import queue

HOST = os.getenv("HOST","tinycar01.local")
DEBUG = int(os.getenv("DEBUG", 0))
CONTINOUS = int(os.getenv("STREAM", 0))
PORT = os.getenv("PORT", 55002)
CAM_PORT = os.getenv("CAM_PORT", 55003)
JPEG_QUALITY = int(os.getenv("JPEG_QUALITY", 58))
PIXELFORMAT = int(os.getenv("PIXELFORMAT", 8))

frame_buffer = None
frame_buffer_view = None
bytes_in_frame_buffer = 0
bytes_for_frame = 0
fragment_max_size = 1024 - 12
sended_tag = -1

FRAME_METADATA = 0x00
FRAME_FRAGMENT = 0x01
BATTERY_STATUS = 0x02

TIMEOUT = 0.1 # s to wait for a packet before discarding the frame
start_new_frame_time = 0
ended_frame_time = 0
last_imshow = 0


data_queue = queue.Queue()

def hex_dump_frame_buffer(frame_buffer, file_path):
    with open(file_path, 'w') as f:
        for i in range(0, len(frame_buffer), 16):
            hex_bytes = ' '.join(f'{b:02X}' for b in frame_buffer[i:i+16])
            ascii_chars = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in frame_buffer[i:i+16])
            f.write(f'{i:08X}  {hex_bytes:<48}  {ascii_chars}\n')

"""
Returns True if another frame should be requested
"""
def decode_jpeg_frame(frame_buffer):
    global last_imshow
    # Decode the JPEG data into a NumPy array
    img_array = np.frombuffer(frame_buffer, dtype=np.uint8)
    img = cv2.imdecode(img_array, cv2.IMREAD_COLOR)

    # Display the image using OpenCV
    if DEBUG:
        print(f"FPS: {1/(time.time()-last_imshow)}")
    last_imshow = time.time()
    cv2.imshow("Frame", img)
    if cv2.waitKey(1) == ord('q'):
        cv2.destroyAllWindows()
        return False
    return CONTINOUS


def handle_data():
    global frame_buffer, frame_buffer_view, sended_tag, bytes_in_frame_buffer, bytes_for_frame, last_fragment_time, start_new_frame_time, ended_frame_time
    while True:
        try:
            data = data_queue.get(timeout=TIMEOUT)

            if data[0] == FRAME_METADATA:
                # it is a metadata packet
                current_tag = int.from_bytes(data[1:5], byteorder='big')
                height = int.from_bytes(data[5:7], byteorder='big')
                width = int.from_bytes(data[7:9], byteorder='big')
                pixelformat = int(data[9])
                frame_buf_size = int.from_bytes(data[10:14], byteorder='big')
                if DEBUG > 2:
                    print(f"tag: {current_tag}, height: {height}, width: {width}, pixelformat: {pixelformat}, frame_buf_size: {frame_buf_size}")
                if current_tag != sended_tag:
                    if DEBUG:
                        print("received metadata for a frame we did not request")
                    continue
                # allocating frame buffer
                frame_buffer = bytearray(frame_buf_size)
                #frame_buffer_view = memoryview(frame_buffer)
                bytes_for_frame = frame_buf_size
                bytes_in_frame_buffer = 0
            elif data[0] == FRAME_FRAGMENT:
                fragment_tag = int.from_bytes(data[1:5], byteorder='big')
                fragment_index = int.from_bytes(data[5:9], byteorder='big')
                fragment_size = int.from_bytes(data[9:11], byteorder='big')
                if DEBUG > 2:
                    print(f"fragment_tag: {fragment_tag}, fragment_index: {fragment_index}, fragment_size: {fragment_size}")
                if fragment_tag == sended_tag:
                    # it is possible that fragments arrive out of order
                    # we need to buffer them until we have all of them
                    begin = fragment_index*fragment_max_size
                    end = begin + fragment_size
                    frame_buffer[begin:end] = data[11:11+fragment_size]
                    bytes_in_frame_buffer += fragment_size
                    if bytes_in_frame_buffer == bytes_for_frame:
                        ended_frame_time = time.time()
                        if DEBUG > 1:
                            took = ended_frame_time - start_new_frame_time
                            print(f"frame received in {took*1000} ms with {bytes_for_frame} bytes")
                        if decode_jpeg_frame(frame_buffer):
                            request_frame(sended_tag + 1)
            else:
                if DEBUG:
                    print(f"unknown packet type: {data[0]}")

        except queue.Empty:
            # Handle the case where the queue is empty
            # if last frame is not complete, we discard it
            if bytes_in_frame_buffer != bytes_for_frame:
                if DEBUG:
                    print("discarding incomplete frame")
                bytes_in_frame_buffer = 0
                bytes_for_frame = 0
                if CONTINOUS:
                    request_frame(sended_tag+1)
                continue

def receive_frame():
    # Create a UDP socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind the socket to the port
    server_address = ("0.0.0.0", CAM_PORT)
    s.bind(server_address)

    if DEBUG:
        print(f"listening for incoming messages")

    while True:
        data, addr = s.recvfrom(1024)
        data_queue.put(data)

def request_frame(tag, ):
    global sended_tag, start_new_frame_time
    sended_tag = tag
    start_new_frame_time = time.time()
    # send a udp packet to the car to start the camera stream
    if DEBUG > 1:
        print(f"requesting frame tag: {tag}")
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.sendto(b'\x03' + tag.to_bytes(4, byteorder='little'), (HOST, PORT))

def request_camera_config():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.sendto(b'\x04' + PIXELFORMAT.to_bytes(1, byteorder='little') + JPEG_QUALITY.to_bytes(1, byteorder='little'), (HOST, PORT))


def main():
    handle_thread = threading.Thread(target=handle_data)
    handle_thread.start()

    receive_thread = threading.Thread(target=receive_frame)
    receive_thread.start()
    time.sleep(1)
    #request_camera_config() # very buggy
    time.sleep(1)
    request_frame(0)
    
                

if __name__ == "__main__":
    main()