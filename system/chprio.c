/* chprio.c - chprio */

#include <xinu.h>

struct groupPriority grp_pri;
/*------------------------------------------------------------------------
 *  chprio  -  Change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
pri16	chprio(
	  pid32		pid,		/* ID of process to change	*/
	  pri16		newprio		/* New priority			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	pri16	oldprio;		/* Priority to return		*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return (pri16) SYSERR;
	}
	prptr = &proctab[pid];
	oldprio = prptr->prprio;
	prptr->prprio = newprio;
	restore(mask);
	return oldprio;
}

pri16 chgprio(
	  int group,  			/* Group of processes to change */
	  pri16 newprio			/* New group priority */
	)
{
	intmask mask;
	pri16 oldprio;
	mask = disable();

	if(group == TSSCHED){
		oldprio = grp_pri.TSS_init_pri;
		grp_pri.TSS_init_pri = newprio;
	}
	else if(group == SRTIME){
		oldprio = grp_pri.SRT_init_pri;
		grp_pri.SRT_init_pri = newprio;
	}
	else{
		//bad process group
		restore(mask);
		return (pri16) SYSERR;
	}
	restore(mask);
	return oldprio;

}
