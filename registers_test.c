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


/* Teste les fonctions de registers.h 
    ATTENTION les tests ne sont pas exhaustifs. Il est impossible de tester tous les cas.
*/
int main(int argc, char** argv) {
    uint32_t donnees = 1;

    // test de registers_create
    registers r = registers_create();

    assert(r != NULL);

    assert(get_mode(r) == MODE_SYSTEM);
    assert(in_a_privileged_mode(r) == 1);
    assert(current_mode_has_spsr(r) == 0);

    for (uint8_t reg = 0; reg < 16; reg++) {
        write_register(r, reg, donnees);
        assert(read_register(r, reg) == donnees);
        donnees = donnees*2;
    }

    write_cpsr(r, 0xFFFFFFFF);
    assert(read_cpsr(r) == 0xF80F03DF);

    write_cpsr(r, MODE_FIQ);
    assert(read_register(r, 8) == 0);
    write_register(r, 12, 1);
    write_spsr(r, 0xFFFFFFFF);
    assert(read_spsr(r) == 0xF80F03DF);

    write_cpsr(r, MODE_SUPERVISOR);
    assert(read_register(r, 8) == 256);
    assert(read_register(r, 13) == 0);
    assert(read_spsr(r) == 0);


    write_cpsr(r, MODE_USER);
    assert(read_cpsr(r) == 0x00000010);
    assert(read_register(r, 12) != 1);

    write_cpsr(r, 0xFFFFFFFF);
    assert(read_cpsr(r) == 0xF80F0210);

    printf("Tests réussis.\n");

    printf("Valeurs brutes de registers à la fin des tests :\n");

    for(int i = 0; i < 16; i++) {
        printf("R%d : 0x%x\n", i, read_usr_register(r, i));
    }

    registers_destroy(r);
    r = NULL;

    return 0;
}
