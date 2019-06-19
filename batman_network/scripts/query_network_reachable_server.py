#!/usr/bin/env python

from batman_network.srv import *
import rospy
import sys 
import paramiko


def create_ping_command(ip_addr, count):
    ping_command = "ping " + ip_addr + " -c " + count
    
    return ping_command 

def run_command(ssh,command):
    ssh_stdin = ssh_stdout = ssh_stderr = None
    try:
        ssh_stdin, ssh_stdout, ssh_stderr = ssh.exec_command(command)
    except Exception as e:
        rospy.logerr("SSH exec_command error: {0}",format(e))

    data_buffer = ""
    if ssh_stdout:
        data_buffer = ssh_stdout.read()
    if ssh_stderr:
        sys.stderr.write(ssh_stdout.read())
    return data_buffer



def handle_network_reachable(req):
    router_user_name = "root"
    router_password = "P@ssw0rd"

    rospy.loginfo("called handle_network_reachable")
    rospy.loginfo("%s, req ip addr: %s", handle_network_reachable.__name__, req.dest_ip_addr)
    rospy.loginfo("%s, req ping timeout: %s", handle_network_reachable.__name__, req.ping_timeout)
    rospy.loginfo("%s, req ping count: %s", handle_network_reachable.__name__, req.ping_count)


    ping_command = create_ping_command(req.dest_ip_addr, req.ping_count)

    rospy.loginfo("ping command: %s", ping_command)

    # create ssh object
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    try:
        rospy.loginfo("sys.argv[1] %s", sys.argv[1])
        ssh.connect(sys.argv[1], username=router_user_name, password=router_password)     
    except Exception as e:
        rospy.loginfo("rats, no connection")
        rospy.logerr("SSH connection error: %s", e)
        return False
        #sys.stderr.write("SSH connection error: {0}".format(e))
        #sys.exit(2)

    print "here"
    output = run_command(ssh, ping_command)

    print "after run command, output"
    print output

    ssh.close()

    
    #print "Returning %s"%(handle_query_count.counter)
    return False


def network_reachable_server():
    rospy.init_node('network_reachable_server', log_level=rospy.DEBUG)
    
    if len(sys.argv) < 2:
        rospy.logfatal("missing argv[1], router ip address")
        exit(2)

    print "argv[1] " + sys.argv[1]

    s = rospy.Service('network_reachable', CommNodeReachable, handle_network_reachable)
    print "ready to handle network reachable"
    rospy.spin()


if __name__ == "__main__":

    numargs = len(sys.argv)
    print "numargs " + str(numargs)
    network_reachable_server()

