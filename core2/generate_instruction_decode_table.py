#/usr/bin/env python3
import csv, os, sys, json, re
rows = csv.reader(open(os.path.dirname(os.path.abspath(__file__)) + '/opcode_table.csv'))

def parse_code(code):

    operand = "\w+|IO\(\w+\)|(?:LoadSP|Load16|Load|Inc|Dec)\(\w+\)"
    def op_func(n):
        if n.isdigit() or n.startswith('0x'): 
            return '{OperandType::IMM8, {(u8)' + n +'}}'
        
        if n.startswith('IO('):
            return 'IO(' + op_func(n[3:-1]) + ')'
        if n.startswith('Load16('):
            return 'Load16(' + op_func(n[7:-1])  + ')'
        if n.startswith('LoadSP('):
            return 'LoadSP(' + op_func(n[7:-1])  + ')'
        if n.startswith('Load('):
            return 'Load(' + op_func(n[5:-1])  + ')'
        if n.startswith('Inc('):
            return 'Inc(' + op_func(n[4:-1])  + ')'
        if n.startswith('Dec('):
            return 'Dec(' + op_func(n[4:-1])  + ')'
        return n + '()'

    m = re.match(rf'(\w+)\(\)', code)
    if m:
        return f'ii.{m.group(1)}();'
    
    # unary op
    m = re.match(rf"(\w+)\(({operand})\)", code)
    if m:
        instr = m.group(1)
        a = m.group(2)
        return f'ii.{instr}({op_func(a)});'
    
    # binary op
    m = re.match(rf"(\w+)\(({operand}),({operand})\)", code)
    if m:
        instr = m.group(1)
        a = m.group(2)
        b = m.group(3)
        return f'ii.{instr}({op_func(a)}, {op_func(b)});'

for row in rows:
    opcode, size, cycles, flags, action = row
    row_str =  ' '.join(row)
    if (opcode == 'opcode'): continue;
    if (size == ''): continue;

    str_action = json.dumps(action)
    print(f'case 0x{opcode}:  // {row_str}')
    if parse_code(action):
        print(f'  {parse_code(action)}', end='')
    else:
        print('  _log("    ");');
        print(f'  log("fallback", {str_action}); // fallback')
        print("  error = 1;");
    # print(f'  if (pc != pc_start + {size}) log("pc error", (i32)pc, (i32)_pc + {size});')
    print(f' pc = pc_start + {size}; break;')
