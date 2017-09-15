#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

//PA1 si
static int cmd_si(char *args) {
	uint64_t sinum = 1;
	if (args == NULL)
	{
		cpu_exec(1);
		return 0;
	}
	int sinum_int = atoi(args);
	sinum = sinum_int; 
	cpu_exec(sinum);
	return 0;
}

//PA1 info r
static int cmd_info(char *args)
{
	if (!strcmp(args,"r"))
		printf("eax:0x%X  ecx:0x%X  edx:0x%X  edx:0x%X\nebx:0x%X  esp:0x%X  ebp:0x%X  esi:0x%X\n",cpu.eax, cpu.ecx, cpu.edx, cpu.ebx, cpu.esp, cpu.ebp, cpu.esi, cpu.edi);
	return 0;
}

//PA1 x N EXPR
int XtoD(int he)
{   
	int re = 0;   // 保存转换为10进制的结果
	int k = 16;   // 16进制
	int n = 1;    // 位权
	while(he != 0)  
	{
		re += (he%10)*n;  // 取出各位位码值，并乘以对应的位权值
		he /= 10;   // 去掉16进制数的最低位，次低位变为最低位
		n *= k;     // 位权乘以16
	}
	return re; // 输出转换后的结果
}
static int cmd_x(char *args)
{
	char *xnum = strtok(args, " ");
	char *xdir = xnum + strlen(xnum) + 3;
	int xnum_int = atoi(xnum);

	int xdir_int = XtoD(atoi(xdir));
	uint32_t xdir_u = xdir_int;
	for (int i = 0; i < 4*(xnum_int); i++)
	{
		//if (i % 4 == 0)
		//	printf("0x");
		printf("%X",vaddr_read(xdir_u+i, 1));
		if (i % 4 == 0)
			printf("\n");
	}
	//printf("0x%X\n",vaddr_read(xdir_u+ xnum_int - , 1));

	return 0;
}


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step for [N] times", cmd_si },
  { "info", "Information for reg/watch", cmd_info },
  { "x", "求出表达式 EXPR 的值, 将结果作为起始内存地址, 以十六进制形式输出连续的N个4字节", cmd_x },

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
