
import paramiko
import sys 

def init(ip_addr, user_name, pass_word):
    global ssh

    # create ssh object
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    try:
        ssh.connect(ip_addr, username=user_name, password=pass_word)        
    except Exception as e:
        sys.stderr.write("SSH connection error: {0}".format(e))