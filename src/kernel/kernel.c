void cpu_idle() {
    for (;;) __asm__ __volatile__ ("hlt");
}

void kernel_main () {
    cpu_idle();
}