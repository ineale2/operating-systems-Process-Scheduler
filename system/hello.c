/* hello.c - hello */
/* Lab0 Code: 1/21/2019 */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  hello - print hello  
 *------------------------------------------------------------------------
 */
syscall	hello(
	void
	)
{
#if XTEST
	kprintf("Hello system call invoked\n");
#endif
	return 0;
}
