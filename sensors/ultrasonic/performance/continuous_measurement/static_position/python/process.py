
import csv

# open file for writing
ifile = open('data.csv', 'r')
reader = csv.reader(ifile)

# convert reader to list, so I can extract totalRows
rlist = list(reader)
totalRows = len(rlist)

print("totalRows: " + str(totalRows))


rowNum = 0
dataCount = 0
dataTotal = 0

for row in rlist:
	print("rowNum: " + str(rowNum))
	print
	colnum = 0
	for col in row:
		print("colnum: " + str(colnum) + ", col data: " + str(col))

		if colnum == 0:
			angle = col
			print("angle: " + angle)
		else:
			dataTotal += float(col)
			dataCount += 1
		colnum += 1
	rowNum += 1

print("dataCount: " + str(dataCount))
average = dataTotal / dataCount

print("average: " + str(average))
ifile.close()

		
		