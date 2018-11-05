

struct IO {

  void write(u8 port, u8 data) {
    switch(port) {
    default:
      printf("Unknown IO Port: %O\n", port);
    }
  }
};
