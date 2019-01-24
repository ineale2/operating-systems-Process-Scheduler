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
	XTEST_KPRINTF("Hello system call invoked\n");
	return 0;
}
