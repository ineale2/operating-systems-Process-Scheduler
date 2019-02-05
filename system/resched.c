/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

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

	/* Compute the busrt time for the currently running process */
	computeBurst(ptold);	
	
	/* Call aging schedule to determine which group to schedule */
	bool8 schedSRT = agingSched();
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
		return;
	}

	//Handle the implicit argument of the current processes state
	//If this process was preempted, then it should stay ready, otherwise do not modify its state
	if(ptold->prstate == PR_CURR){
		//If this process was preempted, then it should stay eligible. 
		ptold->prstate = PR_READY;
		//reinsert into the ready list
		insert(currpid, readylist, ptold->prprio);
	}

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

	qid16 curr = firstid(readylist);
	
	//Loop until next element is the tail 
	while(queuetab[curr] != queuetail(readylist)){
		//Note that the index into queuetab IS the PID
		//Count number of processes in each group, except the current process
		if(proctab[curr].sched_alg == SRTIME){
			sumSRT++;
		}
		else{
			sumTSS++;
		}
		curr = queuetab[curr].qnext;
	}
	
	//Set process group priority of current process to its default value
	if(proctab[currpid].sched_alg == SRTIME){
		grp_pri.SRT_pri = SRT_init_pri;
	}
	else{
		grp_pri.TSS_pri = TSS_init_pri;
	}
	//Increment process group priorities by the number of processes waiting
	grp_pri.SRT_pri += sumSRT;
	grp_pri.TSS_pri += sumTSS;

	return (grp_pri.SRT_pri >= grp_pri.TSS_pri)
} 

//Returns the pid of the next process to schedule under the SRT algorithim
pid32 schedulerSRT(void){
	//Walk through ready queue, checking if SRT and then comparing to min burst time
	//TODO: Add E(0), E(1) and B(0) init to include/process.h 
	return 0;
} 

//Returns the pid of the next process to schedule under the TSS algorithim
pid32 schedulerTSS(void){

	return 0;
} 

//Computes and assigns the burst time of the process specified in the argument
void computeBurst(struct procent const *ptold){

}
