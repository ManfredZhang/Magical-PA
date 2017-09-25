#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp()
{
	if (free_ -> NO == 0)
		printf("go\n");
	return free_;
}

void free_wp(WP* wp)
{
	TODO();
}

uint32_t watch(char* args, bool* success)
{
	WP* wp_new = new_wp();
	wp_new -> record_expr = args;
	wp_new -> current_val = expr(args, success);

	return 0;
}

