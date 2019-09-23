#/usr/bin/env python3
import csv, os
rows = csv.reader(open(os.path.dirname(os.path.abspath(__file__)) + '/opcode_table.csv'))
for row in rows:
    print('//', row)
