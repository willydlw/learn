class RobotLogFile:
	def __init__(self):
		self.motor_ticks = []
		

	def read(self, filename):
		"""
		Reads log data from file. Currently, only call this
		once to read and store motor ticks. If called a second time
		the previously read data will be cleared."""
		f = open(filename)

		# create empty list to store motor ticks
		left_ticks = []
		right_ticks = []

		# read each line of the file
		for line in f:
			sp = line.split()
			if sp[0] == 'M':
				# left motor ticks in col 2, right motor in 6
				left_ticks.append((int(sp[2])))
				right_ticks.append((int(sp[6])))
			else:
				print("error: file contains non-motor data")

		# now that all motor ticks have been read, find the difference
		print("len(left_ticks) "),
		print(len(left_ticks))

		for i in range(1, len(left_ticks)):
			left_diff = left_ticks[i] - left_ticks[i-1]
			right_diff = right_ticks[i] - right_ticks[i-1]

			self.motor_ticks.append(
				tuple([left_diff, right_diff]))
