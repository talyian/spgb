import csv, itertools, sys


for row in itertools.islice(csv.reader(sys.stdin, delimiter='\t'), 512):
    if row[4].startswith('ERR'): continue
    if 'PREFIX' in row[4]: continue
    OPFUNC = 'OP_RAW' if row[4].startswith("{") else 'OP'
    print(f'    case 0x{row[0]}: {OPFUNC}({row[4]},{row[1]},{row[2]},\'{row[3]}\');')
