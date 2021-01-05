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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"

static int arm_execute_instruction(arm_core p) {
    return 0;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}


int inst_cond(arm_core p, uint32_t ins) {
    uint32_t flags = read_cpsr(p->reg);

    uint32_t flag_N = (flags >> N) & 1;
    uint32_t flag_Z = (flags >> Z) & 1;
    uint32_t flag_C = (flags >> C) & 1;
    uint32_t flag_V = (flags >> V) & 1;

    uint32_t cond = ins >> 28;

    switch(cond) {
        // EQ equal
        case 0x0:
            return flag_Z == 1;
            break;
        
        // NE not equal
        case 0x1:
            return flag_Z == 0;
            break;

        // CS/HS carry set/unsigned higher or same
        case 0x2:
            return flag_C == 1;
            break;

        // CC/LO carry clear/usigned lower
        case 0x3:
            return flag_C == 0;
            break;

        // MI minus/negative
        case 0x4:
            return flag_N == 1;
            break;

        // PL plus/positive or zero
        case 0x5:
            return flag_N == 0;
            break;

        // VS overflow
        case 0x6:
            return flag_V == 1;
            break;

        // VC no overflow
        case 0x7:
            return flag_V == 0;
            break;

        // HI unsigned higher
        case 0x8:
            return (flag_C == 1) && (flag_Z == 0);
            break;

        // LS unsigned lower or same
        case 0x9:
            return (flag_C == 0) || (flag_Z == 1);
            break;

        // GE signed greater than or equal
        case 0xA:
            return flag_N == flag_V;
            break;

        // LT signed less than
        case 0xB:
            return flag_N != flag_V;
            break;

        // GT signed greater than
        case 0xC:
            return (flag_Z == 0) && (flag_N == flag_V);
            break;

        // LE signed less than or equal
        case 0xD:
            return (flag_Z == 1) || (flag_N != flag_V);
            break;

        // AL always
        case 0xE:
            return 1;
            break;

        // instruction inconditionnelle (aucune du genre n'est implémentée ici, le cas est traité comme AL)
        case 0xF:
        return 1;
            break;
    }    
}