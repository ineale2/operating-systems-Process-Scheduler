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
int16 agingSched(void);

//Computes and assigns the burst time of the process specified in the argument
void computeBurst(struct procent*);

void insertCurrProc(void);

//Assign next priority and quantum based on current priority
void assignTimes(struct procent*);

void printReadyList(qid16);
void printProcTab(int);
