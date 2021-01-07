/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include <debug.h>
#include <stdlib.h>


int arm_branch(arm_core p, uint32_t ins) {
    /*
        On isole les parties de l'instruction qui nous intéressent pour la suite du décodage.
    */

    // sur 24bits, l'adresse immédiate de l'instruction sur laquelle on veut effectuer le branchement
    uint32_t signed_immed_24 = get_bits(ins, 23, 0);
    // vaut 1 si l'adresse de retour doit être stockée dans r14 (registre LR)
    int store_return_address = get_bit(ins, 24);
    // vaut 1 s'il s'agit bien d'une instruction de branchement (#TODO : à supprimer par la suite)
    int is_branch_instruction = get_bits(ins, 27, 25) == 0b101;

    if(is_branch_instruction) {
        uint32_t pc_value = arm_read_register(p, 15);
        if(store_return_address)
            arm_write_register(p, 14, pc_value);
        
        // extension du bit de signe pour passer la valeur de 24 à 30 bits
        int sign_bit = get_bit(signed_immed_24, 23);
        if(sign_bit) {
            signed_immed_24 = clr_bit(signed_immed_24, 23);
            signed_immed_24 = set_bit(signed_immed_24, 29);
        }

        arm_write_register(p, 15, pc_value + (signed_immed_24 << 2));
    } else { // se produit si l'instruction n'est pas reconnue comme étant une instruction de branchement
        return UNDEFINED_INSTRUCTION;
    }

    return 0;
}

int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {
    if (get_bit(ins, 24)) {
        /* Here we implement the end of the simulation as swi 0x123456 */
        if ((ins & 0xFFFFFF) == 0x123456)
            exit(0);
        return SOFTWARE_INTERRUPT;
    } 
    return UNDEFINED_INSTRUCTION;
}

int arm_miscellaneous(arm_core p, uint32_t ins) {
    /*
        On isole les parties de l'instruction qui nous intéressent pour la suite du décodage.
    */

    // vaut 1 si l'instruction est bien une instruction MRS
    int is_mrs_instruction = 
        get_bits(ins, 11, 0) == 0
        && get_bits(ins, 19, 16) == 0b1111
        && get_bits(ins, 21, 20) == 0b00
        && get_bits(ins, 27, 23) == 0b00010;

    if(is_mrs_instruction) {
        // sur 4 bits, registre de destination pour l'instruction MRS
        uint8_t destination_register = get_bits(ins, 15, 12);
        // vaut  1 si c'est le registre SPSR qui doit être copié, (registre CPSR dans le cas contraire)
        int is_spsr_move = get_bit(ins, 22);
        // valeur du registre CPSR ou SPSR
        uint32_t psr_value;

        if(is_spsr_move)
            psr_value = arm_read_register(p, 17);
        else
            psr_value = arm_read_register(p, 16);
        // if(destination_register == 15) -> UNPREDICTABLE
        arm_write_register(p, destination_register, psr_value);
    } else { // se produit si l'instruction n'est pas connue
        return UNDEFINED_INSTRUCTION;
    }

    return 0;
}
