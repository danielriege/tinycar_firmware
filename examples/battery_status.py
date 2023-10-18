#!/usr/bin/env python3

import socket
import threading
import os
import math

HOST = os.getenv("HOST","tinycar01.local")
PORT = os.getenv("PORT", 55002)
IN_PORT = os.getenv("IN_PORT", 55003)

FRAME_METADATA = 0x01
FRAME_FRAGMENT = 0x02
BATTERY_STATUS = 0x03 

def voltage_to_percent(voltage):
    # LiPo has 3.7V nominal voltage, 4.2V max voltage and 3.0V min voltage (better not to go below 3.3V)
    # 3.3V = 0%
    # 4.2V = 100%
    # 3.85V = 50%
    # use non linear function
    return int(100 * (math.exp(voltage - 3.3) - 1) / (math.exp(4.2 - 3.3) - 1))

def send_battery_request():
    print(f"requesting battery status from {HOST}:{PORT}")
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.sendto(b'\x05', (HOST, PORT))
    print("battery request sent")

def receive_battery_status():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = ("0.0.0.0", IN_PORT)
    s.bind(server_address)
    while True:
        data, addr = s.recvfrom(1024)
        if data[0] == BATTERY_STATUS:
            voltage = int.from_bytes(data[1:3], byteorder='little') / 1000 # from mV to V
            percent = voltage_to_percent(voltage)
            print(f"battery voltage: {voltage} V, {percent} %")
            return
        else:
            print(f"received unknown frame type: {data[0]}")

if __name__ == "__main__":
    recv_thread = threading.Thread(target=receive_battery_status)
    recv_thread.start()
    send_battery_request()
    