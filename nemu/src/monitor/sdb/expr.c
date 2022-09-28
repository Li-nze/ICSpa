/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
//hlz add
#include <string.h>

enum {
  TK_NUM=0, TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

	{"[0-9]+", TK_NUM},	// decimal number
	{"\\/", '/'},		// divide
	{"\\*", '*'},		// multiply
	{"-", '-'},			// minus
	{"\\)", ')'},		// right parenthese:q
						//
	{"\\(", '('},		// left parenthese
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
};

// return the len of rule
#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
// compile regex in rules and sort the rules in re. If go wrong then panic
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

//structure for record type and substr of the recognized token. 
typedef struct token {
  int type;
  char str[32];
} Token;

// record the tokens(type, substr) that has been recognized. 
static Token tokens[32] __attribute__((used)) = {};
//indecate the number of token that has been recognized. 
static int nr_token __attribute__((used))  = 0;

//recognized tokens in 'e' and sort in array 'tokens'. 
static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  //store the recognized tokens' number. 
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
			case TK_NUM: tokens[nr_token].type=TK_NUM;
						 strcpy(tokens[nr_token].str,substr_start);
						 tokens[nr_token].str[substr_len]=0;break;
			case '/': tokens[nr_token].type='/';
					  strcpy(tokens[nr_token].str,substr_start);
					  tokens[nr_token].str[substr_len]=0;break;
			case '*': tokens[nr_token].type='*';
					  strcpy(tokens[nr_token].str,substr_start);
					  tokens[nr_token].str[substr_len]=0;break;
			case '-': tokens[nr_token].type='-';
					  strcpy(tokens[nr_token].str,substr_start);
					  tokens[nr_token].str[substr_len]=0;break;
			case ')': tokens[nr_token].type=')';
					  strcpy(tokens[nr_token].str,substr_start);
					  tokens[nr_token].str[substr_len]=0;break;
			case '(': tokens[nr_token].type='(';
					  strcpy(tokens[nr_token].str,substr_start);
					  tokens[nr_token].str[substr_len]=0;break;
			case '+': tokens[nr_token].type='+';
					  strcpy(tokens[nr_token].str,substr_start);
					  tokens[nr_token].str[substr_len]=0;break;
			case TK_EQ: tokens[nr_token].type=TK_EQ;
						strcpy(tokens[nr_token].str,substr_start);
					tokens[nr_token].str[substr_len]=0;break;
        }
		++nr_token;

        break;
      }
    }

	//if not match
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


static bool check_parentheses(Token *p, Token *q){
	if(p->str[0]=='(' && q->str[0]==')'){
		int b=0;
		bool parentheses_not_pair=false;
		for(Token *i=p+1;i!=q;++i){
			printf("check_par i: %s\n",i->str);
			switch(i->str[0]){
				case '(':++b;
						 break;
				case ')': if(b==0){assert(parentheses_not_pair); return false;}
						  else{--b;}
						  break;
			}
		}
		if(b!=0){
			assert(parentheses_not_pair);
			return false;
		}
		else{
			return true;
		}
	}
	else{
		return false;
	}
}


static Token *find_operator(Token *p, Token *q){
	Token *operator[q-p+1]__attribute__((unused));
	Token *a=p;
	Token *b=q;
	unsigned int len=0;
	while(a-1!=q){
		if(a->type!=TK_NUM){
			operator[len]=a;
			++len;
		}
		++a;
	}
	operator[len]=NULL;

	if(check_parentheses(a,b)){++a;--b;}
	int c=0;
	unsigned int count=0;
	Token *d=NULL;
	printf("find_ count:\n");
	for(;b!=a-1;--b){
		printf("%u, %s\n", count, b->str);
		++count;
		switch(b->str[0]){
			case '(': --c;
					  break;
			case ')': ++c;
					  break;
		   	case '+': case '-'://not in (), immediately return + or -
					  if(c==0){return b;}
			case '*': case '/':
					  if(c==0 && d==NULL){d=b;}//not in () and is the first * or /
		}
	}
	if(d!=NULL){return d;}//no + or - outside (), return the first * or /
	else{bool find_first_operator_error=false; assert(find_first_operator_error); return NULL; }
}


__attribute__((unused))static int eval(Token *p, Token *q, bool *success){
	if(p>q){//bad expression
		*success=false;
		return -1;
	}
	else if(p==q){//single token
		if(p->type==TK_NUM){//is a number
		int a;
	   	sscanf(p->str, "%d", &a);
		return a;
		}
		else{//not a number
			*success=false;
			return -1;
		}
	}
	else if(check_parentheses(p,q)){//enclosed with parentheses
		return eval(p+1, q-1, success);
	}
//	else{//

//	}
	return 0;
}



//main function
word_t expr(char *e, bool *success) {
	//check whether 'e' match tokens in re(specific token types). 
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
	
  /* to test the str tokens
  for(int i=0;i<nr_token;++i){
	  printf("%s ",tokens[i].str);
  }
  printf("\n");
  */
  Token *p=&tokens[0];
  Token *q=&tokens[nr_token-1];
  //Token **operators;
  //operators=extract_operator(p, q);
  printf("if paren: %d\n", check_parentheses(p,q));
  printf("find_: %s\n",find_operator(p,q)->str);
  /*
  }
  */
  assert(0);

  return 0;
}
