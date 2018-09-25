# directly from http://pymotw.com/2/select/

import select
import socket
import sys
import Queue



def main():

    # create a TCP/IP socket
    server = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
    server.setblocking(0)                           # non-blocking

    # Bind the socket to the port
    server_address = ('localhost', 8888)
    print >>sys.stderr, 'starting up on %s port %s' % server_address
    server.bind(server_address)

    # Listen for incoming connections, backlog queue size is 5
    server.listen(5)

    # Sockets from which we expect to read
    read_fd_set = [server]

    # Sockets to which we expect to write
    write_fd_set = []


    # message queue
    message_queue = {}

    # Do not block forever (milliseconds)
    TIMEOUT = 1000

    """ Commonly used polling flag settings
            POLLIN      input ready
            POLLPRI     priority input ready
            POLLHUP     channel closed
            POLLERR     error
            POLLOUT     able to receive output
    """
    READ_ONLY = select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR
    READ_WRITE = READ_ONLY | select.POLLOUT

    # Set up the poller
    poller = select.poll()
    # server socket is registered so that any incoming connections or data triggers an event.
    poller.register(server, READ_ONLY)

    """ Since poll() returns a list of tuples containing the file descriptor for the
        socket and the event flag, a mapping from file descriptor numbers to objects
        is needed to retrieve the socket to read or write from it.
    """
    # Map file descriptors to socket objects
    fd_to_socket = { server.fileno(): server,
                   }

    """ The server's loop calls poll(), then processes the events returned by looking
        up the socket and taking action based on the event flag.
    """
    while True:

        # Wait for at least one of the sockets to be ready for processing
        print >>sys.stderr, '\nwaiting for the next event'

        events = poller.poll(TIMEOUT)

        for fd, flag in events:

            # Retrieve the actual socket from its file descriptor
            s = fd_to_socket[fd]

            """ When the main server socket is readable, this means there is a
                pending client connections. The new connection is resgistered with
                the READ_ONLY flags to watch for new data to come from that client.
            """

            # Handle inputs
            if flag & (select.POLLIN | select.POLLPRI):

                if s is server:
                    # A "readable" server socket is ready to accept a connection
                    connection, client_address = s.accept()
                    print >>sys.stderr, 'new connection from', client_address
                    connection.setblocking(0)
                    fd_to_socket[ connection.fileno() ] = connection
                    poller.register(connection, READ_ONLY)

                    # Give the connection a queue for data we want to send
                    message_queues[connection] = Queue.Queue()

                    # Sockets other than the server are existing clients, and recv() is used to access the data waiting to be read.
                else:
                    data = s.recv(1024)

                    """ If recv() returns any data, it is placed into the outgoing queue for the
                        socket and the flags for that socket are changed using modify() so poll()
                        will watch for the socket to be ready to receive data.
                    """
                    if data:
                        # A readable client socket has data
                        print >>sys.stderr, 'received "%s" from %s' % (data, s.getpeername())
                        message_queues[s].put(data)
                        # Add output channel for response
                        poller.modify(s, READ_WRITE)

                        """ An empty string returned by recv() means the client disconnected, so
                            unregister() is used to tell the poll object to ignore the socket.
                        """

                    else:
                        # Interpret empty result as closed connection
                        print >>sys.stderr, 'closing', client_address, 'after reading no data'
                        # Stop listening for input on the connection
                        poller.unregister(s)
                        s.close()

                        # Remove message queue
                        del message_queues[s]

                """ POLLHUP flag indicates a client that hung up the connection without closing
                    it cleanly. Server stops polling clients that disappear.
                """
            elif flag & select.POLLHUP:
                # Client hung up
                print >>sys.stderr, 'closing', client_address, 'after receiving HUP'
                # Stop listening for input on the connection
                poller.unregister(s)
                s.close()

                """The handling for writable sockets looks like the version used in the example for select(),
                except that modify() is used to change the flags for the socket in the poller,
                instead of removing it from the output list.
                """

            elif flag & select.POLLOUT:
                # Socket is ready to send data, if there is any to send.
                try:
                    next_msg = message_queues[s].get_nowait()
                except Queue.Empty:
                    # No messages waiting so stop checking for writability.
                    print >>sys.stderr, 'output queue for', s.getpeername(), 'is empty'
                    poller.modify(s, READ_ONLY)
                else:
                    print >>sys.stderr, 'sending "%s" to %s' % (next_msg, s.getpeername())
                    s.send(next_msg)

            #  events with POLLERR cause the server to close the socket.

            elif flag & select.POLLERR:
                print >>sys.stderr, 'handling exceptional condition for', s.getpeername()
                # Stop listening for input on the connection
                poller.unregister(s)
                s.close()

                # Remove message queue
                del message_queues[s]


# Run main
if __name__ == "__main__":
    main()
