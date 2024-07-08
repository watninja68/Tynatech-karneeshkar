import socket

def read_sensor_data(ip, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((ip, port))
        while True:
            data = s.recv(1024)
            if not data:
                break
            print(type(data))

if __name__ == "__main__":
    read_sensor_data("10.10.100.100", 18899)
