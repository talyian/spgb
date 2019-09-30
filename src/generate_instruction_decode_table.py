#/usr/bin/env python3
import csv, os, sys, json, re
rows = csv.reader(open(os.path.dirname(os.path.abspath(__file__)) + '/opcode_table.csv'))

class Operand:
    def __init__(self, source, callee, arg):
        self.source = source
        self.callee = callee
        self.arg = arg
    def __str__(self):
        if re.match('((0x[0-9a-fA-F])|[0-9])[0-9a-fA-F]*', self.arg):
            val = self.arg
        else:
            val = str(self.arg) + "()"
        
        if self.callee:
            return f'{self.callee}({val})'
        else:
            return val
        
class Action:
    def __init__(self, callee, args):
        self.callee = callee
        self.args = args
    def __str__(self):
        return f'  ii.{self.callee}({", ".join(map(str, self.args))})'
    def get(self, index):
        if index >= len(self.args): return 0
        return self.args[index];

class FallbackAction:
    def __init__(self, msg):
        self.msg = msg
        self.callee = msg
        self.args = []
    def __str__(self):
        return f'  log("fallback", {json.dumps(self.msg)}); error = 1'
    def get(self, index): return ""
    
def parse_operand(s):
    operand1 = "^^(\w+)(?:\((\w+)\))?$"
    m = re.match(operand1, s)
    callee = m.group(1)
    arg = m.group(2)
    if not arg:
        return Operand(s, '', callee)
    else:
        return Operand(s, callee, arg)
    
def parse_code(code):
    operand = "\w+|\w+\(\w+\)"

    m = re.match(rf'(\w+)\(\)', code)
    if m: return Action(m.group(1), []);

    m = re.match(rf"(\w+)\(({operand})\)", code)
    if m: return Action(m.group(1), [parse_operand(m.group(2))])

    m = re.match(rf"(\w+)\(({operand}),({operand})\)", code)
    if m: return Action(m.group(1), [
            parse_operand(m.group(2)),
            parse_operand(m.group(3))])

    return FallbackAction("PREFIX__");

for row in rows:
    opcode, size, cycles, flags, action = row[:5]
    if (opcode == 'opcode'): continue;
    if (size == ''): continue;

    opcode = int(opcode, 16)
    size = int(size)
    if '/' in cycles:
        cycles, cycles2 = cycles.split('/')
    else:
        cycles2 = '-1'
        
    row_str =  ' '.join(row)

    str_action = json.dumps(action)
    call_syntax = parse_code(action)
    # print(f'case 0x{opcode}:  // {row_str}')
    # print(call_syntax, end=';')
    # print(f' break;')
    if len(call_syntax.args) == 0:
        macro = "ENTRY0"
    elif len(call_syntax.args) == 1:
        macro = "ENTRY1"
    elif len(call_syntax.args) == 2:
        macro = "ENTRY2"
    print(f'  {macro}(0x{opcode:03x}, {size:02}, {cycles:>2}, {cycles2:>2}, "{flags}",'
          f' {call_syntax.callee:7}, '
          f'{str(call_syntax.get(0))}, '
          f'{str(call_syntax.get(1))}'
          ')')
