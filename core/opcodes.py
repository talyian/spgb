import csv, itertools

def op(o): return o
def param(p):
    if p == '(a16)': return 'Load(read16())';
    if p == 'a16': return 'read16()'; # address
    if p == 'd16': return 'read16()';
    if p == 'd8': return 'read8()';
    if p == 'r8': return '(int8_t)read8()';
    if p.startswith('(') and p.endswith(')'): return 'Load(%s)' % p[1:-1]
    if p == 'RLCA': return 'RLC(A)';
    if p == 'RLA': return 'RL(A)';
    return p

class Instr:
  def __init__(self, o, params):
      self.op = op(o)
      if o in ['JR', 'JP', 'CALL']:
          if len(params) == 1: params.insert(0, 'Cond::ALWAYS')
          else: params[0] = 'Cond::' + params[0]
      if o in ['RET']:
          if len(params) == 0: params.insert(0, 'Cond::ALWAYS')
          else: params[0] = 'Cond::' + params[0]
      self.params = [param(p) for p in params]
      self.todo = False
      if 'PREFIX' in str(self): self.todo = True
      if 'RST' in str(self): self.todo = True
      if 'HL-' in str(self): self.todo = True
      if 'HL+' in str(self): self.todo = True
      if 'SP+r8' in str(self): self.todo = True
  def __str__(self):
      return '%s(%s)' % (self.op, ','.join(self.params))

outfile = open('OPCODES_TABLE', 'w')
for row in itertools.islice(csv.reader(open('opcodes.csv'), delimiter='\t'), 512):
    if len(row) > 1:
        # print('%02x   %-15s %2s %2s %s' % (int(row[0]), row[1], row[2], row[3], row[4]))
        print('    case 0x%02x: ' % int(row[0]), end='', file=outfile)
        operation = row[1]
        try:
            opcode, params = row[1].split(' ', 2)
            params = params.split(',')
        except:
            opcode = row[1]
            params = []
        ii = Instr(opcode, params)            
        func = 'OP'
        if ii.todo: func = 'break; //'
        print('%s(%-20s, %2s, %2s, "%s");' % (
            func, 
            ii, row[2], row[3], row[4]), file=outfile)
    else:
        print('    case 0x%02x: break;' % int(row[0]), file=outfile);
