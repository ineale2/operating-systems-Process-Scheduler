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
