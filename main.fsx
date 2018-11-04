fsi.ShowProperties <- false

type Registers8080 () = class
  member val F = 0uy with get, set
  member val A = 0uy with get, set
  member val B = 0uy with get, set
  member val C = 0uy with get, set
  member val D = 0uy with get, set
  member val E = 0uy with get, set
  member val L = 0uy with get, set
  member val H = 0uy with get, set
  member val SP = 0us with get, set
  member val PC = 0us with get, set
  member this.BC
    with get () = (uint16)this.B * 256us + (uint16) this.C
    and set (v:uint16) = this.C <- uint8 v; this.B <- uint8 (v >>> 8)
  member this.DE
    with get () = (uint16)this.D * 256us + (uint16) this.E
    and set (v:uint16) = this.E <- uint8 v; this.D <- uint8 (v >>> 8)
  member this.HL
    with get () = ((uint16)this.H <<< 8) + (uint16) this.L
    and set (v:uint16) = this.L <- uint8 v; this.H <- uint8 (v >>> 8)
  member this.Dump () =
    printfn " A: %02x" this.A
    printfn " B: %02x" this.B
    printfn " C: %02x" this.C
    printfn " D: %02x" this.D
    printfn " E: %02x" this.E
    printfn " F: %02x" this.F
    printfn " H: %02x" this.H
    printfn " L: %02x" this.L
    printfn "HL: %04x" this.HL
    printfn "SP: %04x" this.SP
    printfn "PC: %04x" this.PC
end

/// targets that are 8 bit values
type RegisterOP =
  | A | B | C | D | E | F | H | L
  | AF | BC | DE | HL
  | SP | PC
  | U8 of uint8
  | U16 of uint16
  | Load of RegisterOP
  | LoadF of RegisterOP
with
  override this.ToString () =
    match this with
      | U8 x -> sprintf "%02x" x
      | U16 x -> sprintf "%04x" x
      | Load r -> sprintf "(%O)" r
      | LoadF r -> sprintf "(FF00 + %O)" r
      | x -> sprintf "%A" x

type JumpCond = | JZ | JNZ | JC | JNC | JAL
with
  override this.ToString () = sprintf "%A" this

type Operator = interface
  abstract NOP : unit -> unit
  abstract INC : RegisterOP -> unit
  abstract DEC : RegisterOP -> unit
  abstract LD: RegisterOP * RegisterOP -> unit
  abstract BIT : uint8 * RegisterOP -> unit
  abstract JR : JumpCond * uint16 -> unit
  abstract CP : RegisterOP -> unit
  abstract XOR : RegisterOP -> unit
  abstract AND : RegisterOP -> unit
  abstract OR : RegisterOP -> unit
  abstract SUB_A : RegisterOP -> unit
  abstract ADD_A : RegisterOP -> unit
  // abstract ADC : RegisterOP -> unit
  // abstract  : RegisterOP -> unit
end

type MultiOperator (inner: Operator list) =
  interface Operator with 
    member __.NOP () = for x in inner do x.NOP()
    member __.INC target = for x in inner do x.INC target
    member __.DEC target = for x in inner do x.DEC target
    member __.LD (target, value) = for x in inner do x.LD(target, value)
    member __.BIT(a, b) = for x in inner do x.BIT(a, b)
    member __.JR(cond, v) = for x in inner do x.JR(cond, v)
    member __.CP(target) = for x in inner do x.CP target
    member __.XOR target = for x in inner do x.XOR target
    member __.AND target = for x in inner do x.AND target
    member __.OR target = for x in inner do x.OR target
    member __.SUB_A target = for x in inner do x.SUB_A target
    member __.ADD_A target = for x in inner do x.ADD_A target
    
