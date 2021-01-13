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
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include <debug.h>
#include <stdlib.h>


int arm_branch(arm_core p, uint32_t ins) {
    /*
        On isole les parties de l'instruction qui nous intéressent pour la suite du décodage.
    */
    // vaut 1 s'il s'agit bien d'une instruction de branchement
    /* int is_branch_instruction = get_bits(ins, 27, 25) == 0b101;
    if(!is_branch_instruction) return UNDEFINED_INSTRUCTION; */

    // sur 23bits (24 - signe de bit), l'adresse immédiate de l'instruction sur laquelle on veut effectuer le branchement
    uint32_t signed_immed_23 = get_bits(ins, 22, 0);
    // sur 1 bit, le bit de signe (bit 24) de la valeur immédiate signed_immed_23
    int sign_bit = get_bit(ins, 23);
    // vaut 1 si l'adresse de retour doit être stockée dans r14 (registre LR)
    int store_return_address = get_bit(ins, 24);

    uint32_t pc_value = arm_read_register(p, 15);

    // si instruction BL, stockage de l'adresse de retour dans LR (r14)
    if(store_return_address)
        // "pc_value - 4"  car la valeur de PC (r15) pointe toujours 2 instructions plus loin 
        // (soit adresse_instruction_courante + 8, 
        // d'où le "-4" pour avancer d'une instruction seulement)
        arm_write_register(p, 14, pc_value - 4);
    
    // ajout et extension du bit de signe (s'il est égal à 1) pour passer la valeur de 24 à 30 bits
    if(sign_bit)
        signed_immed_23 |= 0b1111111 << 23;
    // on passe la valeur sur 32 bits
    signed_immed_23 <<= 2;

    // on ré-interprête la valeur obtenue en bit signé
    int32_t new_signed_immed_32 = signed_immed_23;

    // branchement par ré-écriture du registre PC
    arm_write_register(p, 15, pc_value + new_signed_immed_32);

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
        && get_bits(ins, 21, 20) == 0b00;

    if(is_mrs_instruction) {
        // sur 4 bits, registre de destination pour l'instruction MRS
        uint8_t destination_register = get_bits(ins, 15, 12);
        // vaut  1 si c'est le registre SPSR qui doit être copié, (registre CPSR dans le cas contraire)
        int is_spsr_move = get_bit(ins, 22);
        // valeur du registre CPSR ou SPSR
        uint32_t psr_value;

        if(is_spsr_move)
            psr_value = arm_read_spsr(p);
        else
            psr_value = arm_read_cpsr(p);
        // if(destination_register == 15) -> UNPREDICTABLE
        arm_write_register(p, destination_register, psr_value);
    } else { // se produit si l'instruction n'est pas connue
        return UNDEFINED_INSTRUCTION;
    }

    return 0;
}
