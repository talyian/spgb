/// handles uploading roms
function encode(buf, start, len) {
  if (len <= 0x1000)
    return String.fromCharCode.apply(null, new Uint8Array(buf, start, len));
  return encode(buf, start, 0x1000) + encode(buf, start + 0x1000, len - 0x1000);
}

function decode(str) {
  let array = new ArrayBuffer(str.length);
  let data = new Uint8Array(array);
  for(let i = 0; i < str.length; i++)
    data[i] = str.charCodeAt(i);
  return array;
}
            
class FileList {
  constructor(file_upload, file_list) {
    this.file_upload = file_upload;
    this.file_list = file_list;
    let list_object = this;
    file_upload.addEventListener("change", function(e) {
      let file = file_upload.files[0];
      file.arrayBuffer()
        .then(buffer => {
          let encoded = encode(buffer, 0, buffer.byteLength);
          let count = localStorage.getItem("rom_count") | 0;
          localStorage.setItem("rom_" + count + "_name", file.name);
          localStorage.setItem("rom_" + count + "_data", encode(buffer, 0, buffer.byteLength));
          localStorage.setItem("rom_count", count + 1);
          list_object.redraw();
          list_object.load_cart(buffer);
        });
    });
  }

  // Given an ArrayBuffer, copy it into a WASM-accessible buffer and run it
  load_cart(arrayBuffer) {
    let file_data = new Uint8Array(arrayBuffer);
    let offset = instance.exports.spgb_allocate(emulator, file_data.byteLength);
    let rom = new Uint8Array(memory.buffer);
    for(let i = 0; i < file_data.byteLength; i++) {
      rom[offset + i] = file_data[i];
    }
    instance.exports.spgb_load_cart(emulator, offset, file_data.byteLength);
    if (audio) audio.resume();
  }

  redraw() {
    this.file_list.innerHTML = "";
    let list_object = this;
    let count = localStorage.getItem("rom_count");
    for(let i = 0; i < count; i++) {
      let file_name = localStorage.getItem("rom_" + i + "_name");
      let li = document.createElement('li');
      let a =  document.createElement('a');
      a.innerHTML = file_name;
      a.href = "#";
      a.addEventListener("click", e => {
        let index = a.parentElement.dataset.index;
        let file_name = localStorage.getItem("rom_" + index + "_name");
        let file_data = localStorage.getItem("rom_" + index + "_data");
        list_object.load_cart(decode(file_data))
        return false;
      });
      li.appendChild(a);
      li.dataset.index = i;
      this.file_list.appendChild(li);
    }
    if (!count) { this.file_list.innerHTML = "No Saved Roms"; }
  }
};

  
