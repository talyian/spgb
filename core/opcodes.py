import csv, itertools

def op_NOP(): pass
def op_INC(): pass
def op_DEC(): pass
def op_LD(dst, src): pass
def op_CALL(cond, addr): pass
def op_ADD(): pass
def op_RLCA(): pass

def op(o): return o
def param(p):
    if p == '(a16)': return 'Load(read16())';
    if p == 'a16': return 'read16()'; # address
    if p == 'd16': return 'read16()';
    if p == 'd8': return 'read8()';
    if p == 'r8': return '(int8_t)read8()';
    return p
# def op_RLA(): pass
# def op_RRCA(): pass
# def op_RRA(): pass

class Instr:
  def __init__(self, o, params):
      self.op = op(o)
      self.params = [param(p) for p in params]
  def __str__(self):
      return '%4s(%s)' % (self.op, ','.join(self.params))

for row in itertools.islice(csv.reader(open('opcodes.csv'), delimiter='\t'), 512):
    if len(row) > 1:
        # print('%02x   %-15s %2s %2s %s' % (int(row[0]), row[1], row[2], row[3], row[4]))
        print('case 0x%02x: ' % int(row[0]), end='')
        operation = row[1]
        try:
            opcode, params = row[1].split(' ', 2)
            params = params.split(',')
        except:
            opcode = row[1]
            params = []
        print('OP(%20s, %2s, %2s, "%s")' % (Instr(opcode, params), row[2], row[3], row[4]))
    else:
        print('case 0x%02x: break;' % int(row[0]));
