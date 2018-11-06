import csv, itertools

def op(o):
    if o == 'LDH': return 'LD'
    return o
def param(p):
    if p == '(a16)': return 'Load(read16())';
    if p == '(C)': return 'IO(C)';
    if p == '(a8)': return 'IO(read8())';
    if p == 'a16': return 'read16()'; # address
    if p == 'd16': return 'read16()';
    if p == 'd8': return 'read8()';
    if p == 'r8': return '(int8_t)read8()';
    if p.startswith('(') and p.endswith(')'): return 'Load(%s)' % p[1:-1]
    if p == 'RLCA': return 'RLC(A)';
    if p == 'RLA': return 'RL(A)';
    return p

class Instr:
  def __init__(self, o, params, id):
      self.code = id
      if o in ['RLCA', 'RRCA', 'RLA', 'RRA']:
          o = o[:-1]
          params = ['A']
      if o == 'SBC' and params[0] == 'A':
          params = params[1:]
      if o in ['JR', 'JP', 'CALL']:
          if len(params) == 1: params.insert(0, 'Cond::ALWAYS')
          else: params[0] = 'Cond::' + params[0]
      if o in ['RET']:
          if len(params) == 0: params.insert(0, 'Cond::ALWAYS')
          else: params[0] = 'Cond::' + params[0]
      if o in ['RST']:
          params[0] = '0x' + params[0][:-1]
      if o in ['RETI']:
          o = 'RET(Cond::ALWAYS);ii.EI'
      self.op = op(o)
      self.params = [param(p) for p in params]
      self.todo = False
      
      if 'PREFIX' in str(self): self.todo = True
      if 'HL-' in str(self): self.todo = True
      if 'HL+' in str(self): self.todo = True
      if 'JP(Cond::ALWAYS,Load(HL))' in str(self): self.todo = True
      if 'SP+r8' in str(self): self.todo = True
      if self.code == 0x08: self.todo = True
      if self.code == 0x10: self.todo = True
      if self.code == 0x27: self.todo = True
      if self.code == 0x37: self.todo = True
      if self.code == 0x3F: self.todo = True
      if self.code == 0xcb: self.todo = True # extended op
      
      
  def __str__(self):
      return '%s(%s)' % (self.op, ','.join(self.params))

outfile = open('OPCODES_TABLE', 'w')
for row in itertools.islice(csv.reader(open('opcodes.csv'), delimiter='\t'), 512):
    if len(row) > 1:
        # print('%02x   %-15s %2s %2s %s' % (int(row[0]), row[1], row[2], row[3], row[4]))
        operation = row[1]
        try:
            opcode, params = row[1].split(' ', 2)
            params = params.split(',')
        except:
            opcode = row[1]
            params = []
        ii = Instr(opcode, params, int(row[0]))            

        if ii.todo: print('    // ', end='', file=outfile)
        else: print('    ', end='', file=outfile)
        print('case 0x%02x: ' % int(row[0]), end='', file=outfile)
        print('OP(%-20s, %2s, %2s, "%s");' % (
            ii, row[2], row[3], row[4]), file=outfile)
    else:
        pass
