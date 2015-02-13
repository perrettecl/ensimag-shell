/*****************************************************
 * Copyright Grégory Mounié 2008-2013                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

#define EXIT_ON_FAILURE -1

typedef struct {
	pid_t pid;
	int numCmd;
	char ** nomCmd;
	processus * suiv;
} processus;

processus * new_processus(pid_t pid, int numCmd, char ** nomCmd, processus * suivant) {
	processus * nvProc = malloc(sizeof(processus));

	nvProc->pid = pid;
	nvProc->numCmd = numCmd;
	nvProc->nomCmd = nomCmd;
	nvProc->suiv = suivant;

	return nvProc;
}

int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);
	
	processus * listeProc = NULL;
	
	int numCmd = 0;
	//int status;
	
	while (1) {
		struct cmdline *l;
		//int i, j;
		char *prompt = "ensishell>";
		
		numCmd++;

		l = readcmd(prompt);

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		/*
                if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);
		if (l->bg) printf("background (&)\n");
                */

		if(l->seq[0]!=0) {
			
			pid_t pid_fils = fork();
			
			if(pid_fils == -1) {
				if(errno == EAGAIN)
					perror("Impossible d'allouer suffisamment de mémoire ");
				else if (errno == ENOMEM)
					perror("Le noyau n'a plus assez de mémoire ");
				
				exit(EXIT_ON_FAILURE);
			} else if(pid_fils == 0) {
				
				//on se trouve dans le fils
				char **cmd = l->seq[0];
				int err = execvp(cmd[0], cmd);
				
				if(err == -1) {
					perror("Commande inexistante ");
					exit(EXIT_ON_FAILURE);
				}
				
			} else {
				//on se trouve dans le père
				if(!l->bg) {
					//on attend le fils
					wait(0);
				} else {
					char **cmd = l->seq[0];
					listeProc = new_processus(pid_fils, numCmd, cmd, listeProc);
					
					printf("[%d] %d\n", numCmd, pid_fils);
				}
				
				//waitpid(-1, &status, WNOHANG);
			}
		}
		/* Display each command of the pipe */
		/*
                for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
                        for (j=0; cmd[j]!=0; j++) {
                                printf("'%s' ", cmd[j]);
                        }
			printf("\n");
		}
                */

	}
}
