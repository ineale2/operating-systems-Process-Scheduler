#include <test_lab1.h>
#include <xinu.h> 

int test_lab1(void){
	
	sleepms(3000);
	kprintf("==================TEST SRT======================\n");
	
	//Create 3 processes with SRT
	resume(create(cpu_bound, 1024, SRTIME, 20, "proc1", 0));
	//resume(create(cpu_bound, 1024, SRTIME, 20, "proc2", 0));
	//resume(create(cpu_bound, 1024, SRTIME, 20, "proc3", 0));
	
	//Now periodically print the process list
/*	while(1){
		printProcTab(1);
		sleepms(3000);
	}*/
	return OK;
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
	if(SYSERR == suspend(p1)) kprintf("err3 suspend\n"); 
 	//Kill the process... check that it is gone. 
	if(SYSERR == kill(p1)) kprintf("err4 kill\n");

	kprintf("p1 should not appear in following ps\n");
	printProcTab(1);

	//Create a process with uid = 2
	pid32 p2 = create(userProc2, 1024, SRTIME, 20, "p2", 0);
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
	pid32 p4 = create(userProc4, 1024, SRTIME, 20, "p4", 1, p3);
	resume(p4);

	sleepms(1000);
	printProcTab(1);
	kprintf("=======================================\n");
}

void userProc1(void){
	kprintf("Inside userproc1... \n");
	while(1){
		printProcTab(1);
		sleep(3);	
	}
}

void userProc2(void){
	kprintf("Inside userproc2... changing UID");
	if (SYSERR == setuid(2)) kprintf("err9 setsuid\n");
	kprintf("userproc2: My UID is %d\n", proctab[currpid].uid);
	while(1){
		sleepms(0);
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

void cpu_bound(void){
	kprintf("CPU BOUND started: pid %d", getpid());
	int i, j, k;
	uint32 len = 1024;
	uint32* p = (uint32*) getmem(len*sizeof(uint32));
	
	for(i = 0; i < 10; i++){
		for(j = 0; j < 1000; j++){
			//Write into memory junk
			for(k = 0; k < len; k++){
				p[k] = 42;
			}	
		}
		kprintf("pid: %d :: loop = %d :: prempt = %d\n", getpid(), i, preempt);
	}
	kprintf("pid %d finished!\n", getpid());
}
