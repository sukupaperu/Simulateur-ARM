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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"


static int arm_execute_instruction(arm_core p) {
    int res = 0;
    uint32_t opcode;
    //uint32_t *tmp;
    res = arm_fetch(p,&opcode);

    if(inst_cond( p, opcode)){
        
        //------arm_load_store 
        /* LDRH et STRH : 
         *  opcode[27:25] == 0b000
         *  opcode[7:4] == 1011
         */
        if(get_bits(opcode, 27,25)==0 && get_bits(opcode, 7,4)==0b1011 ){
            res=arm_load_store( p, opcode);
        }
        
        /* LDR, LDRB, STR et STRB : 
        * opcode[27:25] == 0b010 
        * or opcode[27:25] == 0b011 , opcode[4] == 0
        */
        else if((get_bits(opcode, 27,25)==0b010) || (get_bits(opcode, 27,25)==0b011 && get_bit(opcode, 4)==0)){
            res=arm_load_store( p, opcode);
        }
        /*LDM et STM :
        * opcode[27:25] == 0b100
        */
        else if(get_bits(opcode, 27,25)==0b100){
            res=arm_load_store_multiple( p, opcode);
        }

        
        // ------arm_branch_other
        /*
        * B/BL:  opcode[27:25] == 0b101
        */
        else if(get_bits(opcode, 27,25)==0b101){
            res=arm_branch(p, opcode);
        }
        /*
        * MRS
        * opcode[27:23] == 0b00010
        * opcode[21:20] == 0b00
        */
        else if((get_bits(opcode, 27,23)==0b00010) && (get_bits(opcode, 21,20)==0b00)){
            res=arm_miscellaneous(p, opcode);
        }
        /* 
         * SWI:
         * opcode[27:24] == 0b1111
         */
        else if(get_bits(opcode, 27,24)==0b1111){
            res=arm_coprocessor_others_swi(p, opcode);
        }

        //------arm_data_processing
        /*
        * AND, EOR, SUB, RSB, ADD, ADC, SBC, RSC, 
        * TST, TEQ, CMP, CMN, ORR, MOV, BIC, MVN
        * opcode[27:26] == 0b00
        * opcode[24:21] = [0000,1111]
        * 
        * STR, LDR, STRB, LDRB :
        * opcode[27:26] == 0b00
        * opcode[22], [20] = [00,11]
        */
        else if(get_bits(opcode, 27,26)==0b00){
            res=arm_data_processing_shift(p, opcode);
        }
        
        /*
        * LDM, STM
        * opcode[27:25] == 0b100
        */
        else if(get_bits(opcode, 27,25)==0b000){
            res=arm_data_processing_shift(p, opcode);
        }
        

        

    }
    return res;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}


int inst_cond(arm_core p, uint32_t ins) {
    uint32_t flags = arm_read_cpsr(p);
    /*uint8_t reg;
    arm_read_register( p, reg);
    uint32_t flags = read_cpsr(reg);*/

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

        // instruction inconditionnelle (aucune du genre n'est impl�ment�e ici, le cas est trait� comme AL)
        case 0xF:
            return 1;
            break;
    }

    return 0;
}