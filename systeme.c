
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "cpu.h"
#include "asm.h"
#include "systeme.h"




/**********************************************************
** Structures de données de l'ordonnanceur (représentation
** d'un ensemble de processus).
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

#define MAX_PROCESS  (20)   /* nb maximum de processus  */

typedef enum {
    EMPTY = 0,              /* processus non-prêt       */
    READY = 1,              /* processus prêt           */
    SLEEP = 2,
} STATE;                    /* État d'un processus      */

typedef struct {
    PSW    cpu;             /* mot d'état du processeur */
    STATE  state;           /* état du processus        */
} PCB;                      /* Un Process Control Block */

PCB process[MAX_PROCESS];   /* table des processus      */

int current_process = -1;   /* nu du processus courant  */
int nb_ready        =  0;   /* nb de processus prêts    */


/**********************************************************
** Changer l'état d'un processus.
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

void change_state(int p, STATE new_state) {
    assert((0 <= p) && (p < MAX_PROCESS));
    STATE old_state = process[p].state;
    process[p].state = new_state;
    if (old_state == READY) nb_ready--;
    if (new_state == READY) nb_ready++;
}


/**********************************************************
** Ordonnancer l'exécution des processus.
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/
void wakeup (int a) {
    process[a].state = READY;
}

PSW scheduler(PSW cpu) {

    if(current_process > -1){
       
        process[current_process].cpu = cpu;
        
        }
    do 
    { 
        current_process = (current_process + 1) % MAX_PROCESS;
    } while (process[current_process].state != READY);

        int p = 0;
        for (p = 0; p<MAX_PROCESS; p++) {
            if(process[p].state == SLEEP) {
             wakeup(p);
            }
        }
    
    return process[current_process].cpu;
}


// Demarrage du systeme (creation d'un programme)

PSW prepare_idle(void) {
    PSW idle = { .PC = 120, .SB = 120, .SE = 125, };
    assemble(idle.SB, "idle.asm");
    return idle;
}


/**********************************************************
** Démarrage du système (création d'un programme)
***********************************************************/

PSW system_init(void) {

    printf("Booting\n");
    
    /*** création d'un programme P1 ***/
    
    PSW P1_cpu = {
        .PC = 20, // le code démarre au début du segment
        .SB = 20, // le segment débute à l'adr physique 20
        .SE = 30, // et se termine à l'adresse 30
                  // les autres registres sont à zéro
    };
    
     current_process = 1;
    process[current_process].cpu = P1_cpu;
    process[current_process].state = READY;



    PSW P2_cpu = {
        .PC = 30, // le code démarre au début du segment
        .SB = 40, // le segment débute à l'adr physique 20
        .SE = 50, // et se termine à l'adresse 30
                  // les autres registres sont à zéro
    };
    current_process = 2;
    process[current_process].cpu = P2_cpu;
    process[current_process].state = READY;

    PSW P3_cpu_sleep_test = {

        .PC = 60, 
        .SB = 70,
        .SE = 80, 
    };

    current_process = 2;
    process[current_process].cpu = P2_cpu;
    process[current_process].state = READY;
    
    
    assemble(P1_cpu.SB, "prog1.asm");
    assemble(P2_cpu.SB, "prog1.asm");
    assemble(P3_cpu_sleep_test.SB  ,"sleep.asm");
    PSW prepare_idle(void);
    
    return P1_cpu;
   // return P2_cpu;
}


/* La fonction thread */

int new_thread(PSW cpu) {
	int child = 0;
	
	while (process[child].state != EMPTY) {
		
        if (process[child].state == EMPTY)
            {
        	process[child].cpu = cpu;
        	process[child].state = READY;
            assemble(process[child].cpu.SB, "prog2.asm");
            nb_ready = nb_ready + 1;
            cpu.PC = cpu.PC + 1;
	        break;
            }
             else 
            {
                child = child +1;
            }
    }
	return nb_ready;
}


PSW sysc_new_thread(PSW cpu) {
		cpu.IN = cpu.DR[cpu.RI.i] = new_thread(cpu);
			return cpu;
}



/**********************************************************
** Traitement des appels au système
***********************************************************/



PSW sysc_exit(PSW cpu) {
    exit(EXIT_SUCCESS);
    return cpu;
}

PSW sysc_puti(PSW cpu) {
    printf("Entier : %d \n", cpu.RI.arg);
    return cpu;
}
enum {
    SYSC_EXIT = 100,   // fin du processus courant
    SYSC_PUTI = 200,   // afficher le contenu de Ri
    SYSC_NEW_THREAD = 300,
    SYSC_SLEEP = 400,
};


static PSW process_system_call(PSW cpu) {
    // suivant l'argument de sysc Ri, Rj, ARG
    switch (cpu.RI.arg) {
        case SYSC_EXIT:
     		  
                    exit(EXIT_FAILURE);
                
                break;
        case SYSC_PUTI:
	    	printf("case puti : %d \n", cpu.RI.i);
                break;
            
        case SYSC_NEW_THREAD:
                new_thread(cpu);
                break;
        
        case SYSC_SLEEP:
            sleep(1);
            break;
        default:
            printf("Appel système inconnu %d\n", cpu.RI.arg);
            break;
    }
    return cpu;
}


/**********************************************************
** Traitement des interruptions par le système (mode système)
***********************************************************/

PSW process_interrupt(PSW cpu) {
    	printf("\n_______ P%d __________\n", current_process);
	printf(" le nombre des interruption est : %d\n",cpu.IN);
	 switch (cpu.IN) {
        case INT_SEGV:
            printf("l'interruption type segv : %d \n",cpu.IN);
            exit(EXIT_FAILURE);
            break;
        case INT_INST:
            printf("l'interruption type inst : %d \n",cpu.IN);
            exit(EXIT_FAILURE);
            break;
        case INT_TRACE:
             dump_cpu(cpu);// sleep(1);
             cpu = scheduler(cpu);
             printf("l'interruption de type trace : %d \n",cpu.IN);
             printf("numéro du process : %d \n",current_process);

            break;
        case INT_SYSC:
            printf(" l'interruption de type sysc : %d \n",cpu.IN);
            cpu = process_system_call(cpu);
            break;
        case INT_KEYBOARD:
            printf(" l'interruption de type keyboard : %d \n",cpu.IN);
            break;
        default:
            break;
    }
    return cpu;

}

