# Plot the ticks from the left and right motor
#01_a_plot_motor_ticks.py
# Claus Brenner's SLAM course

from pylab import *


if __name__ == '__main__':
	# read all ticks from left and right motor
	# File format
	# M timestamp(ms)  pos-left(ticks)  ...   pos-right(ticks) ...
	# field 2 is left motor, field 6 is right
	f = open("robot4_motors.txt")

	# create empty lists
	left_list = []
	right_list = []

	for line in f:
		sp = line.split()
		# convert string to integer
		left_list.append(int(sp[2]))
		right_list.append(int(sp[6]))

	plot(left_list)
	plot(right_list)
	show()
