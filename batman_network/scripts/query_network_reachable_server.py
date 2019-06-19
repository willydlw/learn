#!/usr/bin/env python

from batman_network.srv import CommNodeReachable
import rospy
import sys 
import paramiko
import re


def create_ping_command(ip_addr, count = "3", timeout = "2"):
    # -W Time to wait for a response, in seconds. The option affects  
    # only  timeout  in  absence  of  any responses, otherwise ping waits for two RTTs
    ping_command = "ping " + ip_addr + " -c " + count +" -W " + timeout 
    
    # filter everything except statistics and the line below
    ping_command += " | grep transmitted -A1"

    print ping_command
    
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


def extract_avg_time(thelist):

    avg_time_val = float(-1)

    # string 'min/avg/max' only exists when ping is successful
    try:
        index = thelist.index('min/avg/max')
        thelen = len(thelist)

        # find first and second / 
        if index <= thelen-2:
            avg_time_str = thelist[index+2]
            pos1 = avg_time_str.find('/')
            pos2 = avg_time_str.find('/', pos1+1)
            
            if pos1 != -1 and pos2 != -1:
                try:
                    avg_time_val = float(avg_time_str[pos1+1:pos2])
                except Exception as e:
                    rospy.logerr("failed to convert avg_time_str: %s to float: %s", avg_time_str, e)
            else:
                rospy.logwarn("failed to find / in avg_time_str: %s", avg_time_str)   
    except ValueError as e:
        # term not found in the list, packet loss must be 100%
        pass 

    return avg_time_val


def extract_packet_loss(thelist):
    
    packet_loss_val = float(-1)

    try:
        index = thelist.index('loss')
        if index >= 2:
            packet_loss_str = thelist[index-2]

            # remove % from packet loss string
            pos1 = packet_loss_str.find("%")
            if pos1 != -1:
                packet_loss_str = packet_loss_str[:pos1]
            try:
                packet_loss_val = float(packet_loss_str)
            except Exception as e:
                rospy.logerr("failed to convert packet_loss_str: %s to float: %s", packet_loss, e)
    except ValueError as e:
        rospy.logwarn("failed to find 'loss' in the list")

    return packet_loss_val
    



def handle_network_reachable(req):
    router_user_name = "root"
    router_password = "P@ssw0rd"

    packet_loss_val = -1
    average_time_val = -1

    rospy.loginfo("called handle_network_reachable")
    rospy.loginfo("%s, req ip addr: %s", handle_network_reachable.__name__, req.dest_ip_addr)

    ping_command = create_ping_command(req.dest_ip_addr)

    rospy.loginfo("ping command: %s", ping_command)

    # create ssh object
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    try:
        rospy.loginfo("sys.argv[1] %s", sys.argv[1])
        ssh.connect(sys.argv[1], username=router_user_name, password=router_password)     
    except Exception as e:
        rospy.logerr("SSH connection error: %s", e)
        return packet_loss_val, average_time_val
        
    output = run_command(ssh, ping_command)

    if output:
        output_list = output.split()                # split list into strings
        packet_loss_val = extract_packet_loss(output_list)
        average_time_val = extract_avg_time(output_list)
       
    ssh.close()

    rospy.loginfo("packet loss %f, avg time %f", packet_loss_val, average_time_val)
    
    return packet_loss_val, average_time_val


def network_reachable_server():
    rospy.init_node('network_reachable_server', log_level=rospy.INFO)
    
    if len(sys.argv) < 2:
        rospy.logfatal("missing argv[1], router ip address")
        exit(2)

    s = rospy.Service('network_reachable', CommNodeReachable, handle_network_reachable)
    rospy.spin()


if __name__ == "__main__":
    network_reachable_server()

