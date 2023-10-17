#!/usr/bin/env python3

import socket
import threading
import os
import math

HOST = os.getenv("HOST","tinycar01.local")
PORT = os.getenv("PORT", 55002)
IN_PORT = os.getenv("IN_PORT", 55003)

FRAME_METADATA = 0x00
FRAME_FRAGMENT = 0x01
BATTERY_STATUS = 0x02 

def voltage_to_percent(voltage):
    # LiPo has 3.7V nominal voltage, 4.2V max voltage and 3.0V min voltage (better not to go below 3.3V)
    # 3.3V = 0%
    # 4.2V = 100%
    discharge_curve_coefficient = 12
    full_voltage = 4.2
    percent = math.exp(-discharge_curve_coefficient * (full_voltage - voltage) / full_voltage) * 100
    return percent

def send_battery_request():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.sendto(b'\x04', (HOST, PORT))

def receive_battery_status():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = ("0.0.0.0", IN_PORT)
    s.bind(server_address)
    while True:
        data, addr = s.recvfrom(1024)
        if data[0] == BATTERY_STATUS:
            voltage = int.from_bytes(data[1:3], byteorder='big') / 10_000
            print(f"battery voltage: {voltage}")

if __name__ == "__main__":
    # recv_thread = threading.Thread(target=receive_battery_status)
    # recv_thread.start()
    # send_battery_request()
    print(voltage_to_percent(4.2))
    