type Interpreter(registers:Registers8080, memory:byte[]) =
  let get_value_16 target =
    match target with
    | BC -> registers.BC
    | DE -> registers.DE
    | HL -> registers.HL
    | SP -> registers.SP
    | PC -> registers.PC
    | U16 v -> v
    | other -> failwithf "unable to load: %O" other

  let rec get_value_8 target =
    match target with
    | A -> registers.A
    | B -> registers.B
    | C -> registers.C
    | D -> registers.D
    | E -> registers.E
    | F -> registers.F
    | L -> registers.L
    | H -> registers.H
    | U8 v -> v
    | Load addr -> memory.[int(get_value_16 addr)]
    | LoadF offset -> memory.[int(get_value_8 offset) + 0xFF00]
    | other -> failwithf "unable to load: %O" other

  let set_value_8 target f =
    match target with
    | A -> registers.A <- f (registers.A)
    | B -> registers.B <- f (registers.B)
    | C -> registers.C <- f (registers.C)
    | D -> registers.D <- f (registers.D)
    | E -> registers.E <- f (registers.E)
    | F -> registers.F <- f (registers.F)
    | H -> registers.H <- f (registers.H)
    | L -> registers.L <- f (registers.L)
    | Load x ->
      let addr = int (get_value_16 x)
      match f (memory.[addr]) with
        | 0uy -> ()
        | value -> printfn "[%04x] :: %O" addr value
      memory.[int registers.HL] <- f (memory.[int registers.HL])
    | LoadF x ->
      // printfn "[FF00 + %O] :: %O" x (f 0uy)
      let value = int (get_value_8 x)
      memory.[int registers.C + 0xFF00] <- f 0uy
    | other -> failwithf "unable to set 8: %O" other

  let set_value_16 target f =
    match target with
    | BC -> registers.BC <- f (registers.BC)
    | DE -> registers.DE <- f (registers.DE)
    | HL -> registers.HL <- f (registers.HL)
    | SP -> registers.SP <- f (registers.SP)
    | PC -> registers.PC <- f (registers.PC)
    | other -> failwithf "unable to set 16: %O" other

    
  interface Operator with
    member __.SUB_A target =
      let v = get_value_8 A - get_value_8 target
      let flag_z = if v = 0uy then 0x80uy else 0uy
      let flag_n = 0x40uy
      let flag_h = 0uy // TODO
      let flag_c = if get_value_8 A < get_value_8 target then 0x10uy else 0uy
      registers.F <- flag_z ||| flag_n ||| flag_h ||| flag_c
      set_value_8 A (fun _ -> v)
    member __.ADD_A target = set_value_8 A (fun v -> v + get_value_8 target)
    member __.XOR target = set_value_8 A (fun v -> v ^^^ get_value_8 target)
    member __.AND target = set_value_8 A (fun v -> v &&& get_value_8 target)
    member __.OR target = set_value_8 A (fun v -> v ||| get_value_8 target)
    member __.CP target =
      let v = get_value_8 A - get_value_8 target
      let flag_z = if v = 0uy then 0x80uy else 0uy
      let flag_n = 0x40uy
      let flag_h = 0uy // TODO
      let flag_c = if get_value_8 A < get_value_8 target then 0x10uy else 0uy
      registers.F <- flag_z ||| flag_n ||| flag_h ||| flag_c
    member __.NOP () = ()
    member __.INC target =
      match target with
        | A | B | C | D | E | F | H | L -> set_value_8 target (fun v -> v + 1uy)
        | AF | BC | DE | HL | SP | PC -> set_value_16 target (fun v -> v + 1us)
        | other -> failwithf "inc failed: %O" other
    member __.DEC target =
      match target with
        | A | B | C | D | E | F | H | L ->
          let v = get_value_8 target - 1uy
          let flag_z = if v = 0uy then 0x80uy else 0uy
          let flag_n = 0x40uy
          let flag_h = 0uy // TODO
          let flag_c = if get_value_8 target < 1uy then 0x10uy else 0uy
          registers.F <- flag_z ||| flag_n ||| flag_h ||| flag_c
          set_value_8 target (fun _ -> v)
        | AF | BC | DE | HL | SP | PC ->
          let v = get_value_16 target - 1us
          let flag_z = if v = 0us then 0x80uy else 0uy
          let flag_n = 0x40uy
          let flag_h = 0uy // TODO
          let flag_c = if get_value_16 target < 1us then 0x10uy else 0uy
          registers.F <- flag_z ||| flag_n ||| flag_h ||| flag_c
          set_value_16 target (fun _ -> v)
        | other -> failwithf "dec failed: %O" other
    member __.LD (target, value) =
      match target with
        | A | B | C | D | E | F | H | L | Load _ | LoadF _ -> set_value_8 target (fun v -> get_value_8 value)
        | AF | BC | DE | HL | SP | PC -> set_value_16 target (fun v -> get_value_16 value)
        | other -> failwithf "ld failed: %O" other
    member __.BIT(bit, target) =
      let value = get_value_8 target
      let out_v = (value >>> int bit) &&& 0x1uy
      let f_zero = 0x80uy * ~~~out_v
      registers.F <- f_zero // TODO: Half-carry bit?
    member __.JR(cond, value) =
      let bcond =
        match cond with
          | JumpCond.JZ -> registers.F &&& 0x80uy = 0x80uy
          | JumpCond.JNZ -> registers.F &&& 0x80uy = 0x0uy
          | JumpCond.JC -> registers.F &&& 0x10uy = 0x10uy
          | JumpCond.JNC -> registers.F &&& 0x10uy = 0x0uy
          | JumpCond.JAL -> true
      if bcond then registers.PC <- value
      
