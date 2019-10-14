/// handles uploading roms
function encode(buf, start, len) {
  if (len <= 0x8000)
    return String.fromCharCode.apply(null, new Uint8Array(buf, start, len));
  return encode(buf, 0, 0x8000) + encode(buf, 0x8000, len - 0x8000);
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
          let count = localStorage.getItem("rom_count") | 0;
          localStorage.setItem("rom_" + count + "_name", file.name);
          localStorage.setItem("rom_" + count + "_data", encode(buffer, 0, buffer.byteLength));
          localStorage.setItem("rom_count", count + 1);
          list_object.redraw();
        });
    });
  }

  redraw() {
    this.file_list.innerHTML = "";
    let count = localStorage.getItem("rom_count");
    for(let i = 0; i < count; i++) {
      let file_name = localStorage.getItem("rom_" + i + "_name");
      let li = document.createElement('li');
      let a =  document.createElement('a');
      a.innerHTML = file_name;
      a.href = "#";
      a.addEventListener("click", e => {
        let li = a.parentElement;
        console.log(this);
        let index = li.dataset.index;
        let file_name = localStorage.getItem("rom_" + index + "_name");
        let file_data = localStorage.getItem("rom_" + index + "_data");
        file_data = new Uint8Array(decode(file_data));
        let rom_ptr = instance.exports.get_rom_area(emulator, file_data.byteLength);
        let rom = new Uint8Array(memory.buffer);
        console.log("js-loading", index, file_name, rom_ptr);
        for(let i = 0; i < file_data.byteLength; i++) {
          rom[rom_ptr + i] = file_data[i];
        }
        instance.exports.reset(emulator, rom_ptr, file_data.byteLength);
        return false;
      });
      li.appendChild(a);
      li.dataset.index = i;
      this.file_list.appendChild(li);
    }
    if (!count) { this.file_list.innerHTML = "No Saved Roms"; }
  }

  
};

  
