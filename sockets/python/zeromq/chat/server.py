import zmq

context = zmq.Context()
socket = context.socket(zmq.REP)

#socket.bind("tcp://127.0.0.1:8888")
socket.bind("tcp://100.0.0.182:8888")

while True:
    msg = socket.recv()
    print ('From client: ', msg)
    smsg = raw_input("enter message: ")
    socket.send(smsg)
    print ('')