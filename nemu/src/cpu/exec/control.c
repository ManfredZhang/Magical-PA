#include "cpu/exec.h"
#include "cpu/rtl.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  printf("decoding.opcode = %u, subcode = %u\n", decoding.opcode, subcode);
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // call_rel32
  // 1: 跳转
  // the target address is calculated at the decode stage 
  decoding.jmp_eip = *eip + id_dest->val;
  decoding.is_jmp = 1;

  // 2：保存返回地址
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, *eip);

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  decoding.jmp_eip = vaddr_read(cpu.esp,4);
  decoding.is_jmp = 1;
  cpu.esp += 4;
  print_asm("ret");
}

make_EHelper(call_rm) {
  TODO();

  print_asm("call *%s", id_dest->str);
}
