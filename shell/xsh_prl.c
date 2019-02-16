/* xsh_printReadyList.c */

#include <xinu.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xsh_prl - shell command to print the process table
 *------------------------------------------------------------------------
 */
void prl(qid16);

shellcmd xsh_prl(int nargs, char *args[]){
	
	intmask mask;
	mask = disable();
	kprintf("SRT Ready List:\n");
	prl(readylistSRT);
	
	kprintf("\nTSS Ready List:\n");
	prl(readylistTSS);
	restore(mask);
	return 0;
}

void prl(qid16 list){
	struct procent *prptr;

	char *pstate[]	= {		/* names for process states	*/
		"free ", "curr ", "ready", "recv ", "sleep", "susp ",
		"wait ", "rtime"};

	char *group[] = {"SRT","TSS"};
	qid16 curr = firstid(list);
	kprintf("%3s %-16s %5s %4s %4s %10s %-10s %10s %8s %10s %6s %5s %4s\n",
		   "Pid", "Name", "State", "Prio", "Ppid", "Stack Base",
		   "Stack Ptr", "Stack Size", "EB", "prev_burst", "prevEB", "group", "qkey");

	kprintf("%3s %-16s %5s %4s %4s %10s %-10s %10s %8s %10s %6s %5s %4s\n",
		   "---", "----------------", "-----", "----", "----",
		   "----------", "----------", "----------", "--------", "----------", "------",  "-----","----");
	while(curr != queuetail(list)){
		prptr = &proctab[curr];
		kprintf("%3d %-16s %s %4d %4d 0x%08X 0x%08X %10d %8d %10d %6d %5s %4d\n",
			curr, prptr->prname, pstate[(int)prptr->prstate],
			prptr->prprio, prptr->prparent, prptr->prstkbase,
			prptr->prstkptr, prptr->prstklen, prptr->EB, prptr->prev_burst, prptr->prev_EB,
			group[prptr->sched_alg], queuetab[curr].qkey);
		curr = queuetab[curr].qnext;
	}

}
