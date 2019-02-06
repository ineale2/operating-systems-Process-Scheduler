/* resched.h */

/* Constants and variables related to deferred rescheduling */

#define	DEFER_START	1	/* Start deferred rescehduling		*/
#define	DEFER_STOP	2	/* Stop  deferred rescehduling		*/

/* The follwing constants defines the two process types for scheduling*/
#define SRTIME 0
#define TSSCHED 1

/* Scheduling policy constants*/
#define ALPHA 0.7

/* Structure that collects items related to deferred rescheduling	*/

struct	defer	{
	int32	ndefers;	/* Number of outstanding defers 	*/
	bool8	attempt;	/* Was resched called during the	*/
				/*   deferral period?			*/
};


/* Structure that collects items realted to group scheduling priorities */
struct groupPriority 	{
	pri16 TSS_pri;
	pri16 TSS_init_pri;
	pri16 SRT_pri;
	pri16 SRT_init_pri;
};


extern	struct	defer	Defer;
extern  struct  groupPriority grp_pri;
//agingSched implements the agingScheduler algorithim and selects one of the two groups (SRT or TSS) to schedule
//returns true to schedule SRT, false to schedule TSS
bool8 agingSched(void);

//Returns the pid of the next process to schedule under the TSS algorithim
pid32 schedulerSRT(void);

//Returns the pid of the next process to schedule under the TSS algorithim
pid32 schedulerTSS(void);

//Computes and assigns the burst time of the process specified in the argument
void computeBurst(pid32);

//Sets the start time variables for the process specified in the argument
void setStartTime(pid32);
