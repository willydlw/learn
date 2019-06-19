#!/usr/bin/env python

import sys
import rospy
from batman_network.srv import *

def query_network_reachable_client(ip_addr, timeout = "2", count = "1"):
    rospy.wait_for_service('network_reachable')
    try:
        query_reachable = rospy.ServiceProxy("network_reachable",CommNodeReachable)
        resp1 = query_reachable(ip_addr,timeout, count)
        print "%s, response: %s"%(query_network_reachable_client.__name__,resp1.reachable)
        return resp1
    except rospy.ServiceException, e:
        print "query network reachable service failed: %s"%e

def usage():
    return "usage: %s [ip_addr]"%sys.argv[0]


if __name__ == "__main__":
    if len(sys.argv) == 2:
        ip_addr = sys.argv[1]
    else:
        print usage()
        sys.exit(1)
    print "Requesting %s"%(ip_addr)
    print "main, response: %s"%(query_network_reachable_client(ip_addr))