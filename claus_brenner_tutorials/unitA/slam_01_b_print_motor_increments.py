from pylab import *
from robot_log_file import RobotLogFile 

if __name__ == '__main__':
	logfile = RobotLogFile()
	logfile.read("robot4_motors.txt")

	for i in range(20):
		print(logfile.motor_ticks[i])

	plot(logfile.motor_ticks)
	show()
