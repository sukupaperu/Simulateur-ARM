/*
Armator - simulateur de jeu d'instruction ARMv5T à but pédagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Générale GNU publiée par la Free Software
Foundation (version 2 ou bien toute autre version ultérieure choisie par vous).

Ce programme est distribué car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à la
Licence Publique Générale GNU pour plus de détails.

Vous devez avoir reçu une copie de la Licence Publique Générale GNU en même
temps que ce programme ; si ce n'est pas le cas, écrivez à la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
États-Unis.

Contact: Guillaume.Huard@imag.fr
	 Bâtiment IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'Hères
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "registers.h"
#include "util.h"
#include "arm_constants.h"


/* Teste les fonctions de registers.h 
    ATTENTION les tests ne sont pas exhaustifs. Il est impossible de tester tous les cas.
*/
int main(int argc, char** argv) {
    uint32_t donnees = 1;

    // test de registers_create, doit donner un r != NULL si l'allocation réussit
    registers r = registers_create();

    assert(r != NULL);

    // le mode par défaut doit être SYSTEM
    assert(get_mode(r) == SYS);
    // c'est donc un mode privilégié
    assert(in_a_privileged_mode(r) == 1);
    // qui n'a pas de SPSR
    assert(current_mode_has_spsr(r) == 0);

    // on écrit des multiples de 2 dans les registres R0 à R15, et à chaque fois on vérifie que l'on lit la valeur qu'on vient d'écrire
    for (uint8_t reg = 0; reg < 16; reg++) {
        write_register(r, reg, donnees);
        assert(read_register(r, reg) == donnees);
        donnees = donnees*2;
    }

    // lorsque l'on écrit dans CPSR, seuls les bits autorisés doivent être modifiés (ici avec les autorisations d'un mode privilégié)
    write_cpsr(r, 0xFFFFFFFF);
    assert(read_cpsr(r) == 0xF80F03DF);

    // on passe en mode FIQ
    write_cpsr(r, FIQ);
    // dans ce mode un registre banked doit être différent, ici R8_fiq ne doit pas contenir le multiple de 2 écrit dans R8
    assert(read_register(r, 8) == 0);
    write_register(r, 12, 1);
    write_spsr(r, 0xFFFFFFFF);
    // on essaie d'écrire dans SPSR, cela doit fonctionner selon les autorisations d'un mode privilégié
    assert(read_spsr(r) == 0xF80F03DF);

    // on passe en mode Supervisor
    write_cpsr(r, SVC);
    // ce mode partage le même R8 que System, R8 doit donc contenir le multiple de 2 écrit plus haut
    assert(read_register(r, 8) == 256);
    // mais pas R13, car Supervisor possède son propre R13
    assert(read_register(r, 13) == 0);
    // ainsi que son propre SPSR
    assert(read_spsr(r) == 0);

    /* SI VOUS VOULEZ AJOUTER DES TESTS POUR LES MODES AUTRES QUE USER, METTEZ-LES ICI */

    // on écrit les valeurs de R0 à R15
    // amélioration possible : aussi écrire les valeurs des registres banked et des PSR (mais comme à ce moment on est en User, on ne peut plus changer de mode)
    printf("Valeurs brutes de registers à la fin des tests :\n");

    for(int i = 0; i < 16; i++) {
        printf("R%d : 0x%x\n", i, read_usr_register(r, i)); // multiples de 2
    }

    printf("\nCPSR : 0x%x\n", read_cpsr(r)); // 0x00000013

    printf("\nR13_svc : 0x%x\n", read_register(r, 13)); // 0
    printf("R14_svc : 0x%x\n", read_register(r, 14)); // 0
    printf("SPSR_svc : 0x%x\n\n", read_spsr(r)); // 0

    write_cpsr(r, ABT);
    printf("R13_abt : 0x%x\n", read_register(r, 13)); // 0
    printf("R14_abt : 0x%x\n", read_register(r, 14)); // 0
    printf("SPSR_abt : 0x%x\n\n", read_spsr(r)); // 0
    
    write_cpsr(r, UND);
    printf("R13_und : 0x%x\n", read_register(r, 13)); // 0
    printf("R14_und : 0x%x\n", read_register(r, 14)); // 0
    printf("SPSR_und : 0x%x\n\n", read_spsr(r)); // 0
    
    write_cpsr(r, IRQ);
    printf("R13_irq : 0x%x\n", read_register(r, 13)); // 0
    printf("R14_irq : 0x%x\n", read_register(r, 14)); // 0
    printf("SPSR_irq : 0x%x\n\n", read_spsr(r)); // 0
    
    write_cpsr(r, FIQ);
    printf("R8_fiq : 0x%x\n", read_register(r, 8)); // 0
    printf("R9_fiq : 0x%x\n", read_register(r, 9)); // 0
    printf("R10_fiq : 0x%x\n", read_register(r, 10)); // 0
    printf("R11_fiq : 0x%x\n", read_register(r, 11)); // 0
    printf("R12_fiq : 0x%x\n", read_register(r, 12)); // 1
    printf("R13_fiq : 0x%x\n", read_register(r, 13)); // 0
    printf("R14_fiq : 0x%x\n", read_register(r, 14)); // 0
    printf("SPSR_fiq : 0x%x\n\n", read_spsr(r)); // 0xF80F03DF

    /* Les tests du mode USER vont à la fin car une fois dans ce mode, on ne peut plus changer de mode (on ne peut plus écrire dans CPSR[4:0]) */

    // on passe en mode USER
    write_cpsr(r, USR);
    // simple vérification pour savoir si le mode a été bien écrit
    assert(read_cpsr(r) == 0x00000010);
    // R12 de User n'est pas le même que R12 de FIQ. Il ne doit donc pas contenir la valeur qui a été écrite quand on était en FIQ
    assert(read_register(r, 12) != 1);

    // on essaie d'écrire dans CPSR en mode User
    write_cpsr(r, 0xFFFFFFFF);
    // et on constate que les autorisations ne sont pas les mêmes
    assert(read_cpsr(r) == 0xF80F0210);

    printf("Tests réussis.\n");

    // désallocation des registres
    registers_destroy(r);
    // la mémoire a été libérée mais r pointe toujours dessus. Pour faire propre et ne plus pouvoir s'en servir, on met r à NULL
    r = NULL;

    // a winner is you
    return 0;
}
