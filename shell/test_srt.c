#include <test_lab1.h>
#include <xinu.h> 
#include "shprototypes.h"
void test_longRunningProc(void){
	sleepms(500); //Let console display output be nice

	kprintf("================== TEST LONGRUNNING PROC =============\n");
	int i, j, k;
	uint32 len = 1024;
	uint32* p = (uint32*) getmem(len*sizeof(uint32));
	struct procent* prptr = &proctab[currpid]; 
	for(i = 0; i < 100; i++){
		for(j = 0; j < 5000; j++){
			//Write into memory junk
			for(k = 0; k < len; k++){
				p[k] = 42*j;
			}	
		}
		kprintf("%d B: %d\n", i, prptr->prev_burst);
	}
	kprintf("LRP Complete\n");
	


}


int test_chgprio(void){

	sleep(1);

	kprintf("================== TEST CHGPRIO ======================\n");
	//Start 2 printer processes 
	pid32 p1 = create(printer, 1024, TSSCHED,  10, "proc1", 1, 10000);
	pid32 p2 = create(printer, 1024, TSSCHED,  10, "proc2", 1, 10000);
	pid32 p3 = create(printer, 1024,  SRTIME,  65, "proc3", 1, 100);
	pid32 p4 = create(printer, 1024,  SRTIME,  65, "proc4", 1, 100);
	kprintf("Starting both processes...\n");
	resume(p1); resume(p2); resume(p3); resume(p4);
	sleep(2);

	kprintf("TSS ONLY\n");
	//chgprio for TSS and make sure SRT is blocked
	chgprio(TSSCHED, 100);
	sleep(2);

	kprintf("SRT ONLY\n");
	chgprio(SRTIME, 200);
	sleepms(300);

	kprintf("BOTH\n");
	chgprio(SRTIME, 100);
	sleep(2);
	kill(p1); kill(p3);
	kill(p2); kill(p4);
	if( chgprio(-1, 100) != SYSERR) kprintf("ERR");
	kprintf("TEST COMPLETE");
	return OK;
}

void printer(int loop){
	struct procent* prptr = &proctab[getpid()];
	char* group[] = {"SRT", "TSS"};
	int i, j, k;
	uint32 len = 1024;
	uint32* p = (uint32*) getmem(len*sizeof(uint32));
	
	for(i = 0; i < 10000; i++){
		for(j = 0; j < loop; j++){
			//Write into memory junk
			for(k = 0; k < len; k++){
				p[k] = 42*j;
			}	
		}
		kprintf("%3s %s\n", group[(int)prptr->sched_alg], prptr->prname);
	}
}

int test_lab1(void){
	
	sleepms(3000);
	kprintf("================== TEST LAB1 ======================\n");
	
	//Create 3 processes with SRT
	resume(create(io_bound, 1024, TSSCHED, 50, "proc1", 0));
	resume(create(io_bound, 1024, SRTIME,  50, "proc2", 0));
	resume(create(io_bound, 1024, TSSCHED, 50, "proc3", 0));
	
	resume(create(cpu_bound,  1024, SRTIME,  50, "proc4", 0));
	resume(create(cpu_bound,  1024, TSSCHED, 50, "proc5", 0));
	resume(create(cpu_bound,  1024, SRTIME,  50, "proc6", 0));
	//Now periodically print the process list
/*	while(1){
		printProcTab(1);
		sleepms(3000);
	}*/
	return OK;
}

void cpu_bound(void){
	int i, j, k;
	uint32 len = 1024;
	uint32* p = (uint32*) getmem(len*sizeof(uint32));
	
	for(i = 0; i < 10; i++){
		for(j = 0; j < 5000; j++){
			//Write into memory junk
			for(k = 0; k < len; k++){
				p[k] = 42*j;
			}	
		}
		kprintf("CPU %2d:%2d:%4d\n", getpid(), i, preempt);
	}
	kprintf("CPU %2d DONE\n", getpid());
}

