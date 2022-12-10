#!/usr/bin/env python3
"""Read lines from a serial port and write to stdout."""

import argparse
import sys
from typing import List

import serial  # 3rd party, not standard Python


def main(argv: List[str]):
  args = argv[1:]

  parser = argparse.ArgumentParser()
  parser.add_argument(
      '--port', metavar='PORT', type=str, help='Update generated sections.'
  )

  parsed_args = parser.parse_args(args)
  print('port:', parsed_args.port)

  ser = serial.Serial(parsed_args.port, baudrate=115200, timeout=120)

  while True:
    try:
      line = ser.readline()
      line = line.decode('utf-8')
      print(line, end='', flush=True)
    except KeyboardInterrupt:
      print('Keyboard Interrupt')
      break


if __name__ == '__main__':
  main(sys.argv[:])
