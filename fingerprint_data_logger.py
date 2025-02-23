import serial
import time
import openpyxl

# Configure serial connection
ser = serial.Serial('COM4', 9600)  # Replace 'COM3' with your Arduino's COM port
time.sleep(2)  # Wait for the serial connection to initialize

# Prepare Excel workbook
workbook = openpyxl.Workbook()
sheet = workbook.active
sheet.title = "Fingerprint Data"

# Add headers
sheet.append(['ID', 'Name', 'Address', 'City', 'Fingerprint Data'])

def read_fingerprint_data():
    data = []
    while len(data) < 534:
        if ser.in_waiting:
            data.append(int.from_bytes(ser.read(), 'big'))
    return data

def save_to_excel(fingerprint_id, name, address, city, fingerprint_data):
    row = [fingerprint_id, name, address, city] + fingerprint_data
    sheet.append(row)
    workbook.save('fingerprint_data.xlsx')

while True:
    if ser.in_waiting:
        line = ser.readline().decode('utf-8').strip()
        if line.startswith('ID:'):
            fingerprint_id = int(line.split(':')[1])
        elif line.startswith('Name:'):
            name = line.split(':')[1]
        elif line.startswith('Address:'):
            address = line.split(':')[1]
        elif line.startswith('City:'):
            city = line.split(':')[1]
        elif line.startswith('Fingerprint Data:'):
            fingerprint_data = read_fingerprint_data()
            save_to_excel(fingerprint_id, name, address, city, fingerprint_data)
            print(f"Data for ID {fingerprint_id} saved.")
