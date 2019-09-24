#/usr/bin/env python3
import csv, os, sys, json, re
rows = csv.reader(open(os.path.dirname(os.path.abspath(__file__)) + '/opcode_table.csv'))

def parse_code(code):

    operand = "\w+|\w+\(\w+\)"
    operand1 = "^(\w+)$|^(\w+)\((\w+)\)$"
    def op_func(n):
        m = re.match(operand1, n)
        single = m.group(1)
        callee = m.group(2)
        arg = m.group(3)
        if single:
            if single.isdigit() or single.startswith('0x'):
                return single
            return single + '()'
        elif not arg:
            return f'{callee}()'
        else:
            return f'{callee}({op_func(arg)})'

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
    opcode, size, cycles, flags, action = row[:5]
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
