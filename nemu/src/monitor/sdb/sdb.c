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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
// hlz add
#include<memory/paddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
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
  nemu_state.state=NEMU_QUIT;
  return -1;
}


static int cmd_x(char *args){
	char *arg=strtok(NULL, " ");
	if(arg==NULL){
		printf("Please enter N to specify the bytes to read. \n");
		return 0;
	}
	int n;
	sscanf(arg,"%d",&n);
	arg=strtok(NULL, " ");
	if(arg==NULL){
		printf("Please enter EXPR to specify the memory address to read. \n");
		return 0;
	}
	else{
		unsigned int my_indent=16;
		paddr_t add;
		sscanf(arg, "%x", &add);
		word_t res;
		printf("%-*s%-*s\n",my_indent,"Address",my_indent,"Content");
		printf("\n");
		for(int i=0;i<n;++i){
			res=paddr_read(add,4);
			printf("%#-*x0x%8x\n",my_indent, add, res);
			add+=4;
		}
		return 0;
	}
}


static int cmd_si(char *args){
	// the steps of instructions to execute
	unsigned int step=0;
	char* arg;
	arg=strtok(NULL," ");
	// if no arguement
	if(arg==NULL){
		step=1;
	}
	//if arguement is negtive, then convert to unsigned int
	else if(arg[0]=='-'){
		int res=0;
		for(int i=1;i<strlen(arg);++i){
			res=res*10+(int)arg[i]-(int)'0';
		}
		step=(unsigned int)(-1*res);
	}
	//if arguement is postive
	else{
		for(int i=0;i<strlen(arg);++i){
		  step=step*10+(unsigned int)arg[i]-(unsigned int)'0';
		}	  
	}
	// later found that sscanf(arg, "%u", &step) is much better
	cpu_exec(step);
	return 0;
}

static int cmd_info(char *args);

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute next program line (after stopping); step ? function", cmd_si},
  {"info", "Print the infomation of SUBCMD", cmd_info },
  {"x", "evaluate the EXPR, and print the N 4 btyes from the address of EXPR", cmd_x },
  //{ "p", "evaluate and print the EXPR", cmd_p },
  //{ "w", "halt when the EXPR changes", cmd_w },
  //{ "d", "delete the N watchpoint", cmd_d }

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)


static int  cmd_info(char *args){
	char *arg=strtok(NULL," ");
	if(arg==NULL){
		for (int i = 0; i < NR_CMD;++i){
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else if(arg[0]=='r'){
		isa_reg_display();		
	}
	else if(arg[0]=='w'){
		printf("still unrealized\n");
	}
	return 0;
}


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

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  {//hlzt
	TODO();
  char* e=readline("expr: ");
  bool *su;
  su=(bool *)malloc(sizeof(bool));
  *su=true;
  printf("%d\n", expr(e,su));
  printf("su: %d\n", *su);
	}
  for (char *str; (str = rl_gets()) != NULL; ) {
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

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
	printf("%s\n",cmd);
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();
  {
  //hlzt
  printf("init_regex() succeeds. \n");
  }

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
