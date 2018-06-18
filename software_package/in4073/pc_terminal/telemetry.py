import pandas as pd
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
	headers = ["time","bat_volt","mode","motor1","motor2","motor3","motor4"]
	telemetryFile = pd.read_csv('telemetry.csv',names=headers)
	telemetryFile.set_index("time",inplace=True)
	mode=telemetryFile[["mode"]]
	motor1=telemetryFile[["motor1"]]
	motor2=telemetryFile[["motor2"]]
	motor3=telemetryFile[["motor3"]]
	motor4=telemetryFile[["motor4"]]
	batVolt=telemetryFile["bat_volt"]

	
	modevsTime.clear()
	modevsTime.set_title('Mode')
	# modevsTime.set_xlabel('Time')
	modevsTime.plot(mode)
	motor1vsTime.clear()
	motor1vsTime.set_title('Motor 1')
	# motor1vsTime.set_xlabel('Time')
	motor1vsTime.plot(motor1)
	motor2vsTime.clear()
	motor2vsTime.set_title('Motor 2')
	# motor2vsTime.set_xlabel('Time')
	motor2vsTime.plot(motor2)
	motor3vsTime.clear()
	motor3vsTime.set_title('Motor 3')
	# motor3vsTime.set_xlabel('Time')
	motor3vsTime.plot(motor3)
	motor4vsTime.clear()
	motor4vsTime.set_title('Motor 4')
	# motor4vsTime.set_xlabel('Time')
	motor4vsTime.plot(motor4)
	batVoltvsTime.clear()
	batVoltvsTime.set_title('Battery Voltage')
	# batVoltvsTime.set_xlabel('Time')
	batVoltvsTime.plot(batVolt)

ani = animation.FuncAnimation(fig, animate, interval=1)
plt.show()