void cpu_idle() {
    __asm__ __volatile__ ("hlt");
}

void kernel_main () {
    cpu_idle();
}