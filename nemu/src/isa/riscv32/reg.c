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
#include "local-include/reg.h"

// register name, register is cpu.pc and cpu.gpr[] in cpu-exec.c
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

// print all the registers
void isa_reg_display() {
	unsigned int my_indent=15;
	unsigned int len=(unsigned int)(sizeof(regs)/sizeof(regs[0]));
	printf("%-*s%#-*x%-*d\n", my_indent, "PC",  my_indent, cpu.pc, my_indent,  cpu.pc);
	for(int i=0;i<len;++i){
		printf("%-*s%#-*x%-*d\n", my_indent, regs[i], my_indent, cpu.gpr[i], my_indent, cpu.gpr[i]);
	}
}

word_t isa_reg_str2val(const char *s, bool *success) {
	//printf("%s\n", s);
	printf("$0?: %d\n", strcmp(s, "$0"));
	if(strcmp(s, "$pc")==0){
		*success=true;
		return cpu.pc;
	}
	const char *str=strcmp(s, "$0")==0?s:(s+1);
	unsigned len=(unsigned)(sizeof(regs)/sizeof(regs[0]));
	for(unsigned i=0; i<len; ++i){
		if(strcmp(str, regs[i])==0){
			*success=true;
			return cpu.gpr[i];
		}
	}
	*success=false;
	return 0;
}
