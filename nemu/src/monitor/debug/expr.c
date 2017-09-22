#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ, NUM, DEREF, NEG

  /* TODO: Add more token types */

};
//定义token的类型和匹配规则
static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},
  {"\\-", '-'},  
  {"\\*", '*'}, 
  {"\\/", '/'}, 
  {"\\(", '('}, 
  {"\\)", ')'}, 
  {"[0-9]{1,32}", NUM},
  {"==", TK_EQ}         // equal
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

  for (i = 0; i < NR_REGEX; i ++) {
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
			case '*':	// *
			case '+':	// +
			case '-':	// -
			case '/':	// /
			case '(':	// 258
			case ')':	// 259
			case NUM:	// 260
				tokens[nr_token].type = rules[i].token_type;
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
	int cut = p;

	for (int i = p; i <= q; i++)
	{
		if (tokens[i].type == '(')
		{
			while (tokens[i].type != ')')
				i++;
			i++;	//跳过右括号
		}
		if ((tokens[i].type == '*' || tokens[i].type == '/'))
			cut = i;
	}
	
	for (int i = p; i <= q; i++)
	{
		if (tokens[i].type == '(')
		{
			while (tokens[i].type != ')')
				i++;
			i++;
		}
		if ((tokens[i].type == '+' || tokens[i].type == '-'))
			cut = i;
	}

	return cut;
}

uint32_t eval(int p, int q)
{
	if (p > q)
		panic("zmf: Bad expression!");
	else if (p == q)
	{
		if (tokens[p].type == NUM)
		{
			int val = atoi(tokens[p].str);
			return val;
		}
		else
			panic("zmf: Bad expression!");
	}
	else if (check_parentheses(p, q) == true)
		return eval(p + 1, q - 1);
	else
	{
		int cut = get_dominant_op(p, q);
		printf("breakimp\n");
		int op_type = tokens[cut].type;
		//printf("fcut = %d\n", cut);

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
			default:
				assert(0);
		}
	}
	panic("zmf: Something wrong?");
	return 0;
}







uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  //printf("scan:\n");
  //for (int i = 0; i < 10; i++)
	//  printf("%d %s\n", tokens[i].type, tokens[i].str);
//printf("break1\n");
  for(int i = 0; i < nr_token; i++) 
  {
	int adj_type = 0;
	if (i > 0)
		adj_type = tokens[i-1].type;
	else
		adj_type = tokens[i].type;
	if (tokens[i].type == '*' && (i == 0 || adj_type =='+' || adj_type  == '-' || adj_type == DEREF || adj_type == '*' || adj_type == '/')) 
		tokens[i].type = DEREF;
	if (tokens[i].type == '-' && (i == 0 || adj_type =='+' || adj_type  == '-' || adj_type == NEG || adj_type == '*' || adj_type =='/')) 
		tokens[i].type = NEG;
  }
//printf("break2\n");

  //for (int i = 0; i < 10; i++)
	//  printf("scan: %d %s\n", tokens[i].type, tokens[i].str); 

  return eval(0, nr_token - 1);  

  return 0;
}
