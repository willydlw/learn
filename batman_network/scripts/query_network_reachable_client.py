#!/usr/bin/env python

import sys
import rospy
from batman_network.srv import CommNodeReachable

def query_network_reachable_client(ip_addr):
    rospy.wait_for_service('network_reachable')
    try:
        query_reachable = rospy.ServiceProxy("network_reachable",CommNodeReachable)
        resp1 = query_reachable(ip_addr)
        return resp1
    except rospy.ServiceException, e:
        rospy.logerr("query network reachable service failed: %s", e)


def usage():
    return "usage: %s [ip_addr]"%sys.argv[0]


if __name__ == "__main__":
    rospy.init_node('network_reachable_client', log_level=rospy.INFO)
    if len(sys.argv) == 2:
        ip_addr = sys.argv[1]
    else:
        print usage()
        sys.exit(1)
    rospy.loginfo("%s reachable?", ip_addr)
    resp1 = query_network_reachable_client(ip_addr)
    rospy.loginfo("response, packet loss: %f, avg time: %f", resp1.packet_loss, resp1.avg_time)
