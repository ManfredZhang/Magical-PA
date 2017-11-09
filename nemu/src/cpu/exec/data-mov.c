#include "cpu/exec.h"
#include "monitor/expr.h"

//zmf: TODO?
make_EHelper(xchg) {
	//printf("zzmf: id_src->val = %u, id_dest->val = %u, id_src2->val = %u\n", id_src->val, id_dest->val, id_src2->val);
	//t0 = id_src->val;
	//operand_write(id_src, &id_dest->val);
    //operand_write(id_dest, &t0);
	
	print_asm_template2(xchg);
}

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //TODO();
  rtl_push(&(id_dest->val));
  //operand_write(id_dest, &id_src->val);
  print_asm_template1(push);
  
}

make_EHelper(pop) {
  //TODO();
  rtl_pop(&id_src->val);
  operand_write(id_dest, &id_src->val);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  //TODO();
  uint32_t temp = cpu.esp;
  
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&temp);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  //TODO();
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t0);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {
  //TODO();
  //uint32_t data_temp = vaddr_read(cpu.ebp, 4);
  //vaddr_write(cpu.esp, 4, data_temp);
  cpu.esp = cpu.ebp;
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    //TODO();
	if (cpu.ax < 0)
		cpu.dx = 0x0ffff;
	else
		cpu.dx = 0;
  }
  else {
    //TODO();
	if (cpu.eax < 0)
		cpu.edx = 0xffffffff;
	else
		cpu.edx = 0;
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
	//TODO();
    cpu.ax = (int16_t)((int8_t) cpu.al);
  }
  else {
	//TODO();
	cpu.eax = (int32_t)((int16_t)cpu.ax);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
