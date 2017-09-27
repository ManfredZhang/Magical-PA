#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;	
// head指向最新添加到监视中链表的WP
// free指向下一个要new的监视点

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
	if (free_ == NULL)
		assert(0);
	else
	{
		WP *a_new_wp = free_;
		if (head == NULL)
			head = free_;
		else
		{
			head -> next = head;
			head = free_;
		}
		free_ = free_ -> next;
		return a_new_wp;
	}
}

void free_wp(WP* wp)
{
	TODO();
}

uint32_t watch(char* args, bool* success)
{
	WP* a_new_wp = new_wp();
	a_new_wp -> record_expr = args;
	a_new_wp -> current_val = expr(args, success);
	printf("Created a new watchpoint #%d", a_new_wp -> NO);

	return a_new_wp -> current_val;
}

