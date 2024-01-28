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
RTP_PORT = os.getenv("PORT", 4998)
TCCP_PORT = os.getenv("TCCP_PORT", 55002)

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

def handle_rtp():
    while True:
        data = data_queue.get()
        # get marker from RTP header
        marker = (data[1] & 0x80) >> 7
        sequence_number = int.from_bytes(data[2:4], byteorder='big')
        timestamp = int.from_bytes(data[4:8], byteorder='big')
        fragment_offset = int.from_bytes(data[12:16], byteorder='big')
        width = int(data[18])
        height = int(data[19])
        frame_buffer = data[20:]
        if DEBUG > 1:
            print(f"RTP: marker: {marker}, sequence_number: {sequence_number}, timestamp: {timestamp}, fragment_offset: {fragment_offset}, width: {width}, height: {height}")
        



def rtp_receive():
    # Create a UDP socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind the socket to the port
    server_address = ("0.0.0.0", RTP_PORT)
    s.bind(server_address)

    while True:
        data, addr = s.recvfrom(1024)
        if data is not None:
            data_queue.put(data)

def tccp_receive():
    # Create a UDP socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind the socket to the port
    server_address = ("0.0.0.0", TCCP_PORT)
    s.bind(server_address)

    while True:
        data, addr = s.recvfrom(1024)
        if data is not None:
            fps = int.from_bytes(data[3:4], byteorder='big')
            min_latency = int.from_bytes(data[4:6], byteorder='big')
            print(f"Congestion Control Telemetry: fps: {fps}, min_latency: {min_latency}")

def send_control_packet():
    # construct control packet
    control_packet = bytearray(5)
    control_packet[0] = 0x01 << 7 # version

    # send control packet
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        server_address = (HOST, TCCP_PORT)
        s.sendto(control_packet, server_address)

def main():
    handle_thread = threading.Thread(target=handle_rtp)
    handle_thread.start()

    receive_thread = threading.Thread(target=rtp_receive)
    receive_thread.start()

    receive_tccp_thread = threading.Thread(target=tccp_receive)
    receive_tccp_thread.start()

        
    send_control_packet()

if __name__ == "__main__":
    main()