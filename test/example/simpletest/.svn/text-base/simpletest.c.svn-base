#define MAGIC_INSTRUCTION __asm__ __volatile__ ("xchg %bx,%bx");

void arraytest() {
  int array[10000], i;

  for(i = 0; i < 10000; ++i) {
    array[i] = i * i;
  }
}

int main(int argc, char **argv) {
  MAGIC_INSTRUCTION;

  arraytest();

  MAGIC_INSTRUCTION;

  arraytest();
  
  MAGIC_INSTRUCTION;

  return 0;
}
