#include <stdint.h>

typedef struct registers {
  uint64_t ds;
  uint64_t rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi;
  uint64_t int_no, err_code;
  uint64_t rip, cs, eflags, userrsp, ss;

} registers_t;

void isr_handler(registers_t regs);

typedef void (*isr_t)(registers_t);
void register_interrupt_handler(uint8_t n, isr_t handler);
void timer_handler(registers_t regs);
