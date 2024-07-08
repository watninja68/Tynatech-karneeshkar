import sqlite3
import serial

conn = sqlite3.connect('test.db')
c = conn.cursor()

c.execute("""CREATE TABLE IF NOT EXISTS serial_data (
            key_value TEXT PRIMARY KEY,
             val integer,
             rssi integer,
             vals integer,
            unique_count integer DEFAULT 0)""")

ser = serial.Serial('COM12', 115200, timeout=1)

try:
    while True:
        line = ser.readline().decode('iso-8859-1').strip().split(',')

        # print(line)
            
        if(len(line) == 6):
            key_value = line[0]
            val = int(line[1])
            rssi = int(line[3])
            vals = line[5]

            c.execute("SELECT 1 FROM serial_data WHERE key_value=?", (key_value,))
            if c.fetchone():
                c.execute("UPDATE serial_data SET unique_count=unique_count+1 WHERE key_value=?", (key_value,))
            else:
                c.execute("INSERT INTO serial_data VALUES (?,?,?,?, 1)", (key_value, val, rssi, vals))

except KeyboardInterrupt:
    pass

finally:
    ser.close()
    conn.commit()
    conn.close()