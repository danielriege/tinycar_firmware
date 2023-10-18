#!/usr/bin/env python3

import socket
import threading
import os
import math

HOST = os.getenv("HOST","tinycar01.local")
PORT = os.getenv("PORT", 55002)
IN_PORT = os.getenv("IN_PORT", 55003)

SERVO_CTRL = 0x06

def send_servo_angle(angle):
    print(f"sending servo ctrl msg to {HOST}:{PORT} with angle: {angle}")
    angle = angle * 100
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.sendto(b'\x06' + angle.to_bytes(2, byteorder='little'), (HOST, PORT))
    print("angle sent")

if __name__ == "__main__":
    while True:
        angle = int(input("enter servo angle: "))
        send_servo_angle(angle)
    