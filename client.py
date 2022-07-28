import socket

HOST = "127.0.0.1"
PORT = 8080

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b"/home/fuato1/c/network/test.txt\n")
    data = s.recv(1024)

print("received: ", repr(data))