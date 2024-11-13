#!/usr/bin/env python3

from argparse import ArgumentParser
import socket
import threading
import struct


def recv_task(sock):
    try:
        while True:
            data = sock.recv(1024)
            if not data:
                break
            print(data.decode('ascii').strip())
    except OSError:
        pass


def main():
    parser = ArgumentParser(description='OTA Flash')
    parser.add_argument('--ip', help='IP address of the device', default='192.168.4.1')
    parser.add_argument('--port', help='Port of the device', default=8888)
    parser.add_argument('--file', help='File to flash', default='build/nonna.bin')
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print('Connecting to {}:{}'.format(args.ip, args.port))
    sock.connect((args.ip, args.port))
    print('Sending file: {}'.format(args.file))

    recv_thread = threading.Thread(target=recv_task, args=(sock,))
    recv_thread.start()

    with open(args.file, 'rb') as f:
        file_size = f.seek(0, 2)
        f.seek(0)

        sock.send(struct.pack('I', file_size))

        written = 0
        progress_milestone = 0

        data = f.read(1024)
        while data:
            sock.send(data)
            written += len(data)
            progress = int(written / file_size * 100)
            if progress > progress_milestone:
                print('Progress: {}%'.format(progress))
                progress_milestone += 5
            data = f.read(1024)

    print('Upload complete, please wait... (or don\'t)')

    recv_thread.join()


if __name__ == '__main__':
    main()
