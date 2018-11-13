#include "nx_lib.h"


static struct pid {
	struct pid *next;
	FILE *fp;
	pid_t pid;
} *pidlist;

extern char **environ;


FILE * nx_popen(const char *program, const char *type)
{
	struct pid * volatile cur;
	FILE *iop;
	int pdes[2];
	pid_t pid;
	char *argp[] = {"sh", "-c", NULL, NULL};

	if ((*type != 'r' && *type != 'w') || type[1] != '\0') {
		errno = EINVAL;
		return (NULL);
	}

	if ((cur = malloc(sizeof(struct pid))) == NULL)
		return (NULL);

	if (pipe(pdes) < 0) {
		free(cur);
		return (NULL);
	}

	switch (pid = vfork()) {
		case -1:	/* Error. */
			(void)close(pdes[0]);
			(void)close(pdes[1]);
			free(cur);
			return (NULL);
			/* NOTREACHED */

		case 0:	/* Child. */
			{
				struct pid *pcur;
				/*
				 * We fork()'d, we got our own copy of the list, no
				 * contention.
				 */
				for (pcur = pidlist; pcur; pcur = pcur->next)
					close(fileno(pcur->fp));
				if (*type == 'r') {
					(void) close(pdes[0]);
					if (pdes[1] != STDOUT_FILENO) {
						(void)dup2(pdes[1], STDOUT_FILENO);
						(void)close(pdes[1]);
					}
				} else {
					(void)close(pdes[1]);
					if (pdes[0] != STDIN_FILENO) {
						(void)dup2(pdes[0], STDIN_FILENO);
						(void)close(pdes[0]);
					}
				}
				argp[2] = (char *)program;
#define X_PATH_BSHELL		"/system/bin/sh"
				execve(X_PATH_BSHELL, argp, environ);
//				execl(program, program, NULL);
//				execl("/system/bin/dt", "/system/bin/dt", "-p", "/data/test", NULL);
//				execlp("dt", "dt", "-p", "/data/test", NULL);
//				execlp("ls", "ls", "-al", NULL);
				_exit(127);
				/* NOTREACHED */
			}
	}

	/* Parent; assume fdopen can't fail. */
	if (*type == 'r') {
		iop = fdopen(pdes[0], type);
		(void)close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], type);
		(void)close(pdes[0]);
	}

	/* Link into list of file descriptors. */
	cur->fp = iop;
	cur->pid = pid;
	cur->next = pidlist;
	pidlist = cur;

	return (iop);
}

/*
 * pclose --
 * Pclose returns -1 if stream is not associated with a `popened' command,
 * if already `pclosed', or waitpid returns an error.
 */
int nx_pclose(FILE *iop)
{
	struct pid *cur, *last;
	int pstat;
	pid_t pid;

	/* Find the appropriate file pointer. */
	for (last = NULL, cur = pidlist; cur; last = cur, cur = cur->next)
		if (cur->fp == iop)
			break;

	if (cur == NULL)
		return (-1);

	(void)fclose(iop);

	do {
		pid = waitpid(cur->pid, &pstat, 0);
	} while (pid == -1 && errno == EINTR);

	/* Remove the entry from the linked list. */
	if (last == NULL)
		pidlist = cur->next;
	else
		last->next = cur->next;
	free(cur);

	return (pid == -1 ? -1 : pstat);
}
