import serial
ser = serial.Serial('COM12', 115200, timeout=1)
try:
    while True:
        line = ser.readline().decode('iso-8859-1').strip().split(',')

        print(line)
except KeyboardInterrupt:
    pass

finally:
    ser.close()
