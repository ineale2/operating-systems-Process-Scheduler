/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;
struct  groupPriority grp_pri;
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */
	XDEBUG_KPRINTF("============================================\n");
	printProcTab(XDEBUG);
	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */
	ptold = &proctab[currpid];

	
	/* Call aging schedule to determine which group to schedule */
	XDEBUG_KPRINTF("Calling agingsSched()... ");
	bool8 schedSRT = agingSched();
	XDEBUG_KPRINTF("schedSRT: %d\n",schedSRT);
	pid32 newPID = 0;
	if(schedSRT){
		//Call the SRT scheduler and then schedule the process it returns
		newPID = schedulerSRT(); 	
	}
	else{
		//Call the TSS scheduler and then schedule the process is returns
		newPID = schedulerTSS();
	}
	//If we are rescheduling the same process, do nothing.
	if(newPID == currpid){
		XDEBUG_KPRINTF("Same process :: newPID: %d currpid %d\n", newPID, currpid);
		return;
	}
	XDEBUG_KPRINTF("Scheduling %d\n", newPID);
	XDEBUG_KPRINTF("\n\n");

	/* Compute the burst time for the currently running process */
	computeBurst(currpid);	

	//Handle the implicit argument of the current processes state
	//If this process was preempted, then it should stay ready, otherwise do not modify its state
	if(ptold->prstate == PR_CURR){
		//If this process was preempted, then it should stay eligible. 
		ptold->prstate = PR_READY;
		//reinsert into the ready list
		enqueue(currpid, readylist);
	}
	//Remove the to be scheduled process from the ready list
	
	//Loop until next element is the tail 
	XDEBUG_KPRINTF("readylist: before getitem()\n");
	printReadyList();

	getitem(newPID);

	XDEBUG_KPRINTF("\n\n");
	XDEBUG_KPRINTF("readylist: after  getitem()\n");
	printReadyList();
	XDEBUG_KPRINTF("============================================\n");
	/* Force context switch to the new process */
	currpid = newPID;
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}


//agingSched implements the agingScheduler algorithim and selects one of the two groups (SRT or TSS) to schedule
//returns 1 to schedule SRT, 0 to schedule TSS
bool8 agingSched(void){
	//Traverse the ready queue and count number of processes in each group
	uint32 sumSRT = 0;
	uint32 sumTSS = 0;
	XDEBUG_KPRINTF(" in agingSched()\n");
	qid16 curr = firstid(readylist);
	
	//Loop until next element is the tail 
	while(curr != queuetail(readylist)){
		//Note that the index into queuetab IS the PID
		//Count number of processes in each group, except the current process
		XDEBUG_KPRINTF("curr: %d ", curr);
		XDEBUG_KPRINTF(" sched_alg: %d ", proctab[curr].sched_alg);
		XDEBUG_KPRINTF(" state: %d ", proctab[curr].prstate);
		XDEBUG_KPRINTF(" name %s\n",  proctab[curr].prname);  
		if(proctab[curr].sched_alg == SRTIME  && curr != NULLPROC && curr != currpid){
			sumSRT++;
		}
		if(proctab[curr].sched_alg == TSSCHED && curr != NULLPROC && curr != currpid){
			sumTSS++;
		}
		curr = queuetab[curr].qnext;
	}
	XDEBUG_KPRINTF("Counted: %d SRT and %d TSS\n",sumSRT, sumTSS);	
	//Set process group priority of current process to its default value
	if(proctab[currpid].sched_alg == SRTIME){
		grp_pri.SRT_pri = grp_pri.SRT_init_pri;
	}
	else{
		grp_pri.TSS_pri = grp_pri.TSS_init_pri;
	}

	//If any of the groups is empty, then schedule the other group. 
	if(sumSRT == 0){
		XDEBUG_KPRINTF("Early return, sumSRT = 0\n");
		return 0;
	}
	if(sumTSS == 0) {
		XDEBUG_KPRINTF("Early return, sumTSS = 0\n");
		return 1;
	}

	//Increment process group priorities by the number of processes waiting
	grp_pri.SRT_pri += sumSRT;
	grp_pri.TSS_pri += sumTSS;
	XDEBUG_KPRINTF("SRT_pri: %d TSS_pri %d\n", grp_pri.SRT_pri, grp_pri.TSS_pri);
	return (grp_pri.SRT_pri >= grp_pri.TSS_pri);
} 

