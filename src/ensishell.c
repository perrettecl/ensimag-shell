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
#include <signal.h>

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

#define EXIT_ON_FAILURE -1
#define PID_INEXISTANT_LISTE -1
#define PID_SUPPRIME 0

#define IN 0
#define OUT 1


/* fonction et types de gestion de la liste des processus */
struct liste_processus {
	pid_t pid;
	int numCmd;
	char* nomCmd;
	struct liste_processus * suiv;
};
typedef struct liste_processus T_liste_processus;

T_liste_processus* new_processus(pid_t pid, int numCmd, char* nomCmd, T_liste_processus* suivant) {
	T_liste_processus * nvProc = malloc(sizeof(T_liste_processus));

	nvProc->pid = pid;
	nvProc->numCmd = numCmd;
	
	//copie du nom
	nvProc->nomCmd = malloc(strlen(nomCmd)+1);
	strcpy(nvProc->nomCmd, nomCmd);
	
	nvProc->suiv = suivant;

	return nvProc;
}

T_liste_processus* supprimer_processus(T_liste_processus* p) {
	free(p->nomCmd);
	T_liste_processus* proc_suivant = p->suiv;
	free(p);
	return proc_suivant;
}

void afficher_liste_processus(T_liste_processus* liste) {
    T_liste_processus* cour = liste;
    while (cour != NULL) {
    	//printf("[%d]\n", cour->pid);
        printf("[%d] %s\n", cour->pid, cour->nomCmd);
        cour = cour->suiv;
    }
}

T_liste_processus* listeProc = NULL;
int numCmd_Glob = 0;


void gestion_signaux(int sig)
{
	pid_t id=0;
	int status=0;
	
	switch (sig)
	{
		case SIGCHLD :
			//on recupere le pid du processus 
			id=wait(&status);
			
			/*suppression du processus dans la liste*/
			T_liste_processus* cour = listeProc;
			T_liste_processus* precedent = NULL;
    			while (cour != NULL)
    			{
    				/*On a trouvé dans la liste*/
    				if(cour->pid == id)
    				{
    					printf("[%d]+ Done \t %s\n", cour->numCmd, cour->nomCmd);
    					
    					T_liste_processus* suiv = supprimer_processus(cour);
    					
    					/* cas du premier */
    					if(precedent == NULL)
						listeProc = suiv;
    					else
						precedent->suiv =suiv;
					
					
					
					/* on decremante le numero */
					if(listeProc != NULL)
						numCmd_Glob = listeProc->numCmd;
					else
						numCmd_Glob = 0;
					
					/*fin*/	
					break;
							
    				}
    				
    				precedent = cour;
    			}
		
		break;
		
		default : break;
	
	}
		
}

int main() {
    printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);


	while (1) {
		struct cmdline *l;
		//int i, j;
		char *prompt = "ensishell>";
		

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

		/* compteur de commandes */
		int nb_commandes=0;
		while(l->seq[nb_commandes]!=NULL)
			nb_commandes++;
			
		int commande_courante = 0;
		
		int* lesFils = (int*)malloc(nb_commandes*sizeof(int));
				
		/* creation des pipes */
		int** pipes;
 		if(nb_commandes > 1)
		{
			pipes = (int**)malloc((nb_commandes-1)*sizeof(int*));
			for(int i =0; i < nb_commandes-1 ; i++)
			{
				pipes[i] = (int*)malloc(2*sizeof(int));
				pipe(pipes[i]);
			}
			
				
			
		}
		
		while(l->seq[commande_courante]!=NULL) {
			
			pid_t pid_fils = fork();
			
			if(pid_fils == -1) {
				if(errno == EAGAIN)
					perror("Impossible d'allouer suffisamment de mémoire ");
				else if (errno == ENOMEM)
					perror("Le noyau n'a plus assez de mémoire ");
				
				exit(EXIT_ON_FAILURE);
			} else if(pid_fils == 0) {
				
				//on se trouve dans le fils
				/*redirection entree*/
				if(commande_courante == 0 && l->in)
				{
					FILE* fichier = NULL;
					fichier = fopen(l->in, "r");
					
					if(fichier == NULL)
					{
						perror("Impossible d'ouvrir le fichier d'entrée ");
						exit(EXIT_ON_FAILURE);
					}
					
					dup2(fileno(fichier), IN);
					fclose(fichier);
				}
				
				
				/* mise en place des pipes */
				if(nb_commandes > 1)
				{
					/* sortie */
					if(commande_courante != nb_commandes-1)
						dup2(pipes[commande_courante][OUT],OUT);
					
					/*entree*/
					if(commande_courante != 0)
						dup2(pipes[commande_courante-1][IN],IN);
						
					/* on ferme les pipes inutiles */
					for(int i =0; i < nb_commandes-1; i++)
					{
						if(commande_courante == nb_commandes-1 || i != commande_courante)
							close(pipes[i][OUT]);
							
						if(commande_courante == 0 || i != commande_courante-1)
							close(pipes[i][IN]);
						
						
					}
						


				}
				
				/* mise en place de la redirection en sortie */
				if(commande_courante == nb_commandes-1 && l->out)
				{
					FILE* fichier = NULL;
					fichier = fopen(l->out, "w");
					
					if(fichier == NULL)
					{
						perror("Impossible d'ouvrir le fichier de sortie ");
						exit(EXIT_ON_FAILURE);
					}
					
					dup2(fileno(fichier), OUT);
					fclose(fichier);
				}
				
				char **cmd = l->seq[commande_courante];
				int err = 0;
				
				if(strcmp(l->seq[commande_courante][0], "jobs") == 0)
				{
					afficher_liste_processus(listeProc);
					exit(0);
				}
				else
					err = execvp(cmd[0], cmd);
				
				if(err == -1) {
					perror("Commande inexistante ");
					exit(EXIT_ON_FAILURE);
				}
				
				
			} else {
				//on se trouve dans le père
				//on est dans la derniere commande
				//if(commande_courante == nb_commandes-1)
				lesFils[commande_courante] = pid_fils;
				
				if(commande_courante == nb_commandes-1)
				{
					if(!l->bg) {
					
						waitpid(pid_fils, 0, WUNTRACED);
												
					} else {
						char* nomcmd = *(l->seq[commande_courante]);
						numCmd_Glob++;
						listeProc = new_processus(pid_fils, numCmd_Glob, nomcmd, listeProc);
					
						printf("[%d] %d\n", numCmd_Glob, pid_fils);
					
						//fonction de fin du fils
						signal(SIGCHLD,gestion_signaux);
					}
				}
				
				commande_courante++;
				

				
			}
			
			//on free les pipes
			/*for(int i=0; i < nb_commandes-1; i++)
				free(pipes[i]); 
				
			free(pipes);*/
			
			
		}


	}
}
