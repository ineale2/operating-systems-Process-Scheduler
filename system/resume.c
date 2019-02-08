/* resume.c - resume */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  resume  -  Unsuspend a process, making it ready
 *------------------------------------------------------------------------
 */
pri16	resume(
	  pid32		pid		/* ID of process to unsuspend	*/
	)
{
	intmask	mask;				/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	struct	procent *cprptr;	/* Ptr to current processes's table entry	*/
	pri16	prio;				/* Priority to return		*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return (pri16)SYSERR;
	}
	
	prptr = &proctab[pid];

	cprptr = &proctab[currpid];
	//Only allow non-root users to resume if uid matches
	if( (cprptr->uid != ROOT) && (cprptr->uid != prptr->uid) ){
		restore(mask);
		return SYSERR;
	}

	if (prptr->prstate != PR_SUSP) {
		restore(mask);
		return (pri16)SYSERR;
	}
	prio = prptr->prprio;		/* Record priority to return	*/
	ready(pid);
	restore(mask);
	return prio;
}
