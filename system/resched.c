/* resched.c - resched, resched_cntl */


#include <xinu.h>
#include <table.h>

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
	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */
	ptold = &proctab[currpid];

	/* Call aging schedule to determine which group to schedule */
	int16 sched = agingSched();

	/* Insert current process into the correct ready list, if necessary */
	insertCurrProc();

	pid32 newPID = 0;
	if(sched == SRTIME){
		//Call the SRT scheduler and then schedule the process it returns
		newPID = dequeue(readylistSRT); 
		preempt = QUANTUM;
	}
	else{
		//Call the TSS scheduler and then schedule the process is returns
		newPID = dequeue(readylistTSS);
		preempt = proctab[newPID].nx_quantum;
	}
	//If we are rescheduling the same process, do nothing.
	if(newPID == currpid){
		ptold->prstate = PR_CURR; /* Reset state */
		return;
	}
	/* Force context switch to the new process */
	currpid = newPID;
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
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
int16 agingSched(void){
	//Traverse the ready queue and count number of processes in each group
	uint32 sumSRT = 0;
	uint32 sumTSS = 0;
	qid16 curr = firstid(readylistTSS);
	
	//Loop until next element is the tail 
	while(curr != queuetail(readylistTSS)){
		//Note that the index into queuetab IS the PID
		if(curr != NULLPROC && curr != currpid){
			sumTSS++;
		}
		curr = queuetab[curr].qnext;
	}
	
	curr = firstid(readylistSRT);
	while(curr != queuetail(readylistSRT)){
		if(curr != NULLPROC && curr != currpid){
			sumSRT++;
		}
		curr = queuetab[curr].qnext;
	}	

	//Set process group priority of current process to its default value
	if(proctab[currpid].sched_alg == SRTIME){
		grp_pri.SRT_pri = grp_pri.SRT_init_pri;
	}
	else{
		grp_pri.TSS_pri = grp_pri.TSS_init_pri;
	}
	//If both counts are zero, but the null proess is in one of the ready lists. 
	if(sumSRT == 0 && sumTSS == 0){
		//Want to schedule the null process
		return proctab[NULLPROC].sched_alg;
	}

	//If any of the groups is empty, then schedule the other group. 
	if(sumSRT == 0){
		return TSSCHED;
	}
	if(sumTSS == 0) {
		return SRTIME;
	}

	//Increment process group priorities by the number of processes waiting
	grp_pri.SRT_pri += sumSRT;
	grp_pri.TSS_pri += sumTSS;
	if(grp_pri.SRT_pri >= grp_pri.TSS_pri){
		return SRTIME;
	}
	else{
		return TSSCHED;
	}
} 

void insertCurrProc(void){
	struct procent *ptold = &proctab[currpid];
	/* Only insert the process if it should remain eligible */
	/* Decide which ready list to insert it into */
	if(ptold->sched_alg == SRTIME){
		//Compute the burst time of the process
		computeBurst(ptold);
		if(ptold->prstate != PR_CURR){
			return;
		}
		//Insert into SRT ready list
		//Note use of insertA: insert in ascending order
		insertA(currpid, readylistSRT, ptold->EB);
	}
	else{ //TSSCHED
		//Get the new priority of this process
		assignTimes(ptold);
		if(ptold->prstate != PR_CURR){
			return;
		}
		//Insert into the TSS ready list
		insert(currpid, readylistTSS, ptold->prprio);
	}

	ptold->prstate = PR_READY;
}



void assignTimes(struct procent* prptr){
	//Categorize current process as IO bound or CPU bound by process state
	if(prptr->prstate != PR_CURR){
		//Yielded CPU 
		prptr->tss_type = IO_BOUND;
	}
	else{
		//Time slice expired or interupted by higher priority process
		prptr->tss_type = CPU_BOUND; 
	}
	//Index into tsd_tab is the processes old priority
	struct tsd_ent* tsdPtr = &tsd_tab[prptr->prprio];
	//Assign quantum and new priority based on current priority
	if(prptr->tss_type == CPU_BOUND){	
		prptr->prprio = tsdPtr->ts_tqexp;
	}
	else{ //(prptr->tss_type == IO_BOUND)
		prptr->prprio = tsdPtr->ts_slpret;
	}
	prptr->nx_quantum = tsdPtr->ts_quantum;
}

//Computes the burst time of process and then predicts the next burst
void computeBurst(struct procent* prptr){
	//If the process has not yet yielded the CPU, then accumulate burst
	if(prptr->accumFlag == 1){
		prptr->prev_burst += (int32)(1000*(QUANTUM - preempt));
	}
	else{
		prptr->prev_burst  = (int32)(1000*(QUANTUM - preempt));
	}

	//Set flag to begin accumulating if CPU has not been yielded
	if(preempt  ==  0 && prptr->prstate == PR_CURR){
		prptr->accumFlag = 1;
	}
	else{
		prptr->accumFlag = 0;
	}
	
	prptr->EB = prptr->prev_burst*7/10 + prptr->prev_EB*3/10;
	//prptr->EB = ALPHA*prptr->prev_burst + (1-ALPHA)*prptr->prev_EB;
		
	//Only reassign prev_EB if the process has yielded the CPU
	if(prptr->accumFlag == 0){
		prptr->prev_EB = prptr->EB;
	}
}

void printReadyList(qid16 list){
	qid16 curr = firstid(list);
	while(curr != queuetail(list)){
		XDEBUG_KPRINTF("pid: %d ", curr);
		XDEBUG_KPRINTF(" sched_alg: %d ", proctab[curr].sched_alg);
		XDEBUG_KPRINTF(" state: %d ", proctab[curr].prstate);
		XDEBUG_KPRINTF(" key: %d ", queuetab[curr].qkey);
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
		char *group[] = {"SRTIME ", "TSSCHED"};
		kprintf("%3s %-16s %5s %4s %4s %10s %-10s %10s %5s %4s %3s %9s\n",
			   "Pid", "Name", "State", "Prio", "Ppid", "Stack Base",
			   "Stack Ptr", "Stack Size", "burst", "expB", "uid", "sched_alg");

		kprintf("%3s %-16s %5s %4s %4s %10s %-10s %10s %5s %4s %3s %9s\n",
			   "---", "----------------", "-----", "----", "----",
			   "----------", "----------", "----------", "-----", "----", "---", "---------");
		for (i = 0; i < NPROC; i++) {
			prptr = &proctab[i];
			if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
				continue;
			}
			kprintf("%3d %-16s %s %4d %4d 0x%08X 0x%08X %8d %5d %4d %3d %9s\n",
				i, prptr->prname, pstate[(int)prptr->prstate],
				prptr->prprio, prptr->prparent, prptr->prstkbase,
				prptr->prstkptr, prptr->prstklen, prptr->prev_burst,
				prptr->prev_EB, prptr->uid, group[(int)prptr->sched_alg]);
		}
	}
}










