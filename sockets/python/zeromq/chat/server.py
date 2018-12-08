import zmq

context = zmq.Context()
socket = context.socket(zmq.REP)

# uncomment to test on localhost ip
#socket.bind("tcp://127.0.0.1:8888")

# bind the server to any interface with wildcard *
# port 8888
socket.bind("tcp://*:8888")

while True:
    msg = socket.recv()
    print ('From client: ', msg)
    smsg = raw_input("enter message: ")
    socket.send(smsg)
    print ('')