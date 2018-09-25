# http://pymotw.com/2/select/

"""
    Python's select() function is a direct interface to the underlying operating system
    implementation. It monitors sockets, open files, and pipes (anything with a fileno()
    method that returns a valid file descriptor) until they become readable or writable,
    or a communication error occurs.

    select() makes it easier to monitor multiple connections
    at the same time, and is more efficient than writing a polling loop in Python using
    socket timeouts, because the monitoring happens in the operating system network layer,
    instead of the interpreter.

"""


# creating a non-blocking TCP/IP socket and configuring it to listen on an address.
import select
import socket
import sys
import Queue

# Create a TCP/IP socket
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setblocking(0)

# Bind the socket to the port
server_address = ('localhost', 10000)
print >>sys.stderr, 'starting up on %s port %s' % server_address
server.bind(server_address)

# Listen for incoming connections
server.listen(5)

""" The arguments to select() are three lists containing communication channels to monitor.
    The first is a list of the objects to be checked for incoming data to be read, the
    second contains objects that will receive outgoing data when there is room in their
    buffer, and the third those that may have an error (usually a combination of the input
    and output channel objects). The next step in the server is to set up the lists containing
    input sources and output destinations to be passed to select().
"""

# Sockets from which we expect to read
inputs = [ server ]

# Sockets to which we expect to write
outputs = [ ]

""" Connections are added to and removed from these lists by the server main loop. Since
    this version of the server is going to wait for a socket to become writable before
    sending any data (instead of immediately sending the reply), each output connection
    needs a queue to act as a buffer for the data to be sent through it.
"""

# Outgoing message queues (socket:Queue)
message_queues = {}

# The main portion of the server program loops, calling select() to block and wait for network activity.
while inputs:

    # Wait for at least one of the sockets to be ready for processing
    print >>sys.stderr, '\nwaiting for the next event'
    # set timeout to zero for blocking
    timeout = 1
    readable, writable, exceptional = select.select(inputs, outputs, inputs, timeout)

    if not (readable or writable or exceptional):
        print >>sys.stderr, '  timed out, do some other work here'
        continue

    """ The readable sockets represent three possible cases. If the socket is the main
        server socket, the one being used to listen for connections, then the readable
        condition means it is ready to accept another incoming connection. In addition
        to adding the new connection to the list of inputs to monitor, this section sets
        the client socket to not block.
    """

     # Handle inputs
    for s in readable:

        if s is server:
            # A "readable" server socket is ready to accept a connection
            connection, client_address = s.accept()
            print >>sys.stderr, 'new connection from', client_address
            connection.setblocking(0)
            inputs.append(connection)

            # Give the connection a queue for data we want to send
            message_queues[connection] = Queue.Queue()

            """ The next case is an established connection with a client that has sent data.
            The data is read with recv(), then placed on the queue so it can be sent
            through the socket and back to the client.
            """
        else:
            data = s.recv(1024)
            if data:
                # A readable client socket has data
                print >>sys.stderr, 'received "%s" from %s' % (data, s.getpeername())
                message_queues[s].put(data)
                # Add output channel for response
                if s not in outputs:
                    outputs.append(s)

                """ A readable socket without data available is from a client that has disconnected,
                    and the stream is ready to be closed.
                """
            else:
                # Interpret empty result as closed connection
                print >>sys.stderr, 'closing', client_address, 'after reading no data'
                # Stop listening for input on the connection
                if s in outputs:
                    outputs.remove(s)
                inputs.remove(s)
                s.close()

                # Remove message queue
                del message_queues[s]

    """ There are fewer cases for the writable connections. If there is data in the queue for
        a connection, the next message is sent. Otherwise, the connection is removed from the
        list of output connections so that the next time through the loop select() does not
        indicate that the socket is ready to send data.
    """
    # Handle outputs
    for s in writable:
        try:
            next_msg = message_queues[s].get_nowait()
        except Queue.Empty:
            # No messages waiting so stop checking for writability.
            print >>sys.stderr, 'output queue for', s.getpeername(), 'is empty'
            outputs.remove(s)
        else:
            print >>sys.stderr, 'sending "%s" to %s' % (next_msg, s.getpeername())
            s.send(next_msg)

    """ Finally, if there is an error with a socket, it is closed.
    """
    # Handle "exceptional conditions"
    for s in exceptional:
        print >>sys.stderr, 'handling exceptional condition for', s.getpeername()
        # Stop listening for input on the connection
        inputs.remove(s)
        if s in outputs:
            outputs.remove(s)
        s.close()

        # Remove message queue
        del message_queues[s]


