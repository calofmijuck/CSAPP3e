/*
 * tsh - A tiny shell program with job control
 *
 * <Put your student number and login ID here>
 * 2017-18570
 * stu133
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs);
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid);
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

// Wrapper functions for system calls
void Kill(pid_t pid, int sig);
pid_t Fork(void);
void Setpgid(pid_t pid, pid_t pgid);
void Sigemptyset(sigset_t* set);
void Sigaddset(sigset_t* set, int signum);
void Sigprocmask(int how, const sigset_t* set, sigset_t* oldset);

// Sio Functions
void sio_reverse(char s[]);
void sio_lota(long v, char s[], int b);
size_t sio_strlen(char s[]);
ssize_t sio_puts(char s[]);
ssize_t sio_putl(long v);
void sio_error(char s[]);
ssize_t Sio_puts(char s[]);
ssize_t Sio_putl(long v);
void Sio_error(char s[]);


/*
 * main - The shell's main routine
 */
int main(int argc, char **argv) {
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	        break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	        break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	        break;
	    default:
            usage();
	    }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

    	/* Read command line */
    	if (emit_prompt) {
    	    printf("%s", prompt);
    	    fflush(stdout);
    	}
    	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
    	    app_error("fgets error");
    	if (feof(stdin)) { /* End of file (ctrl-d) */
    	    fflush(stdout);
    	    exit(0);
    	}

    	/* Evaluate the command line */
    	eval(cmdline);
    	fflush(stdout);
    	fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
*/
void eval(char *cmdline) {
    char* argv[MAXARGS]; // Argument list execve()
    char buf[MAXLINE]; // Holds modified command line
    int bg; // Foreground? Background?
    pid_t pid; // Process ID
    sigset_t mask; // Mask for blocking signals

    strcpy(buf, cmdline); // copy to buf
    bg = parseline(buf, argv); // check if the process should run background

    if(argv[0] == NULL) return; // Ignore empty lines

    if(!builtin_cmd(argv)) {
        Sigemptyset(&mask); // signal set
        Sigaddset(&mask, SIGCHLD); // mask sigchld
        Sigprocmask(SIG_BLOCK, &mask, NULL); // Block signal

        if((pid = Fork()) == 0) { // Child process
            Setpgid(0, 0); // Child's group -> new process group, identical to child's PID
            Sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock before execve

            if(execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        if(!bg) { // running in the foreground
            addjob(jobs, pid, FG, cmdline);
            Sigprocmask(SIG_UNBLOCK, &mask, NULL); // Unblock here
            waitfg(pid); // wait foreground job to finish
        } else { // running in the background
            addjob(jobs, pid, BG, cmdline);
            Sigprocmask(SIG_UNBLOCK, &mask, NULL);
            printf("[%d] (%d) %s", pid2jid(pid), (int) pid, cmdline); // background process info
        }
    }

    return;
}

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int parseline(const char *cmdline, char **argv) {
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	    buf++;

    /* Build the argv list */
    argc = 0;
    if(*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
    } else {
	    delim = strchr(buf, ' ');
    }

    while (delim) {
    	argv[argc++] = buf;
    	*delim = '\0';
    	buf = delim + 1;
    	while (*buf && (*buf == ' ')) /* ignore spaces */
    	    buf++;

    	if(*buf == '\'') {
    	    buf++;
    	    delim = strchr(buf, '\'');
    	} else {
    	    delim = strchr(buf, ' ');
    	}
    }
    argv[argc] = NULL;

    if (argc == 0)  /* ignore blank line */
	    return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	    argv[--argc] = NULL;
    }
    return bg;
}

/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
int builtin_cmd(char **argv) {
    if(!strcmp(argv[0], "quit")) // quit command
        exit(0);
    if(!strcmp(argv[0], "&")) // ignore singleton &
        return 1;
    if(!strcmp(argv[0], "jobs")) { // job list
        listjobs(jobs);
        return 1;
    }
    if(!strcmp(argv[0], "fg")) {
        do_bgfg(argv);
        return 1;
    }
    if(!strcmp(argv[0], "bg")) {
        do_bgfg(argv);
        return 1;
    }

    return 0;     /* not a builtin command */
}

/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) {
    // Check if the job has second argument
    if(argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return;
    }

    // Check if second argument is valid for the job
    if(!isdigit(argv[1][0]) && argv[1][0] != '%') {
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return;
    }

    int jid = (argv[1][0] == '%' ? 1 : 0); // check if job id is given
    struct job_t *job;

    if(jid) { // job id is given
        job = getjobjid(jobs, atoi(&argv[1][1])); // Get job id
        if(job == NULL) { // is given job active ?
            printf("%s: No such job\n", argv[1]);
            return;
        }
    } else { // process id is given
        job = getjobjid(jobs, (pid_t) atoi(argv[1]));
        if(job == NULL) {
            printf("(%d): No such process\n", atoi(argv[1]));
            return;
        }
    }

    if(!strcmp(argv[0], "bg")) {
        job -> state = BG; // Set state to background
        printf("[%d] (%d) %s", job -> jid, job -> pid, job -> cmdline);
        Kill(-(job -> pid), SIGCONT); // send sigcont signal
    } else {
        job -> state = FG;
        Kill(-(job -> pid), SIGCONT);
        waitfg(job -> pid); // must wait for the foreground job to finish
    }

    return;
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid) {
    while(fgpid(jobs) == pid) {
        sleep(1); // run until fg job is done
    }
    return;
}