//Returns the pid of the next process to schedule under the SRT algorithim
pid32 schedulerSRT(void){
//	XDEBUG_KPRINTF("In schedulerSRT... ");
	qid16  curr = firstid(readylist);
	uint32 EB = 0; 						//expected burst of current process 
	uint32 minEB = UINT_MAX; 			//minimum expected burst time 
	pid32  newPID = 0; 					//pID to schedule
	//Walk through ready queue, checking if SRT and then comparing to min burst time
	//This must be round robin in the event of a tie to prevent starvation
	//To achieve round robin: always pick the first element with the min EB
	//resched() will reinsert processes at the back of the ready queue to ensure RR
	while(curr != queuetail(readylist)){
		EB = ALPHA*proctab[curr].prev_burst + (1-ALPHA)*proctab[curr].prev_exp_burst;
		XDEBUG_KPRINTF("curr: %d :: group %d :: EB %d\n", curr, proctab[curr].sched_alg, EB);
		if(proctab[curr].sched_alg == SRTIME && EB < minEB){
			minEB = EB;
			newPID = curr;
		}
		curr = queuetab[curr].qnext; 
	}
	//Assign E(n+1) for process that was picked
	proctab[newPID].prev_exp_burst = minEB;
	XDEBUG_KPRINTF("SRT picks pID %d\n", newPID);
	return newPID;
} 

//Returns the pid of the next process to schedule under the TSS algorithim
pid32 schedulerTSS(void){
//	XDEBUG_KPRINTF("In schedulerTSS... ");
	pid32 newPID = 0; 							//process to schedule
	struct procent* prptr = &proctab[currpid]; 	//pointer to process table entry of current process

	//Categorize current process as IO bound or CPU bound by looking at time remaining in preempt
	if(preempt  == 0){
		prptr->tss_type = CPU_BOUND;
	}
	else{
		prptr->tss_type = IO_BOUND; 
	}
	//Assign priority and quantum to process
	assignTimes(prptr);

	//Reinsert into ready queue if necessary 
	
	//Select next process from ready queue based on priority


	return newPID;
} 

void assignTimes(struct procent* prptr){
	
	prptr->prprio =  		
	prptr->next_quantum = 
}

//Computes and assigns the burst time of the process specified in the argument
void computeBurst(pid32 pid){
	XDEBUG_KPRINTF("computeBurst:: preempt = %d, prstate = %d, quantum = %d, accumFlg = %d, burst = %d\n",
		preempt, proctab[pid].prstate, QUANTUM, proctab[pid].accumFlag, proctab[pid].prev_burst);
	//Set the burst to the used time
	//If the process has not yet yielded the CPU, then accumulate burst
	if(proctab[pid].accumFlag == 1){
		proctab[pid].prev_burst += QUANTUM - preempt;
	}
	else{
		proctab[pid].prev_burst  = QUANTUM - preempt;
	}

	//Set flag to begin accumulating if CPU has not been yielded
	if(preempt > 0 && proctab[pid].prstate == PR_CURR){
		proctab[pid].accumFlag = 1;
	}
	else{
		proctab[pid].accumFlag = 0;
	}
	XDEBUG_KPRINTF("computeBurst:: preempt = %d, prstate = %d, quantum = %d, accumFlg = %d, burst = %d\n",
		preempt, proctab[pid].prstate, QUANTUM, proctab[pid].accumFlag, proctab[pid].prev_burst);
}

void printReadyList(void){
	qid16 curr = firstid(readylist);
	while(curr != queuetail(readylist)){
		XDEBUG_KPRINTF("curr: %d ", curr);
		XDEBUG_KPRINTF(" sched_alg: %d ", proctab[curr].sched_alg);
		XDEBUG_KPRINTF(" state: %d ", proctab[curr].prstate);
		XDEBUG_KPRINTF(" name %s\n",  proctab[curr].prname);  
		curr = queuetab[curr].qnext;
	}
}

void printProcTab(int print){
	if(print){
		int i;
		struct procent* prptr;
		char *pstate[]	= {		/* names for process states	*/
			"free ", "curr ", "ready", "recv ", "sleep", "susp ",
				"wait ", "rtime"};
		kprintf("%3s %-16s %5s %4s %4s %10s %-10s %10s %5s %4s %3s\n",
			   "Pid", "Name", "State", "Prio", "Ppid", "Stack Base",
			   "Stack Ptr", "Stack Size", "burst", "expB", "uid");

		kprintf("%3s %-16s %5s %4s %4s %10s %-10s %10s %5s %4s %3s\n",
			   "---", "----------------", "-----", "----", "----",
			   "----------", "----------", "----------", "-----", "----", "---");
		for (i = 0; i < NPROC; i++) {
			prptr = &proctab[i];
			if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
				continue;
			}
			kprintf("%3d %-16s %s %4d %4d 0x%08X 0x%08X %8d %5d %4d %3d\n",
				i, prptr->prname, pstate[(int)prptr->prstate],
				prptr->prprio, prptr->prparent, prptr->prstkbase,
				prptr->prstkptr, prptr->prstklen, prptr->prev_burst,
				prptr->prev_exp_burst, prptr->uid);
		}
	}
}










