#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <limits.h>

enum {
  TK_NOTYPE = 256, TK_EQ, NUM, DEREF, NEG, HEX, REG, TK_NEQ, AND, OR, NOT

  /* TODO: Add more token types */

};
//定义token的类型和匹配规则
static struct rule {
  char *regex;
  int token_type;
  int level;
  int single;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE, 100, false},    // spaces
  {"\\+", '+', 70, false},
  {"\\-", '-', 70, false},  
  {"\\*", '*', 80, false}, 
  {"\\/", '/', 80, false}, 
  {"\\(", '(', 100, false}, 
  {"\\)", ')', 100, false},
  {"0x[0-9a-fA-F]{1,32}", HEX, 100, false}, 
  {"\\$[a-zA-Z]+", REG, 100, false},
  {"[0-9]{1,32}", NUM, 100, false},
  {"==", TK_EQ, 60, false}, 
  {"!=", TK_NEQ, 60, false},
  {"&&", AND, 50, false},
  {"\\|\\|", OR, 50, false},
  {"\\!", NOT, 60, true},
  {"\\*", DEREF, 90, true},
  {"-", NEG, 90, true},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];
//检查并编译以上定义的正则表达式
/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
  int level;
  bool single;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
		
        switch (rules[i].token_type) {
			case '*':	
			case '+':	
			case '-':	
			case '/':
			case '(':	
			case ')':	
			case NUM:	
			case HEX:
			case REG:
			case TK_EQ:
			case TK_NEQ:
			case AND:
			case OR:
			case NOT:
				tokens[nr_token].type = rules[i].token_type;
				tokens[nr_token].level = rules[i].level;
				tokens[nr_token].single = rules[i].single;
				for (int j = 0; j < substr_len; j++)
					tokens[nr_token].str[j] = substr_start[j];	
				nr_token++;
				break;
			default:
				break;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q)
{
	if (tokens[p].type == '(' && tokens[q].type == ')')
	{
		int pair_num = 0;
		for (int i = p + 1; i < q; i++)
		{
			if (tokens[i].type == '(')
				pair_num++;
			if (tokens[i].type == ')')
				pair_num--;
			if (pair_num < 0)
				return false;
		}			
	}
	else
		return false;
	return true;
}

int get_dominant_op(int p, int q)
{
	int inper = 0;
	int min_prio = 100;

	for (int i = p; i <= q; i++) 
	{
		if (tokens[i].type == '(') inper++;
		if (tokens[i].type == ')') inper--;
		if ((tokens[i].level < min_prio) && (inper == 0)) 
			min_prio = tokens[i].level;
	}
	for (int i = q; i >= p; i--)
	{
		if (tokens[i].type == '(') inper++;
		if (tokens[i].type == ')') inper--;
		if ((tokens[i].level == min_prio) && (inper == 0))
		{
			if (tokens[i].single == true)
				while (i > p && tokens[i-1].single == true)
					i--;
			return i;
		}
	}
	panic("zmf: Can't find dominate op");
	return 0;
	/*
	if (cut == p) 
		for (int i = p; i <= q; i++) 
		{
			if (tokens[i].type == '(') inper++;
			if (tokens[i].type == ')') inper--;
			if ((tokens[i].type == '*' || tokens[i].type == '/') && (inper == 0))
				cut = i;
		}
	if (cut == p) 
		for (int i = p; i <= q; i++) 
		{
			if (tokens[i].type == '(') inper++;
			if (tokens[i].type == ')') inper--;
			if ((tokens[i].type == TK_EQ) && (inper == 0)) 
				cut = i;
		}
	return cut;*/
}

uint32_t eval(int p, int q)
{
	if (p > q)
		panic("zmf: Bad expression!");
	else if (p == q)
	{
		int val = 0;
		if (tokens[p].type == NUM)
		{
			val = atoi(tokens[p].str);
			return val;
		}
		else if (tokens[p].type == HEX)
		{
			for(int i = 2; i < strlen(tokens[p].str); ++ i) 
			{
				if(tokens[p].str[i] >= '0' && tokens[p].str[i] <= '9')
					val = val * 16 + tokens[p].str[i] - '0'; 
				if(tokens[p].str[i] >= 'A' && tokens[p].str[i] <= 'F')
					val = val * 16 + tokens[p].str[i] - 'A' + 10; 
				if(tokens[p].str[i] >= 'a' && tokens[p].str[i] <= 'f')
					val = val * 16 + tokens[p].str[i] - 'a' + 10;
			}
			return val;
		}
		else if (tokens[p].type == REG)
		{
			char *reg = tokens[p].str + 1;
			if (!strcmp(reg,"eip"))
				return cpu.eip;
			else
			{
				for (int i = 0; i < 8; i++)
					if (!strcmp(reg, regsl[i]))
					{
						return cpu.gpr[i]._32;
						if (i == 7)
							panic("zmf: REG not available!");
					}
			}

		}

		else
			panic("zmf: Bad expression!");
	}
	else if (check_parentheses(p, q) == true)
		return eval(p + 1, q - 1);
	else
	{
		int cut = get_dominant_op(p, q);
		//printf("breakimp\n");
		int op_type = tokens[cut].type;
		//printf("fcut = %d\n", cut);
		//assert(0);

		int val1 = eval(p, cut - 1);
		//printf("val1= %d\n",val1);
		int val2 = eval(cut + 1, q);
		//printf("val2= %d\n",val2);

		switch(op_type)
		{
			case '+':
				printf("+: %d\n", val1+val2);
				return val1 + val2;
			case '-':				
				printf("-: %d\n", val1-val2);
				return val1 - val2;
			case '*':
				printf("*: %d\n", val1*val2);
				return val1 * val2;
			case '/':				
				printf("/: %d\n", val1/val2);
				return val1 / val2;
			case TK_EQ:	
				return val1 == val2;
			case NEG:
				return -val1;
			case DEREF:
				return vaddr_read(val1, 1);
			case AND:
				return val1 && val2;
			case OR:
				return val1 || val2;
			case NOT:
				return !val1;
			default:
				assert(0);
		}
	}
	panic("zmf: Something wrong?");
	return 0;
}







uint32_t expr(char *e, bool *success) {
  for (int i = 0; i < nr_token; i++)
  {
	  tokens[i].type = 0;
	  tokens[i].level = 0;
	  tokens[i].single = 0;
	  memset(tokens[i].str, 0, sizeof(tokens[i].str));
  }

  if (!make_token(e)) {
	  *success = false;
	  return 0;
  }
  for(int i = 0; i < nr_token; i++) 
  {
	int adj_type = 0;
	if (i > 0)
		adj_type = tokens[i-1].type;
	else
		adj_type = tokens[i].type;
	if (tokens[i].type == '*' && (i == 0 || (adj_type != '(' && adj_type != ')' && adj_type != HEX && adj_type != REG && adj_type != NUM )))
	{	
		tokens[i].type = DEREF;
		tokens[i].level = 90;
		tokens[i].single = true;
	}
	if (tokens[i].type == '-' && (i == 0 || (adj_type != '(' && adj_type != ')' && adj_type != HEX && adj_type != REG && adj_type != NUM ))) 
	{
		tokens[i].type = NEG;
		tokens[i].level = 90;
		tokens[i].single = true;
	}
  }

  printf("scan:\n");
  for (int i = 0; i < 10; i++)
	  printf("%d %s\n", tokens[i].type, tokens[i].str);

  return eval(0, nr_token - 1);  

  return 0;
}