void io_bound(void){
	int i, j;
	for(i = 0; i < 10; i++){
		for(j = 0; j < 2; j++){
			sleepms(500);
		}
		kprintf("IO %2d:%2d\n", getpid(), i);
	}
	kprintf("IO %2d DONE\n", getpid());
}

void test_uid(void){
	//This process is from main... should have root. 
	kprintf("=================== TEST UID ====================\n");
	kprintf("test_uid->uid = %d\n", proctab[currpid].uid);
	
	//Create a process, check that it has uid == ROOT
	pid32 p1 = create(userProc1, 1024, SRTIME, 20, "p1", 0);
	if(p1 == SYSERR) kprintf("err0 create\n");
	if(proctab[p1].uid != proctab[currpid].uid) kprintf("err1 create\n");
	//Resume the process... check return message
	if(SYSERR == resume(p1)) kprintf("err2 resume\n");

	//Wait
	sleepms(1000);

	//Suspend the process... check return message.
	kprintf("before suspend \n"); printProcTab(1);
	if(SYSERR == suspend(p1)) kprintf("err3 suspend\n"); 
	kprintf("\nafter suspend:\n"); printProcTab(1);
 	//Kill the process... check that it is gone. 
	if(SYSERR == kill(p1)) kprintf("err4 kill\n");

	kprintf("p1 should not appear in following ps\n");
	printProcTab(1);

	//Create a process with uid = 2
	pid32 p2 = create(userProc2, 1024, TSSCHED, 20, "p2", 0);
	if(SYSERR == p2) kprintf("err5 create\n");
	if(SYSERR == resume(p2)) kprintf("err6 resume\n");
	
	//Wait for output confirming that it changed its user id
	sleepms(1000);
	kprintf("should have seen new uid\n");

	printProcTab(1);
	//Suspend...
	if(SYSERR == suspend(p2)) kprintf("err7 suspend\n");
	//Kill
	if(SYSERR == kill(p2)) kprintf("err8 kill\n");

	//Create a process with uid = 3
	pid32 p3 = create(userProc3, 1024, SRTIME, 20, "p3", 0);
	if(SYSERR == resume(p3)) kprintf("err14 resume\n");
	
	sleepms(1000);
	//Create process and sick it on p3
	pid32 p4 = create(userProc4, 1024, TSSCHED, 20, "p4", 1, p3);
	resume(p4);

	sleepms(1000);
	printProcTab(1);
	kprintf("=======================================\n");
}

void userProc1(void){
	kprintf("Inside userproc1... \n");
	int count = 0;
	while(1){
		count ++;
	}
}

void userProc2(void){
	kprintf("Inside userproc2... changing UID");
	if (SYSERR == setuid(2)) kprintf("err9 setsuid\n");
	kprintf("userproc2: My UID is %d\n", proctab[currpid].uid);
	int count = 0;
	while(1){
		count++;
	}

}

void userProc3(void){
	kprintf("Inside userproc3... changing UID");
	if(SYSERR == setuid(3)) kprintf("err10 setuid\n");
	kprintf("userproc3: my userID is %d\n", proctab[currpid].uid);
	if(SYSERR != setuid(45)) kprintf("err12 setuid\n");
	
	suspend(currpid);
	
}

void userProc4(pid32 p3){
	kprintf("Inside userproc4... trying to mess with p3");
	if (SYSERR == setuid(4)) kprintf("err11setsuid\n");
	kprintf("userproc4: My UID is %d\n", proctab[currpid].uid);
	
	if(SYSERR != resume(p3)) kprintf("err15 resume\n");
	if(SYSERR != kill(p3)) kprintf("err16 kill\n");
	resume(create(userProc5, 1024, SRTIME, 20, "p5", 0));
}

void userProc5(void){
	kprintf("userProc5...\n");
	if(4 != proctab[currpid].uid) kprintf("err17 setuid\n");
}

