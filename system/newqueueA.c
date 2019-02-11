/* newqueueA.c - newqueueA */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newqueueA  -  Allocate and initialize a ASCENDING queue in the global queue table
 *------------------------------------------------------------------------
 */
qid16	newqueueA(void)
{
	qid16		q;		/* ID of allocated queue 	*/

	q = newqueue();

	queuetab[queuehead(q)].qkey  = MINKEY;
	queuetab[queuetail(q)].qkey  = MAXKEY;
	return q;
}
