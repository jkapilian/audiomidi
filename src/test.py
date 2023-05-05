import serial
import matplotlib.pyplot as plt

ser = serial.Serial('/dev/cu.usbserial-0001', 115200)

arr = []

while True:
    message = ser.readline().decode().strip('\n').split(',')
    try:
        arr.append(int(message[0]))
    except:
        continue
    if len(arr) == 1024:
        break


plt.plot(arr[10:])
plt.show()