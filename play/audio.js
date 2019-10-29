var audio, gain, wave_source, wasm_buffer_ptr, wasm_buffer_view;

function on_audio_update (e) {
  // Get array associated with the output ports.
  var left = e.outputBuffer.getChannelData(0);
  var right = e.outputBuffer.getChannelData(1);
  var available_frames = left.length;

  instance.exports.spgb_audio_sample(
    emulator, audio.sampleRate, 2, available_frames, wasm_buffer_ptr);

  wasm_buffer_view = new Float32Array(memory.buffer, wasm_buffer_ptr, 2 * 4096);
  for (var i = 0; i < available_frames; ++i) {
    left[i] = wasm_buffer_view[2 * i];
    right[i] = wasm_buffer_view[2 * i + 1];
  }
}

function start_audio() {
  if (!audio) {
    audio = new AudioContext();
    //var buffer = audio.createBuffer(2, 4800, audio.sampleRate);
    wave_source = audio.createScriptProcessor(4096, 0, 2);
    wave_source.onaudioprocess = on_audio_update;
    gain = audio.createGain();
    gain.gain.value = 0.1;
    wave_source.connect(gain);
    gain.connect(audio.destination);

    wasm_buffer_ptr = instance.exports.spgb_audio_create_buffer(2 * 4096);
  }
  audio.resume();
};