/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
void sigchld_handler(int sig) {
    pid_t pid;
    int status;
    int jobid;

    // Hard part...
    while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) { // Start Reaping!
        jobid = pid2jid(pid);

        // Follow the explanation in p 781 of the textbook

        if(WIFEXITED(status)) { // true if child has terminated normally
            deletejob(jobs, pid); // delete the child
        } else if(WIFSIGNALED(status)) { // child terminated by a signal
            // WIFSIGNALED: Returns true if the child process terminated because of a signal that was not caught
            deletejob(jobs, pid);
            Sio_puts("Job [");
            Sio_putl(jobid);
            Sio_puts("] (");
            Sio_putl(pid);
            Sio_puts(") terminated by signal ");
            Sio_putl(WTERMSIG(status));
            Sio_puts("\n");
            // printf("Job [%d] (%d) terminated by signal %d\n", jobid, (int) pid, WTERMSIG(status));
        } else if(WIFSTOPPED(status)) {
            getjobpid(jobs, pid) -> state = ST; // update to stopped
            Sio_puts("Job [");
            Sio_putl(jobid);
            Sio_puts("] (");
            Sio_putl(pid);
            Sio_puts(") stopped by signal ");
            Sio_putl(WSTOPSIG(status));
            Sio_puts("\n");
            // printf("Job [%d] (%d) stopped by signal %d\n", jobid, (int) pid, WSTOPSIG(status));
        }
    }
    return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig) {
    pid_t pid = fgpid(jobs); // Get PID of foreground process

    if(pid != 0) {
        Kill(-pid, sig);
    }
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig) {
    pid_t pid = fgpid(jobs);
    if(pid != 0) {
        Kill(-pid, sig);
    }
    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for(i = 0; i < MAXJOBS; i++)
	    clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) {
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) {
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
    	if (jobs[i].pid == 0) {
    	    jobs[i].pid = pid;
    	    jobs[i].state = state;
    	    jobs[i].jid = nextjid++;
    	    if (nextjid > MAXJOBS)
    		    nextjid = 1;
    	    strcpy(jobs[i].cmdline, cmdline);
      	    if(verbose) {
    	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
    	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	    return 0;

    for (i = 0; i < MAXJOBS; i++) {
    	if (jobs[i].pid == pid) {
    	    clearjob(&jobs[i]);
    	    nextjid = maxjid(jobs)+1;
    	    return 1;
    	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
    	if (jobs[i].state == FG)
    	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	   return NULL;
    for (i = 0; i < MAXJOBS; i++)
    	if (jobs[i].pid == pid)
    	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) {
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) {
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
        return jobs[i].jid;
    }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG:
		    printf("Running ");
		    break;
		case FG:
		    printf("Foreground ");
		    break;
		case ST:
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ",
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void)
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if(sigaction(signum, &action, &old_action) < 0)
	    unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) {
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

// Check exit status of system calls
void Kill(pid_t pid, int sig) {
    if(kill(pid, sig) < 0)
        unix_error("kill error");
}

pid_t Fork() {
    pid_t pid;
    if((pid = fork()) < 0) {
        unix_error("fork error");
    }
    return pid;
}

void Sigemptyset(sigset_t* set) {
    if(sigemptyset(set) < 0) {
        app_error("sigemptyset error");
    }
}

void Sigaddset(sigset_t* set, int signum) {
    if(sigaddset(set, signum) < 0) {
        app_error("sigaddset error");
    }
}

void Sigprocmask(int how, const sigset_t* set, sigset_t* oldset) {
    if(sigprocmask(how, set, oldset) < 0) {
        app_error("sigprocmask error");
    }
}

void Setpgid(pid_t pid, pid_t pgid) {
    if(setpgid(pid, pgid) < 0) {
        unix_error("setpgid error");
    }
}


// Sio
/* Private sio functions */
/* $begin sioprivate */
/* sio_reverse - Reverse a string (from K&R) */
void sio_reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* sio_ltoa - Convert long to base b string (from K&R) */
void sio_ltoa(long v, char s[], int b)
{
    int c, i = 0;
    int neg = v < 0;

    if (neg)
	v = -v;

    do {
        s[i++] = ((c = (v % b)) < 10)  ?  c + '0' : c - 10 + 'a';
    } while ((v /= b) > 0);

    if (neg)
	s[i++] = '-';

    s[i] = '\0';
    sio_reverse(s);
}

/* sio_strlen - Return length of string (from K&R) */
size_t sio_strlen(char s[])
{
    int i = 0;

    while (s[i] != '\0')
        ++i;
    return i;
}
/* $end sioprivate */

/* Public Sio functions */
/* $begin siopublic */

ssize_t sio_puts(char s[]) /* Put string */
{
    return write(STDOUT_FILENO, s, sio_strlen(s)); //line:csapp:siostrlen
}

ssize_t sio_putl(long v) /* Put long */
{
    char s[128];

    sio_ltoa(v, s, 10); /* Based on K&R itoa() */  //line:csapp:sioltoa
    return sio_puts(s);
}

void sio_error(char s[]) /* Put error message and exit */
{
    sio_puts(s);
    _exit(1);                                      //line:csapp:sioexit
}
/* $end siopublic */

/*******************************
 * Wrappers for the SIO routines
 ******************************/
ssize_t Sio_putl(long v) {
    ssize_t n;

    if ((n = sio_putl(v)) < 0)
	sio_error("Sio_putl error");
    return n;
}

ssize_t Sio_puts(char s[]) {
    ssize_t n;

    if ((n = sio_puts(s)) < 0)
	sio_error("Sio_puts error");
    return n;
}

void Sio_error(char s[]) {
    sio_error(s);
}
