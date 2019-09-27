  function draw_display(ctx, data) {
    var p = 0, i = 0;
    var H = 144, W = 160;
    for(var y = 0; y < H; y++) {
      for(var x = 0; x < W; x++) {
        p = data[i++];
        ctx.fillStyle = ['white', 'blue', 'olive', 'black'][p % 3];
        ctx.fillRect(x, y, 1, 1);
      }
    }
  }


function Renderer(canvas) {
    this.ctx_2d = canvas.getContext("2d");
    this.gl = canvas.getContext("webgl");
}

Renderer.prototype.set_display = function(data, len) {
    draw_display(this.ctx_2d, data);
}
