#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <limits.h>
#include <stdio.h>
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

static int cmd_info(char *args)
{
	if (!strcmp(args,"r"))
	{
		printf("eax:0x%08X  ecx:0x%08X  edx:0x%08X  ebx:0x%08X\nesp:0x%08X  ebp:0x%08X  esi:0x%08X  edi:0x%08X\n",cpu.eax, cpu.ecx, cpu.edx, cpu.ebx, cpu.esp, cpu.ebp, cpu.esi, cpu.edi);
		printf("eip:0x%08X\n", cpu.eip);
	}
	return 0;
}

static int cmd_p(char *args)
{
	bool success = true;
	printf("%d\n",expr(args,&success));
	return 0;
}

static int cmd_w(char *args)
{
	bool success = true;
	printf("%d\n", watch(args,&success));
	return 0;
}

static int cmd_x(char *args)
{
	char *xnum = strtok(args, " ");
	char *to_cal = xnum + strlen(xnum) + 1;
	int n_byte = atoi(xnum);
	n_byte = INT_MAX;

	sscanf(xnum, "%d", &n_byte);
	printf("%d %s\n", n_byte, to_cal);

	if (n_byte == INT_MAX || to_cal == NULL)
		return 0;
	else
	{
		bool success = true;
		uint32_t addr = expr(to_cal, &success);
	
		for (int j = 0; j < n_byte; j++)
		{
			printf("0x%08x: ", addr + 4*j);
			printf("0x ");
			for (int i = 0; i < 4; i++)
			{
				int val = vaddr_read(addr + 4*j + i, 1);
				printf("%02X ", val);
			}
			printf("\n");
		}
		return 0;
	}

	return 0;
}


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help",	 "Display informations about all supported commands"	, cmd_help },
  { "c",	 "Continue the execution of the program"				, cmd_c },
  { "q",	 "Exit NEMU"											, cmd_q },
  { "si",	 "Step for [N] times"									, cmd_si },
  { "info",	 "Information for reg/watch"							, cmd_info },
  { "x",	 "将expr结果作为起始内存地址, 以十六进制输出N个4字节"	, cmd_x },
  { "p",	 "打印表达式expr"										, cmd_p },
  { "w",	 "监视表达式内容,若变化则暂停程序执行"					, cmd_w },

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
