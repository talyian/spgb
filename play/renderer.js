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
            p += 0x100 * data[i++];
            idata[j++] = 8 * (p & 0x1F);
            idata[j++] = 8 * ((p >>= 5) & 0x1F);
            idata[j++] = 8 * ((p >>= 5) & 0x1F);
            idata[j++] = 255;
        }
    }
    ctx.putImageData(image_buffer, 0, 0);
}

function draw_vram(canvas, data) {
  var ctx = canvas.getContext("2d");
  var image_buffer = canvas._buffer;
  if (!image_buffer) {
    image_buffer = ctx.createImageData(canvas.width, canvas.height);
    canvas._buffer = image_buffer;
  }

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

let tile_maps = [0, 0, 0];
function push_frame(category, data, len) {
  var buf = new Uint8Array(memory.buffer.slice(data, data + len));
  if (category == 0x100) { // tile data
    // draw_vram(tile0, buf);
    tile_maps[0] = buf;
  }
  if (category == 0x101) { // tile data
    // draw_vram(tile1, buf);
    tile_maps[1] = buf;
  }
  if (category == 0x102) { // tile data
    // draw_vram(tile2, buf);
    tile_maps[2] = buf;
  }
  if (category == 0x300) { // screen
    draw_display(cvscreen, buf, len);
  }
  if (category == 0x200) { // background
    return;
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
