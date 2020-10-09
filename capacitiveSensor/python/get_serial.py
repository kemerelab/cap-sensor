import serial
import sys
import argparse
import math
import time

BAUD_RATES = [300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200]

parser = argparse.ArgumentParser(description='Get serial data from Arduino.')
parser.add_argument('output_filepath',
                    help='output filepath')
parser.add_argument('-p', '--port', type=str, default='/dev/ttyACM0',
                    help='port on which Arduino is mounted')
parser.add_argument('-b', '--baud-rate', type=int, choices=BAUD_RATES, default=9600,
                    help='baud rate to read serial data')
parser.add_argument('-t', '--timeout', type=float, default=0,
                    help='serial port timeout (s)')
parser.add_argument('-d', '--duration', type=float, default=None,
                    help='duration of data acquisition (s)')
args = parser.parse_args()

with open(args.output_filepath, 'wb') as f:
    ser = serial.Serial(port=args.port,
                        baudrate=args.baud_rate,
                        parity=serial.PARITY_NONE,
                        stopbits=serial.STOPBITS_ONE,
                        bytesize=serial.EIGHTBITS,
                        timeout=1)
    input('Press any key to start...')
    print('Piping from %s to %s' % (args.port, args.output_filepath))
    
    if args.duration is not None:
        t_total = args.duration
    else:
        t_total = math.inf
        print('Press Ctrl-C to stop.')

    t_start = time.time()
    while ser.is_open and ((time.time() - t_start) < t_total):
        if ser.in_waiting > 0:
            f.write(ser.readline())
    ser.close()

print('Logged %.2f seconds of data.' % t_total)



