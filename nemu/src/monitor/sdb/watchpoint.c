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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
// head: watchpoint on use. free: watchpoint available.  
static WP *head = NULL, *free_ = NULL, *tail=NULL;
// hlz Add
static int wpassert=1;
//static int wpassert=0;


WP *new_wp(){
	if(free_!=NULL){
		WP *a=free_;
		free_=free_->next;
		a->next=NULL;
		if(tail!=NULL){
			tail->next=a;
		}
		else{
			head=a;
		}
		tail=a;
		return a;
	}
	else{
		int no_watchpoint_available=0;
		wpassert?assert(no_watchpoint_available):printf("No free watchpoint available!\n");
		return NULL;
	}
}


void free_wp(WP *wp){
	WP *a=head;
	if(a==NULL){
		int head_empty=0;
		wpassert?assert(head_empty):printf("watchpoint head is empty\n");
	}
	else{
		if(a==wp){
			if(head==tail){
				head=NULL;tail=NULL;
				printf("now no watchpoint is on use.\n");
			}
			else{
				head=a->next;
			}
			wp->next=free_;
			free_=wp;
			return;
		}
		while(a->next!=NULL && a->next!=wp){a=a->next;}
		if(a->next==NULL){
			int no_watchpoint_in_head=0;
			wpassert?assert(no_watchpoint_in_head):printf("NO: %d watchpoint not in head.\n", wp->NO);
		}
		else{
			a->next=wp->next;
			wp->next=free_;
			free_=wp;
		}
	}
	return;
}


void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

