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

// La spécification et documentation pour tout ce qui concerne les registres se trouve à la section A2.3 Registers (page 42 du PDF) du Reference Manual

#include "registers.h"
#include "arm_constants.h"
#include <stdlib.h>

struct registers_data {
    // R0 à R15 (16 au total)
    uint32_t* main_registers;
    /* registres spécifiques aux modes d'interruption (15 au total)
        on les range ainsi :
        14_svc, 13_svc, 14_abt, 13_abt, 14_und, 13_und, 14_irq, 13_irq, puis de R8_fiq à R14_fiq
        Cette implémentation a deux avantages :
            - les registres R8_fiq à R14_FIQ ont comme indices 8 à 14,
            - et pour les huits premiers, ils sont demandés à la fonction par le numéro 13 ou 14.
              Les inverser permet de les trouver par modulo (et éviter encore plus de conditions imbriquées, il y en a déjà beaucoup)
    */
    uint32_t* banked_registers;
    // CPSR et SPSR (6 au total), dans l’ordre CPSR, SPSR_svc, SPSR_abt, SPSR_und, SPSR_irq, SPSR_fiq
    uint32_t* psr_registers;
};


registers registers_create() {
    registers r = malloc(sizeof(registers_data));

    if (r != NULL) {
        // rappel : calloc(a, sizeof(b)) équivaut à malloc(a * sizeof(b)) mais le tableau est initialisé à 0
        r->main_registers = calloc(16, sizeof(uint32_t));
        r->banked_registers = calloc(15, sizeof(uint32_t));
        r->psr_registers = calloc(6, sizeof(uint32_t));
    }

    // si l'une des allocations échoue, on vide tout (free n'a pas d'effet si on lui passe une valeur NULL en paramètre, cela permet de factoriser)
    if (r->main_registers == NULL || r->banked_registers == NULL || r->psr_registers == NULL) {
        free(r->main_registers);
        free(r->banked_registers);
        free(r->psr_registers);
        free(r);
        r = NULL;
    }
    
    return r;
}


void registers_destroy(registers r) {
    if (r != NULL) {
        free(r->main_registers);
        free(r->banked_registers);
        free(r->psr_registers);
        free(r);
        r = NULL;
    }
}


uint8_t get_mode(registers r) {
    uint32_t mode = read_cpsr(r);
    mode = (uint8_t) mode & 0x1F; // masque pour ne garder que les bits [4:0]
    return mode;
} 


int current_mode_has_spsr(registers r) {
    uint8_t current_mode = get_mode(r);

    if (current_mode != MODE_USER && current_mode != MODE_SYSTEM) {
        return 1;
    } else {
        return 0;
    }
}


int in_a_privileged_mode(registers r) {
    uint8_t current_mode = get_mode(r);

    if (current_mode != MODE_USER) {
        return 1;
    } else {
        return 0;
    }
}


uint32_t read_register(registers r, uint8_t reg) {
    uint32_t value = 0;

    if (reg >= 0 && reg <= 15) {

        // si le registre est un unbanked ou si on est pas en mode interruption, la lecture est facile
        if (current_mode_has_spsr(r) == 0 || reg < 8 || reg == 15) {

            value = r->main_registers[reg];

        // tous les cas possibles pour les modes d'interruption
        } else {

            uint8_t current_mode = get_mode(r);

            // registres communs (R8 à R12) à tous sauf FIQ
            if (current_mode != MODE_FIQ && reg < 13) {

                value = r->main_registers[reg];

            // registres R8 à R14 spécifiques à FIQ
            } else if (current_mode == MODE_FIQ) {

                value = r->banked_registers[reg];

            // la partie la plus complexe, les registres 13 et 14 des interruptions autres que FIQ
            // ce code peut être factorisé mais aux dépends de la lisibilité
            } else {

                if (current_mode == MODE_SUPERVISOR) {
                    value = r->banked_registers[(reg % 2)];
                } else if (current_mode == MODE_ABORT) {
                    value = r->banked_registers[(reg % 2) + 2];
                } else if (current_mode == MODE_UNDEFINED) {
                    value = r->banked_registers[(reg % 2) + 4];
                // si on arrive là, on est forcément en MODE_IRQ (en supposant que CPSR[4:0] soit un mode valide)
                } else {
                    value = r->banked_registers[(reg % 2) + 6];
                }

            }

        }

    }

    return value;
}


uint32_t read_usr_register(registers r, uint8_t reg) {
    uint32_t value = 0;

    if (reg >= 0 && reg <= 15) {
        value = r->main_registers[reg];
    }

    return value;
}


