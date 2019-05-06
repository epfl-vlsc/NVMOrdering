struct LogEntry {
  __attribute((annotate("sent"))) int Size : 30;
  __attribute((annotate("x"))) int Type : 2;
} __attribute((annotate("lol")));