type InstructionPrinter () =
  interface Operator with
    member __.NOP () = printfn "NOP"
    member __.INC target = printfn "INC %O" target
    member __.DEC target = printfn "DEC %O" target
    member __.LD(target, value) = printfn "LD %O %O" target value
    member __.BIT(bit, target) = printfn "BIT %O %O" bit target
    member __.JR(cond, target) = printfn "JP %O %O" cond target
    member __.ADD_A target = printfn "ADD A %O" target
    member __.SUB_A target = printfn "SUB A %O" target
    member __.OR target = printfn "OR A %O" target
    member __.AND target = printfn "AND A %O" target
    member __.XOR target = printfn "XOR A %O" target
    member __.CP target = printfn "CP %O" target
    
type CPU8080 (rom) = class
  let memory = Array.zeroCreate 0x10000
  let mutable jump_table = ResizeArray<_>()
  let mutable registers = new Registers8080()
  let exec =
    let args = [
      // InstructionPrinter() :> Operator;
      Interpreter(registers, memory) :> Operator]
    MultiOperator(args) :> Operator
  do
    Array.blit rom 0 memory 0 rom.Length

  member this.LoadRom(bytes: byte[]) = Array.blit bytes 0xff memory 0xff (bytes.Length - 0xff)
  member this.ReadU8() = 
    let v = memory.[int registers.PC]
    registers.PC <- registers.PC + 1us
    v
  member this.ReadU16() = 
    let v = uint16 memory.[int registers.PC] + (256us * uint16 memory.[int registers.PC + 1])
    registers.PC <- registers.PC + 2us
    v

  member this.Registers with get () = registers
  member this.JumpTo addr = registers.PC <- addr
  member this.Dump() = registers.Dump()
  member this.Step() =
    let op = this.ReadU8()
    let read8 () = U8 <| this.ReadU8()
    let read16 () = U16 <| this.ReadU16 ()
    match op with
      | 0x00uy -> exec.NOP ()
      | 0x04uy -> exec.INC B
      | 0x05uy -> exec.DEC B
      | 0x06uy -> exec.LD(B, read8())
      | 0x0Cuy -> exec.INC C
      | 0x0Euy -> exec.LD(C, read8())
      | 0x0duy -> exec.DEC C
      | 0x11uy -> exec.LD(DE, read16())
      | 0x13uy -> exec.INC(DE)
      | 0x15uy -> exec.DEC(D)
      | 0x16uy -> exec.LD(D, read8())
      | 0x17uy -> printfn "RLA"
      | 0x18uy ->
        let v = this.ReadU8()
        exec.JR(JAL, (registers.PC + uint16 (int8 v)))
      | 0x1Duy -> exec.DEC E
      | 0x1auy -> exec.LD(A, Load DE)
      | 0x1euy -> exec.LD(E, read8())
      | 0x20uy ->
        let v = this.ReadU8()
        exec.JR(JNZ, (registers.PC + uint16 (int8 v)))
        // let v = this.ReadU8() in printfn "JR NZ %x" (registers.PC + uint16 (int8 v))
      | 0x21uy -> exec.LD(HL, read16())
      | 0x22uy -> exec.LD(Load HL, A); exec.INC HL // printfn "LD (HL+), A"
      | 0x23uy -> exec.INC HL
      | 0x24uy -> exec.INC H
      | 0x28uy ->
          let v = this.ReadU8()
          exec.JR(JZ, (registers.PC + uint16 (int8 v)))
          // let v = this.ReadU8() in printfn "JR Z %x" (registers.PC + uint16 (int8 v))
      | 0x2Euy -> exec.LD(L, read8())
      | 0x31uy -> exec.LD(SP, read16())
      | 0x32uy -> exec.LD(Load HL, A); exec.DEC HL; // printfn "LD (HL-) A"
      | 0x3Duy -> exec.DEC(A); //printfn "DEC A"
      | 0x3Euy -> exec.LD(A, read8())
      | 0x4fuy -> exec.LD(C, A)
      | 0x57uy -> exec.LD(D, A)
      | 0x67uy -> exec.LD(H, A)
      | 0x77uy -> exec.LD(Load HL, A) // printfn "LD (HL), A"
      | 0x78uy -> exec.LD(A, B)
      | 0x79uy -> exec.LD(A, C)
      | 0x7Auy -> exec.LD(A, D)
      | 0x7Buy -> exec.LD(A, E)
      | 0x7Cuy -> exec.LD(A, H)
      | 0x7Duy -> exec.LD(A, L)
      | 0x86uy -> exec.ADD_A (Load HL)
      | 0x90uy -> exec.SUB_A B
      | 0xAFuy -> exec.XOR A
      | 0xCBuy ->
        match this.ReadU8() with
          | 0x11uy -> printfn "RL C"
          | 0x7cuy -> exec.BIT(7uy, H)
          | op -> printfn "Extended op: %x" op
      | 0xCDuy -> printfn "CALL %04x" (this.ReadU16())
      | 0xE0uy -> exec.LD(LoadF (read8()), A) // printfn "LD ($FF00 + %x), A" (this.ReadU8())
      | 0xE2uy -> exec.LD(LoadF C, A) // printfn "LD ($FF00 + C), A"
      | 0xEAuy -> exec.LD(Load (read16()), A) // printfn "LD (%x), A" (this.ReadU16())
      | 0xF0uy -> exec.LD(A, LoadF (read8())) // printfn "LD A, ($FF00 + %x)" (this.ReadU8())
      | 0xFEuy -> exec.CP (read8())
      | 0xBEuy -> exec.CP (Load HL)
      | 0xc1uy -> printfn "POP BC"
      | 0xc5uy -> printfn "PUSH BC"
      | 0xc9uy -> printfn "RET"
      | op -> printfn " -- OP: %x" op
end

let readbytes s = System.IO.File.ReadAllBytes s
let mutable z80 = CPU8080(readbytes "DMG_ROM.bin")
z80.LoadRom(readbytes "ttt.gb") // tic tac toe
z80.JumpTo 0us

// run BIOS
while z80.Registers.PC < 0x64us do z80.Step()

z80.Dump()
// while z80.Registers.PC < 0xa8us do
//   z80.Step()
// z80.JumpTo 0xE0us
// while z80.Registers.PC < 0xffus do
//   z80.Step()
