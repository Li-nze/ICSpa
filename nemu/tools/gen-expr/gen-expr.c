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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static long len=0;
static int max_len=65536;
static char buf[65536] = {'\0'};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static unsigned int choose(unsigned int a){
	return (rand()%a);
}

static void gen_operator(){
	++len;
	switch(choose(4)){
		case 0: strncat(buf, "+", max_len-len-1); ;break;
		case 1: strncat(buf, "-", max_len-len-1); ;break;
		case 2: strncat(buf, "*", max_len-len-1); ;break;
		default: strncat(buf, "/", max_len-len-1); ;break;
	}
	return;
}

static void gen_num(){
	unsigned int a=rand();
	char s[15];
	sprintf(s, "%u", a);
	strncat(buf, s, max_len-len-1); ;
	len+=strlen(s);
	return;
}

static void gen(char *a){
	++len;
	strncat(buf, a, max_len-len-1); ;
	return;
}

static void gen_space(){
	unsigned int a=choose(5);
	for(unsigned int i=0;i<a; ++i){
		gen(" ");
	}
	return;
}

static void gen_rand_expr(unsigned * a) {
	if(len>=max_len-1){*a=0; return;}
	switch(choose(3)){
		case 0: gen_space();
				gen_num();
				gen_space();break;
		case 1:  gen_space();
				 gen("(");
				 gen_space();
				 gen_rand_expr(a);
				 gen_space();
			   	 gen(")");
				 gen_space();  break;
		default: gen_space();
				 gen_rand_expr(a);
				 gen_space();
			   	 gen_operator();
				 gen_space();
			   	 gen_rand_expr(a);
				 gen_space();  break;
	}
	return;
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0]=0;
	len=0;
	unsigned *a;
	a=(unsigned *)malloc(sizeof(unsigned));
	*a=1;
	//printf("i: %d\n", i);
    gen_rand_expr(a);
	if(*a==0){
		if(i!=0){--i;}
		continue;
	}
	
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

	//printf("1\n");
    int ret = system("gcc -w /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

	//printf("2\n");
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);
	//printf("3\n");

    int result;
    //fscanf(fp, "%d", &result);
	fscanf(fp, "%u", &result);
    pclose(fp);
	//printf("4\n");
	//printf("i: %d\n", i);
    //printf("res: %u \n buf%s\n", result, buf);
	//printf("\n");
	printf("%u %s\n", result, buf);
  }
  return 0;
}
