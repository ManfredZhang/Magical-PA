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
		free_ = free_ -> next;

		a_new_wp -> next = head;
		head = a_new_wp;
		return a_new_wp;
	}
}

void free_wp(int watch_num)
{
	WP *temp_head = head;
	WP *temp_head_2 = head;

	if (watch_num > 9998)
	{
		init_wp_pool();
		printf("zmf: No more watchpoints left!\n");
		return;
	}

	while (temp_head != NULL)
	{
		//printf("temp head num %d\n", temp_head -> NO);
		if (temp_head -> NO != watch_num)
		{	
			temp_head = temp_head -> next;
			continue;
		}
		if (head == temp_head)
		{
			head = temp_head -> next;
			temp_head -> next = free_;
			free_ = temp_head;
			printf("Watchpoint #%d was deleted successfully\n", watch_num);
			if (head == NULL)
				printf("zmf: No more watchpoints left\n");
			return;
		}
		while (temp_head_2 -> next != temp_head)
			temp_head_2 = temp_head_2 -> next;
		temp_head_2 -> next = temp_head -> next;
		temp_head -> next = free_;
		free_ = temp_head;
		printf("Watchpoint #%d was deleted successfully\n", watch_num);
		return;
	}
	printf ("zmf: Sorry, but there's no such watchpoint\n");
}

uint32_t watch(char *args, bool *success)
{
	WP* a_new_wp = new_wp();
	a_new_wp -> record_expr = args;
	a_new_wp -> current_val = expr(args, success);
	printf("Created a new watchpoint #%d\nvalue: ", a_new_wp -> NO);

	return a_new_wp -> current_val;
}

bool every_check()
{
	WP *temp_head = head;
	bool success = true;
	bool flag = false;
	while (temp_head != NULL)
	{
		int val = expr(temp_head -> record_expr, &success);
		if (val != temp_head -> current_val)
		{
			flag = true;
			printf("\nzmf stopped the nemu at watchpoint #%d: %s\n", temp_head -> NO, temp_head -> record_expr);
			printf("\nOld value = %d\n", temp_head -> current_val);
			printf("New value = %d\n", val);
		}
		temp_head = temp_head -> next;
	}
	return flag;
}



