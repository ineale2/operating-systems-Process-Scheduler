/*  main.c  - main */

#include <xinu.h>
#include <test_lab1.h>

process	main(void)
{

	/* Run the Xinu shell */

	recvclr();
	pid32 pid = create(shell, 8192, SRTIME, 50, "shell", 1, CONSOLE);
	XTEST_KPRINTF("Spawning new shell with PID = %d...\n", pid);
	resume(pid);

	//resume(create(test_uid, 8192, TSSCHED, 50, "test_uid", 0));
	//resume(create(test_lab1, 8192, TSSCHED, 50, "test_lab1", 0));
	//resume(create(test_chgprio, 8192, SRTIME, 50, "test_chgprio", 0));
	resume(create(test_longRunningProc, 8192, SRTIME, 50, "test_LRP", 0));
	/* Wait for shell to exit and recreate it */
/*
	recvclr();
	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		pid = create(shell, 4096, SRTIME, 20, "shell", 1, CONSOLE);
		XTEST_KPRINTF("Spawning new shell with PID = %d...\n", pid);
		resume(pid);
	}
*/
	return OK;
    
}
