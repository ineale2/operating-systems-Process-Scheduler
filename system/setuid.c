/* setuid.c - setuid */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  setuid  -  set the user ID of a process
 *------------------------------------------------------------------------
 */
syscall	setuid(
	  int		newuid		/* new uid for a process*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();

	//Attempting to change current processes uid
	prptr = &proctab[currpid];

	//Only root processes can change uid
	if (prptr->uid != ROOT) {
		restore(mask);
		return SYSERR;
	}

	//Change uid to argument
	prptr->uid = newuid;

	restore(mask);
	return OK;
}
