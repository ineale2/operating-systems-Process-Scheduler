/* xsh_hello.c - xsh_hello */

#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xhs_hello - calls the system call hello
 *------------------------------------------------------------------------
 */
shellcmd xsh_hello(int nargs, char *args[])
{
	hello();	

	return 0;
}
