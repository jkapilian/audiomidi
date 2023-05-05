import serial
import mido

ser = serial.Serial('/dev/cu.usbserial-0001', 115200)
outport = mido.open_output('IAC Driver Bus 1')

cur = 0

while True:
	try:
		message = int(ser.readline().decode().strip('\n'))
		if message == 0:
			outport.send(mido.Message('note_off', note=cur))
		else:
			outport.send(mido.Message('note_off', note=cur))
			cur = message
			outport.send(mido.Message('note_on', note=cur))
	except:
		continue