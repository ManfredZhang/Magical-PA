#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  //TODO();
  rtl_push(&cpu.EFLAGS);
  t0 = cpu.cs;
  rtl_push(&t0);
  rtl_push(&ret_addr);

  uint32_t offset_high = vaddr_read(cpu.idtr.base + NO*8, 4) & 0xffff0000;
  uint32_t offset_low = vaddr_read(cpu.idtr.base + NO*8 + 4, 4) & 0x0000ffff;

  decoding.is_jmp = 1;
  decoding.jmp_eip = offset_high | offset_low;
}

void dev_raise_intr() {
}
