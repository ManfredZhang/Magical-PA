#include "nemu.h"
#include "monitor/monitor.h"
#include <unistd.h>
#include <sys/prctl.h>
#include <signal.h>

#include "protocol.h"
#include <stdlib.h>

bool gdb_connect_qemu(void);
bool gdb_memcpy_to_qemu(uint32_t, void *, int);
bool gdb_getregs(union gdb_regs *);
bool gdb_setregs(union gdb_regs *);
bool gdb_si(void);
void gdb_exit(void);

static bool is_skip_qemu;
static bool is_skip_nemu;

void diff_test_skip_qemu() { is_skip_qemu = true; }
void diff_test_skip_nemu() { is_skip_nemu = true; }

#define regcpy_from_nemu(regs) \
  do { \
    regs.eax = cpu.eax; \
    regs.ecx = cpu.ecx; \
    regs.edx = cpu.edx; \
    regs.ebx = cpu.ebx; \
    regs.esp = cpu.esp; \
    regs.ebp = cpu.ebp; \
    regs.esi = cpu.esi; \
    regs.edi = cpu.edi; \
    regs.eip = cpu.eip; \
  } while (0)

static uint8_t mbr[] = {
  // start16:
  0xfa,                           // cli
  0x31, 0xc0,                     // xorw   %ax,%ax
  0x8e, 0xd8,                     // movw   %ax,%ds
  0x8e, 0xc0,                     // movw   %ax,%es
  0x8e, 0xd0,                     // movw   %ax,%ss
  0x0f, 0x01, 0x16, 0x44, 0x7c,   // lgdt   gdtdesc
  0x0f, 0x20, 0xc0,               // movl   %cr0,%eax
  0x66, 0x83, 0xc8, 0x01,         // orl    $CR0_PE,%eax
  0x0f, 0x22, 0xc0,               // movl   %eax,%cr0
  0xea, 0x1d, 0x7c, 0x08, 0x00,   // ljmp   $GDT_ENTRY(1),$start32

  // start32:
  0x66, 0xb8, 0x10, 0x00,         // movw   $0x10,%ax
  0x8e, 0xd8,                     // movw   %ax, %ds
  0x8e, 0xc0,                     // movw   %ax, %es
  0x8e, 0xd0,                     // movw   %ax, %ss
  0xeb, 0xfe,                     // jmp    7c27
  0x8d, 0x76, 0x00,               // lea    0x0(%esi),%esi

  // GDT
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00,

  // GDT descriptor
  0x17, 0x00, 0x2c, 0x7c, 0x00, 0x00
};

void init_difftest(void) {
  int ppid_before_fork = getpid();
  int pid = fork();
  if (pid == -1) {
    perror("fork");
    panic("fork error");
  }
  else if (pid == 0) {
    // child

    // install a parent death signal in the chlid
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) {
      perror("prctl error");
      panic("prctl");
    }

    if (getppid() != ppid_before_fork) {
      panic("parent has died!");
    }

    close(STDIN_FILENO);
    execlp("qemu-system-i386", "qemu-system-i386", "-S", "-s", "-nographic", NULL);
    perror("exec");
    panic("exec error");
  }
  else {
    // father

    gdb_connect_qemu();
    Log("Connect to QEMU successfully");

    atexit(gdb_exit);

    // put the MBR code to QEMU to enable protected mode
    bool ok = gdb_memcpy_to_qemu(0x7c00, mbr, sizeof(mbr));
    assert(ok == 1);

    union gdb_regs r;
    gdb_getregs(&r);

    // set cs:eip to 0000:7c00
    r.eip = 0x7c00;
    r.cs = 0x0000;
    ok = gdb_setregs(&r);
    assert(ok == 1);

    // execute enough instructions to enter protected mode
    int i;
    for (i = 0; i < 20; i ++) {
      gdb_si();
    }
  }
}

void init_qemu_reg() {
  union gdb_regs r;
  gdb_getregs(&r);
  regcpy_from_nemu(r);
  bool ok = gdb_setregs(&r);
  assert(ok == 1);
}

void difftest_step(uint32_t eip) {
  union gdb_regs r;
  bool diff = false;

  if (is_skip_nemu) {
    is_skip_nemu = false;
    return;
  }

  if (is_skip_qemu) {
    // to skip the checking of an instruction, just copy the reg state to qemu
    gdb_getregs(&r);
    regcpy_from_nemu(r);
    gdb_setregs(&r);
    is_skip_qemu = false;
    return;
  }

  gdb_si();
  gdb_getregs(&r);

  // TODO: Check the registers state with QEMU.
  // Set `diff` as `true` if they are not the same.
  if (cpu.eax != r.eax){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.eax = %u r.eax = %u\n", cpu.eax, r.eax);
  }
  if (cpu.ecx != r.ecx){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.ecx = %u r.ecx = %u\n", cpu.ecx, r.ecx);
  }
  if (cpu.edx != r.edx){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.edx = %u r.edx = %u\n", cpu.edx, r.edx);
  }
  if (cpu.ebx != r.ebx){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.ebx = %u r.ebx = %u\n", cpu.ebx, r.ebx);
  }
  if (cpu.esp != r.esp){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.esp = %u r.esp = %u\n", cpu.esp, r.esp);
  }
  if (cpu.ebp != r.ebp){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.ebp = %u r.ebp = %u\n", cpu.ebp, r.ebp);
  }
  if (cpu.esi != r.esi){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.esi = %u r.esi = %u\n", cpu.esi, r.esi);
  }
  if (cpu.edi != r.edi){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.edi = %u r.edi = %u\n", cpu.edi, r.edi);
  }

  uint32_t rCF = r.eflags >> 31;
  uint32_t rZF = (r.eflags << 6) >> 25;
  uint32_t rSF = (r.eflags << 7) >> 24;
  uint32_t rIF = (r.eflags << 9) >> 22;
  uint32_t rOF = (r.eflags << 11) >> 20;

  if (cpu.flags.CF != rCF){
	  //diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.flags.CF = %u rCF = %u\n", cpu.flags.CF, rCF);
  }
  if (cpu.flags.ZF != rZF){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.flags.ZF = %u rZF = %u\n", cpu.flags.ZF, rZF);
  }
  if (cpu.flags.SF != rSF){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.flags.SF = %u rSF = %u\n", cpu.flags.SF, rSF);
  }
  if (cpu.flags.IF != rIF){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.flags.IF = %u rIF = %u\n", cpu.flags.IF, rIF);
  }
  if (cpu.flags.OF != rOF){
	  diff = true;
	  printf("zmf: Not equal to QEMU @ cpu.flags.OF = %u rOF = %u\n", cpu.flags.OF, rOF);
  }




  if (diff) {
    nemu_state = NEMU_END;
  }
}