uint32_t read_cpsr(registers r) {
    uint32_t value = r->psr_registers[0];
    return value;
}


uint32_t read_spsr(registers r) {
    uint32_t value = 0;

    if (current_mode_has_spsr(r)) {
        uint8_t current_mode = get_mode(r);

        if (current_mode == MODE_SUPERVISOR) {
            value = r->psr_registers[1];
        } else if (current_mode == MODE_ABORT) {
            value = r->psr_registers[2];
        } else if (current_mode == MODE_UNDEFINED) {
            value = r->psr_registers[3];
        } else if (current_mode == MODE_IRQ) {
            value = r->psr_registers[4];
        // à ce moment on est en MODE_FIQ si le code du mode est correct
        } else {
            value = r->psr_registers[5];
        }
    }

    return value;
}


void write_register(registers r, uint8_t reg, uint32_t value) {
    if (reg >= 0 && reg <=15) {

        // si le registre est un unbanked ou si on est pas en mode interruption, l'écriture est facile
        if (current_mode_has_spsr(r) == 0 || reg < 8 || reg == 15) {

            r->main_registers[reg] = value;

        // tous les cas possibles pour les modes d'interruption
        } else {

            uint8_t current_mode = get_mode(r);

            // registres communs (R8 à R12) à tous sauf FIQ
            if (current_mode != MODE_FIQ && reg < 13) {

                r->main_registers[reg] = value;

            // registres R8 à R14 spécifiques à FIQ
            } else if (current_mode == MODE_FIQ) {

                r->banked_registers[reg] = value;

            // la partie la plus complexe, les registres 13 et 14 des interruptions autres que FIQ
            // ce code peut être factorisé mais aux dépends de la lisibilité
            } else {

                if (current_mode == MODE_SUPERVISOR) {
                    r->banked_registers[(reg % 2)] = value;
                } else if (current_mode == MODE_ABORT) {
                    r->banked_registers[(reg % 2) + 2] = value;
                } else if (current_mode == MODE_UNDEFINED) {
                    r->banked_registers[(reg % 2) + 4] = value;
                // si on arrive là, on est forcément en MODE_IRQ (en supposant que CPSR[4:0] soit un mode valide)
                } else {
                    r->banked_registers[(reg % 2) + 6] = value;
                }

            }

        }

    }
}


void write_usr_register(registers r, uint8_t reg, uint32_t value) {
    if (reg >= 0 && reg <=15) {
        r->main_registers[reg] = value;
    }
}


/* Les masques ça rend maboul
    Explication en prenant un octet comme exemple
    On a value = XXXXXXXX et CPSR = ABBAAABB
    Où X est un bit à écrire, A un bit qui peut être écrit et B un bit qui ne doit pas être changé
    On veut donc obtenir CPSR = XBBXXXBB
    Pour chaque bit B on applique (X | 1) & B = B
    Pour chaque bit A on applique X & (A | 1) = X
    Car X | 1 = 1 et X & 1 = X
    Il faut donc un masque qui sépare A et B
    On prend un masque où les bits à ne pas changer sont à 1 : masque = 01100011
    On fait :
        value | masque soit X | 1
        CPSR | ~masque soit A | 1
        Puis CPSR & value qui fait la partie & de nos calculs
        On obtient donc CPSR = XBBXXXBB
*/
void write_cpsr(registers r, uint32_t value) {
    uint32_t masque;

    if (in_a_privileged_mode(r)) {
        masque = 0x07F0FC20;
    } else {
        masque = 0x7F0FDFF;
    }

    value = value | masque;
    r->psr_registers[0] = r->psr_registers[0] | ~masque;
    r->psr_registers[0] = r->psr_registers[0] & value;
}


// fonction identique à write_cpsr mais qui tient compte du mode courant
void write_spsr(registers r, uint32_t value) {
    if (current_mode_has_spsr(r)) {

        uint8_t current_mode = get_mode(r);
        uint32_t masque = 0x07F0FC20; // forcément un mode privilégié si on a SPSR
        value = value | masque;
        int n = 1;

        if (current_mode == MODE_SUPERVISOR) {
            n = 1;
        } else if (current_mode == MODE_ABORT) {
            n = 2;
        } else if (current_mode == MODE_UNDEFINED) {
            n = 3;
        } else if (current_mode == MODE_IRQ) {
            n = 4;
        // à ce moment on est en MODE_FIQ si current_mode est correct
        } else {
            n = 5;
        }

        r->psr_registers[n] = r->psr_registers[n] | ~masque;
        r->psr_registers[n] = r->psr_registers[n] & value;

    }
}
