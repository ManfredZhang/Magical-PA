#include "cpu/exec.h"

make_EHelper(rol) {
  /*uint32_t temp = id_dest->width * 8;
  t0 = id_src->val % temp;
  t1 = (id_dest->val << t0) | (id_dest->val >> (temp-t0));
  operand_write(id_dest, &t1);
  t1 = (t1 & 0x00000001) == 1;
  rtl_set_CF(&t2);
  TODO();*/
  uint32_t temp = id_src->val % (id_dest->width * 8);
  while (temp) {
	  uint8_t tmpcf = id_dest->val >> (id_dest->width*8 - 1);
	  id_dest->val = id_dest->val * 2 + tmpcf;
	  temp--;
  }
  if (id_src->val % (id_dest->width * 8) == 1) {
	  if ((id_dest->val >> (id_dest->width*8 - 1)) != cpu.flags.CF)
		  cpu.flags.OF = 1;
	  else
		  cpu.flags.OF = 0;
  }
  t1 = (t1 & 0x00000001) == 1;
  rtl_set_CF(&t2);

  print_asm_template2(rol);
}
//paddr_write
make_EHelper(test) {
  //TODO();
  rtl_and(&t2, &id_dest->val, &id_src->val);
  //operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  cpu.flags.CF = 0;
  cpu.flags.OF = 0;

  print_asm_template2(test);
}

make_EHelper(and) {
  //TODO();
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  cpu.flags.CF = 0;
  cpu.flags.OF = 0;

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  //printf("zzmf: id_dest->imm = %u, t2 = %u\n", id_dest->imm, t2);
  
  rtl_update_ZFSF(&t2, id_dest->width);
  cpu.flags.CF = 0;
  cpu.flags.OF = 0;
  print_asm_template2(xor);
}

make_EHelper(or) {
  //TODO();
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  cpu.flags.CF = 0;
  cpu.flags.OF = 0;


  print_asm_template2(or);
}

make_EHelper(sar) {
  //TODO();
  // unnecessary to update CF and OF in NEMU

  if (id_dest->width == 1)
	  id_dest->val = (int8_t)id_dest->val;
  else if (id_dest->width == 2)
	  id_dest->val = (int16_t)id_dest->val;
     
  rtl_sar(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  //TODO();
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}
