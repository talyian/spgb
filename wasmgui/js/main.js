if (typeof TextDecoder == "undefined") {
  function TextDecoder() {  }
  TextDecoder.prototype.decode = function (arr) {
    return String.fromCharCode.apply(null, new Uint8Array(arr));
  }
}

var emulator = 0, frames = 0;
var instance, line=[], dec = new TextDecoder();
var memory = new WebAssembly.Memory({ initial: 32 });
var tile_maps = [0, 0, 0];
function draw_display(canvas, data) {
  frames++;
  var p = 0, i = 0;
  var image_buffer = canvas._buffer;
  var ctx = canvas.getContext("2d");
  if (!image_buffer)
    image_buffer = canvas._buffer = ctx.createImageData(160, 144);
  var H = 144, W = 160;
  var idata = image_buffer.data, j = 0;
  for(var y = 0; y < H; y++) {
    for(var x = 0; x < W; x++) {
      p = data[i++];
      
      idata[j++] = (p / 36 | 0) * 51;
      idata[j++] = (((p / 6) | 0) % 6) * 51;
      idata[j++] = (p % 6) * 51;
      idata[j++] = 255;
    }
  }
  ctx.putImageData(image_buffer, 0, 0);
}
// setInterval(function() {
//   console.log("frames / sec", frames);
//   frames = 0;
// }, 1000);
function draw_vram(canvas, data) {
  var ctx = canvas.getContext("2d");
  var image_buffer = canvas._buffer;
  if (!image_buffer)
    image_buffer = canvas._buffer = ctx.createImageData(canvas.width, canvas.height);
  
  var idata = image_buffer.data, k = 0;
  var w = 8, tile_idx = 0, tile_number = 0;
  // 0x800 bytes, 0x80 sprites, so each one takes up 0x10 bytes
  for(var y = 0; y < 8; y++) {
    for(var x = 0; x < 16; x++) {
      tile_number++;
      for(var j = 0; j < 8; j++) {
        var b0 = data[tile_idx++];
        var b1 = data[tile_idx++];
        var w0 = b0 * 0x100 + b1;
        for(var i = 0; i < 8; i++) {
          var p = (w0 >> (15 - i)) & 0x1;
          p = p << 1;
          var q = (w0 >> (7 - i)) & 0x1;
          p = p | q;
          k = 4 * ((y * 8 + j) * canvas.width + x * 8 + i);
          idata[k++] = p * 80;
          idata[k++] = p * 80;
          idata[k++] = p / 2 ? 0 : 255;
          idata[k++] = 255;
        }
      }
    }
  }
  ctx.putImageData(image_buffer, 0, 0);
}

function push_frame(category, data, len) {
  var buf = new Uint8Array(memory.buffer.slice(data, data + len));
  if (category == 0x100) { // tile data
    draw_vram(tile0, buf);
    tile_maps[0] = buf;
  }
  if (category == 0x101) { // tile data
    draw_vram(tile1, buf);
    tile_maps[1] = buf;
  }
  if (category == 0x102) { // tile data
    draw_vram(tile2, buf);
    tile_maps[2] = buf;
  }
  if (category == 0x300) { // screen
    draw_display(cvscreen, buf, len);
  }
  if (category == 0x200) { // background
    var ctx = bg.getContext("2d");
    var image_buffer = bg._buffer, k = 0;
    var ctx = bg.getContext("2d");
    if (!image_buffer)
      image_buffer = bg._buffer = ctx.createImageData(160, 144);
    var idata = image_buffer.data;
    var w = 8, i = 0, cc = {};
    for(var y = 0; y < 32; y++) {
      for(var x = 0; x < 32; x++) {
        var tile = buf[i++]
        if (tile < 0x80) {
          ctx.drawImage(
            tile0, 0x8 * (tile % 0x10), 8 * (tile / 0x10 | 0),
            w, w,
            x * w, y * w, w, w);
        }
        else if (tile < 0x100) {
          var t = tile - 0x80;
          ctx.drawImage(
            tile1, 0x8 * (t % 0x10), 8 * (t / 0x10 | 0),
            w, w,
            x * w, y * w, w, w);
        }
        else {
          ctx.fillStyle = ['red', 'blue','green', 'pink'][tile % 4]
          ctx.fillRect(x * w, y * w, w, w);
        }
      }
    }
    // ctx.putImageData(image_buffer, 0, 0);
  }
}
var error = 0;
var env = {
  _test: function(foobar) { console.log(foobar); },
  _logf: function(f) { line.push(f); },
  _logx8: function(f) { line.push(('00' + f.toString(16)).substr(-2)); },
  _logx16: function(f) { line.push(('0000' + f.toString(16)).substr(-4)); },
  _logx32: function(f) { line.push(f.toString(16)); },
  _logs: function(i, len) {
    if (!len) { line.push(i.toString(16)); return; }
    line.push(dec.decode(memory.buffer.slice(i, i + len)));
  },
  _showlog: function() { console.log.apply(console, line); line = []; },
  _push_frame: push_frame,
  _stop: function() { chkPaused.checked = true; },
  _check_memory_size: function () { 
    console.log(
      'byte size', memory.buffer.byteLength,
      'page size', memory.buffer.byteLength / 64 / 1024);
  },
  _serial_putc: function(c) { console.log("Serial", String.fromCharCode(c)); },
  memory:memory,
};
fetch("build/gb_emulator.wasm")
  .then((resp) => resp.arrayBuffer())
  .then((wasm) => WebAssembly.instantiate(wasm, { env: env }))
  .then((module) => {
    instance = module.instance;

    emulator = instance.exports.get_emulator();
    let DRAW = 1;
    let DEBUG = 2;
    let ERROR = 3;
    function loop() {
      if (chkPaused.checked) return;
      instance.exports.step_frame(emulator);
      requestAnimationFrame(loop);
    }
    requestAnimationFrame(loop);
    chkPaused.addEventListener("change", loop);

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
      }
      else if (e.keyCode in key_map)
        instance.exports.button_down(emulator, key_map[e.keyCode]);
      else
        ; // console.log("keydown", e.key, e.keyCode);
    });
    document.addEventListener("keyup", e => {
      if (e.keyCode in key_map)
        instance.exports.button_up(emulator, key_map[e.keyCode]);
    });
    
  });
