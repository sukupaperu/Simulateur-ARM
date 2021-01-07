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


uint8_t get_register_from_mode(uint8_t mode) {
	switch(mode) {
		case SVC: return 1; break;
		case ABT: return 2; break;
		case UND: return 3; break;
		case IRQ: return 4; break;
		// à ce moment mode est en FIQ si mode est correct
		default: return 5; break;
	}
}

uint8_t get_banked_register_from_mode(uint8_t mode, uint8_t reg) {
    switch(mode) {
        case FIQ: return reg; break;
        case SVC: return reg % 2; break;
        case ABT: return (reg % 2) + 2; break;
        case UND: return (reg % 2) + 4; break;
        // si on arrive là, on est forcément en IRQ (en supposant que CPSR[4:0] soit un mode valide)
        default: return (reg % 2) + 6; break;
    }
}


registers registers_create() {
    registers r = malloc(sizeof(struct registers_data));

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

    // on démarre les registres en mode System
    r->psr_registers[0] = SYS;
    
    return r;
}


void registers_destroy(registers r) {
    if (r != NULL) {
        free(r->main_registers);
        free(r->banked_registers);
        free(r->psr_registers);
        free(r);
    }
}


uint8_t get_mode(registers r) {
    uint32_t mode = read_cpsr(r);

    return (uint8_t) mode & 0x1F; // masque pour ne garder que les bits [4:0]
} 


int current_mode_has_spsr(registers r) {
    uint8_t current_mode = get_mode(r);

    return current_mode != USR && current_mode != SYS;
}


int in_a_privileged_mode(registers r) {
    uint8_t current_mode = get_mode(r);

    return current_mode != USR;
}


uint32_t read_register(registers r, uint8_t reg) {
    uint32_t value = 0;

    // si le registre est un unbanked ou si on est pas en mode interruption, la lecture est facile
    if (reg < 8 || reg == 15 || !current_mode_has_spsr(r)) {
        value = r->main_registers[reg];

    // tous les cas possibles pour les modes d'interruption
    } else if (reg <= 15) { 
        uint8_t current_mode = get_mode(r);

        // registres communs (R8 � R12) � tous sauf FIQ
        if (current_mode != FIQ && reg < 13) { 
            value = r->main_registers[reg];

        // registres R8 � R14 sp�cifiques � FIQ
        // la partie la plus complexe, les registres 13 et 14 des interruptions
        } else {
            uint8_t banked_register_number = get_banked_register_from_mode(current_mode, reg);
            
            value = r->banked_registers[banked_register_number];
        }
    }

    return value;
}


uint32_t read_usr_register(registers r, uint8_t reg) {
    uint32_t value = (reg <= 15) ? r->main_registers[reg] : 0;

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
        int register_number = get_register_from_mode(current_mode);

        value = r->psr_registers[register_number];
    }

    return value;
}


void write_register(registers r, uint8_t reg, uint32_t value) {

    // si le registre est un unbanked ou si on est pas en mode interruption, l'�criture est facile
    if (reg < 8 || reg == 15 || !current_mode_has_spsr(r)) {
        r->main_registers[reg] = value;

    // tous les cas possibles pour les modes d'interruption
    } else if (reg <= 15) { 
        uint8_t current_mode = get_mode(r);

        // registres communs (R8 � R12) � tous sauf FIQ
        if (current_mode != FIQ && reg < 13) {
            r->main_registers[reg] = value;

        // la partie la plus complexe, les registres 13 et 14 des interruptions
        } else {
            uint8_t banked_register_number = get_banked_register_from_mode(current_mode, reg);
            
            r->banked_registers[banked_register_number] = value;
        }
    }

}


void write_usr_register(registers r, uint8_t reg, uint32_t value) {
    if (reg <= 15)
        r->main_registers[reg] = value;
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
    uint32_t masque = in_a_privileged_mode(r) ? 0x07F0FC20 : 0x7F0FDFF;

    value |= masque;
    r->psr_registers[0] |= ~masque;
    r->psr_registers[0] &= value;
}


// fonction identique � write_cpsr mais qui tient compte du mode courant
void write_spsr(registers r, uint32_t value) {

    if (current_mode_has_spsr(r)) {
        uint8_t current_mode = get_mode(r);
        uint8_t register_number = get_register_from_mode(current_mode);
        uint32_t masque = 0x07F0FC20; // forc�ment un mode privil�gi� si on a SPSR
        
        value |= masque;
        r->psr_registers[register_number] |= ~masque;
        r->psr_registers[register_number] &= value;
    }
}
