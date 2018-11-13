#include <sys/param.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>

FILE * nx_popen(const char *program, const char *type);
/*
 * pclose --
 * Pclose returns -1 if stream is not associated with a `popened' command,
 * if already `pclosed', or waitpid returns an error.
 */
int nx_pclose(FILE *iop);
