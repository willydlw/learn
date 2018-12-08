import zmq

context = zmq.Context()
socket = context.socket(zmq.REQ)
#socket.connect("tcp://127.0.0.1:8888")
socket.connect("tcp://10.0.0.182:8888")

while True:

    msg = raw_input("enter msg here: ")
    socket.send(msg)
    print ('sent: ', msg)
    print ('From server: ', socket.recv())
    print ('')