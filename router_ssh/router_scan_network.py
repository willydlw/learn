''' Purpose: scan for cell phone artifact network

    ssh into router
    run iwinfo scan
        if cell phone network essid is detected, extract the signal and quality values

    return output
        output is empty when the network is not detected
        output contains two value when network is detected
            signal           string containing integer digits representing dBm
            quality          string of the form integer/integer

    Note: signal value will be negative.


    Example output for command: iwinfo wlan1 | grep "sparky" -A2

    ESSID: "sparky"
    Mode: Master  Channel: 6
    Signal: -31 dBm  Quality: 70/70

'''


import paramiko
import re
import routerConstant


'''
OpenWrt command to scan for a network ESSID and return the signal level is
of the form: iwinfo <interface name> scan | grep "network ESSID" -A2

Example: Scan for a network named "sparky". The wireless interface name is wlan0.
iwinfo wlan0 scan | grep "sparky" -A2


Some routers in our network have two wireless interfaces: wlan0, wlan1. The active interface
has ESSID: MARBLE, while the other is ESSID: unknown. 

The command iwinfo | grep "MARBLE" returns <interface name> ESSID: "MARBLE".
awk '{ print $1 }' extracts the interface name, the $() inserts it into the command 

SSH_COMMAND = "iwinfo | grep 'MARBLE' | iwinfo $(awk '{ print $1 }') scan | grep 'sparky' -A2

Example output for command: iwinfo wlan1 | grep "sparky" -A2

    ESSID: "sparky"
    Mode: Master  Channel: 6
    Signal: -31 dBm  Quality: 70/70

To simplify parsing this output, the additional commands: | awk '$1=="Signal:" && $4=="Quality:" {print $2,$5}' 
result in just the values -31 70/70

'''
def create_scan_command(our_essid, search_essid):
    extract_iface_name = 'iwinfo | grep ' + "'" + our_essid + "'"

    # scan_command = "iwinfo | grep 'MARBLE' | iwinfo $(awk '{ print $1 }') scan | grep 'sparky' -A2 | awk '$1=="Signal:" && $4=="Quality:" {print $2,$5}'

    scan_command = extract_iface_name + " | iwinfo $(awk '{print $1}') scan | grep " + "'" + search_essid + "' -A2 "

    # extract Signal and Quality value fields
    scan_command += "| awk '$1==" + '"Signal:" && $4=="Quality:" {print $2,$5}' + "'"

    print scan_command
    return scan_command


def run_command(ssh,command):
    ssh_stdin = ssh_stdout = ssh_stderr = None
    try:
        ssh_stdin, ssh_stdout, ssh_stderr = ssh.exec_command(command)
    except Exception as e:
        sys.stderr.write("ssh connection error: {0}".format(e))

    data_buffer = ""
    if ssh_stdout:
        data_buffer = ssh_stdout.read()
    if ssh_stderr:
        sys.stderr.write(ssh_stdout.read())
    return data_buffer



def main():

    SEARCH_FOR_ESSID = "MARBLE" #sparky"
    scan_command = create_scan_command(routerConstant.NETWORK_ESSID, SEARCH_FOR_ESSID)

    # create ssh object
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    try:
        ssh.connect(routerConstant.SSH_ADDRESS, username=routerConstant.SSH_USERNAME, password=routerConstant.SSH_PASSWORD)        
    except Exception as e:
        sys.stderr.write("SSH connection error: {0}".format(e))
        sys.exit(2)

    output = run_command(ssh, scan_command)
   
    if output:
        output_list = output.split()        # split list into strings
        # first item in list is signal, second is quality
        if len(output_list) == 2:
            signal_val = output_list[0]
            quality_str = output_list[1]
           
            # quality value string will be integer/integer
            index = quality_str.find("/")   # find location of /
            q1 = quality_str[:index]        # extract integer left of /
            q2 = quality_str[index+1:]      # extract integer right of /
            print ("signal value: %s dBm, quality: %s/%s" %(signal_val, q1, q2) )
        else:
            print("error, output list length: %d"%len(output_list))
            print("output: %s"%output)
    else:
        print("%s not detected"%SEARCH_FOR_ESSID)
    
    

    ssh.close()




if __name__ == '__main__':
    import sys
    sys.exit(main())