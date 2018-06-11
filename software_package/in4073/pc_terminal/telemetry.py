#Telemetry.py - Saumil Sachdeva
import csv
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style

style.use('seaborn-dark')

fig = plt.figure()
batVoltvsTime = fig.add_subplot(2,3,1)
modevsTime = fig.add_subplot(2,3,4)
motor1vsTime = fig.add_subplot(2,3,2)
motor2vsTime = fig.add_subplot(2,3,3)
motor3vsTime = fig.add_subplot(2,3,5)
motor4vsTime = fig.add_subplot(2,3,6)

def animate(i):
	with open('telemetry.txt') as telemetryFile:
		readTelemetry = csv.reader(telemetryFile)
		time = []
		batVolt = []
		mode = []
		motor1 = []
		motor2 = []
		motor3 = []
		motor4 = []
		for row in readTelemetry:
			if len(row) >1:
				time.append(row[0])
				batVolt.append(row[6])
				mode.append(row[1])
				motor1.append(row[2])
				motor2.append(row[3])
				motor3.append(row[4])
				motor4.append(row[5])
	batVoltvsTime.clear()
	# batVoltvsTime.set_title('Battery Voltage Value')
	# batVoltvsTime.set_xlabel('Time')
	batVoltvsTime.set_ylabel('Battery Voltage')
	batVoltvsTime.plot(time,batVolt)

	modevsTime.clear()
	# modevsTime.set_title('Mode over Time')
	# modevsTime.set_xlabel('Time')
	modevsTime.set_ylabel('Mode')
	modevsTime.plot(time,mode)
	
	motor1vsTime.clear()
	# motor1vsTime.set_title('Motor 1 Value')
	# motor1vsTime.set_xlabel('Time')
	motor1vsTime.set_ylabel('Motor 1')
	motor1vsTime.plot(time,motor1)

	motor2vsTime.clear()
	# motor2vsTime.set_title('Motor 2 Value')
	# motor2vsTime.set_xlabel('Time')
	motor2vsTime.set_ylabel('Motor 2')
	motor2vsTime.plot(time,motor2)

	motor3vsTime.clear()
	# motor3vsTime.set_title('Motor 3 Value')
	# motor3vsTime.set_xlabel('Time')
	motor3vsTime.set_ylabel('Motor 3')
	motor3vsTime.plot(time,motor3)

	motor4vsTime.clear()
	# motor4vsTime.set_title('Motor 4 Value')
	motor4vsTime.set_xlabel('Time')
	motor4vsTime.set_ylabel('Motor 4')
	motor4vsTime.plot(time,motor4)

ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.show()