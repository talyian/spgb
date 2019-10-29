var emulator = 0, frames = 0;
var instance, line=[];
var memory = new WebAssembly.Memory({ initial: 32 });

var error = 0;
var env = {
  spgb_logf: function(f) { line.push(f); },
  spgb_logx8: function(f) { line.push(('00' + f.toString(16)).substr(-2)); },
  spgb_logx16: function(f) { line.push(('0000' + f.toString(16)).substr(-4)); },
  spgb_logx32: function(f) { line.push(f.toString(16)); },
  spgb_logs: function(i, len) {
    if (!len) { line.push(i.toString(16)); return; }
    let bytes = new Uint8Array(memory.buffer.slice(i, i + len));
    line.push(String.fromCharCode.apply(null, bytes));
  },
  spgb_showlog: function() { console.log.apply(console, line); line = []; },
  spgb_push_frame: push_frame,
  spgb_stop: function() { },
  spgb_check_memory_size: function () { 
    console.log(
      'byte size', memory.buffer.byteLength,
      'page size', memory.buffer.byteLength / 64 / 1024);
  },
  spgb_serial_putc: function(c) { console.log("Serial", String.fromCharCode(c)); },
  spgb_get_timestamp: function() { return +new Date(); },
  memory:memory,
};

fetch("build/gb_emulator.wasm")
  .then((resp) => resp.arrayBuffer())
  .then((wasm) => WebAssembly.instantiate(wasm, { env: env }))
  .then((module) => {
    instance = module.instance;

    emulator = instance.exports.spgb_create_emulator();
    let DRAW = 1;
    let DEBUG = 2;
    let ERROR = 3;
    function loop() {
      if (!chkPaused.checked && current_cart)
        instance.exports.spgb_step_frame(emulator);
      requestAnimationFrame(loop);
    }
    start_audio();
    requestAnimationFrame(loop);
    // chkPaused.addEventListener("change", loop);

    var RIGHT = 0, LEFT = 1, UP = 2, DOWN = 3,
        A = 4, B = 5, SELECT = 6, START = 7,
        key_map = {
          65: LEFT,
          83: DOWN,
          68: RIGHT,
          87: UP,
          74: B,
          75: A,
          16: SELECT,
          13: START,
        };
    document.addEventListener("keydown", e => {
      if (e.key == " ") {
        instance.exports.step_instruction(emulator);
        e.preventDefault();
        return false;
      }
      else if (e.key == "Pause") {
        chkPaused.click();
        return false;
      }
      else if (e.keyCode in key_map) {
        instance.exports.spgb_button_down(emulator, key_map[e.keyCode]);
        e.preventDefault();
        return false;
      }
      else
        ; // console.log("keydown", e.key, e.keyCode);
      return true;
    });
    document.addEventListener("keyup", e => {
      if (e.keyCode in key_map)
        instance.exports.spgb_button_up(emulator, key_map[e.keyCode]);
    });

  });
