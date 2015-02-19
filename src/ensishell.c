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
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

#define EXIT_ON_FAILURE -1
#define PID_INEXISTANT_LISTE -1
#define PID_SUPPRIME 0


typedef struct liste_processus {
	pid_t pid;
	int numCmd;
	char ** nomCmd;
	struct liste_processus * suiv;
} liste_processus;

struct liste_processus * new_processus(pid_t pid, int numCmd, char ** nomCmd, struct liste_processus * suivant) {
	struct liste_processus * nvProc = malloc(sizeof(struct liste_processus));

	nvProc->pid = pid;
	nvProc->numCmd = numCmd;
	nvProc->nomCmd = nomCmd;
	nvProc->suiv = suivant;

	return nvProc;
}

void afficher_liste_processus(struct liste_processus * liste) {
    struct liste_processus * cour = liste;
    while (cour != NULL) {
        printf("[%d] %s\n", cour->pid, cour->nomCmd[0]);
        cour = cour->suiv;
    }
}

int supprimer_processus_liste(pid_t pid_proc, struct liste_processus * liste {
        return PID_SUPPRIME;
}

int main() {
    printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);
	
	struct liste_processus * listeProc = NULL;
	
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
		
            if(strcmp(l->seq[0][0], "jobs") == 0) {
                    afficher_liste_processus(listeProc);
                    continue;
            }

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
					waitpid(pid_fils, 0, WUNTRACED);
				} else {
					char **cmd = l->seq[0];
					listeProc = new_processus(pid_fils, numCmd, cmd, listeProc);
					
					printf("[%d] %d\n", numCmd, pid_fils);
				}
				
                //on attend les processus en bg, si un termine, on l'affiche et
                //                              on le supprime de la liste
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
