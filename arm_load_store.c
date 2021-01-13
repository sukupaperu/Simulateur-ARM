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
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"

int arm_load_store(arm_core p, uint32_t ins) {
    /*
        On isole les parties de l'instruction qui nous intéressent pour la suite du décodage.
    */

    // Rd, sur 4 bits, registre de destination ou source (en fonction d'un accès en écriture ou en lecture)
    uint8_t rd_register = get_bits(ins, 15, 12);
    // Rn, sur 4 bits, registre contenant l'adresse mémoire à laquelle aura lieu l'accès en lecture/écriture
    uint8_t rn_register = get_bits(ins, 19, 16);
    // vaut 1 si l'offset doit être ajouté à l'adresse (l'offset devra être soustrait dans le cas contraire)
    int is_offset_added = get_bit(ins, 23);
    // vaut 1 si on est en mode "pre-indexed addressing" ou "offset addressing"
    int is_pre_indexed_or_offset_addressing_mode = get_bit(ins, 24);
    // vaut 1, si la mise à jour du contenu du registre contenant l'adresse mémoire est activée
    int is_writeback_mode = get_bit(ins, 21);
    // bit l (usage variable)
    int is_l_bit_set = get_bit(ins, 20);

    // valeur de décalage à appliquer (addition ou soustraction) à l'adresse mémoire 
    // à laquelle aura lieu l'accès en lecture/écriture
    uint32_t offset;
    // adresse de base (adresse mémoire à laquelle aura lieu l'accès en lecture/écriture)
    uint32_t address = arm_read_register(p, rn_register);

    // vaut 1 si on l'accès mémoire concerne un mot (32bits) ou bien un octet non signé
    int is_word_or_unsigned_byte_access = get_bits(ins, 27, 26) == 0b01;

    if(is_word_or_unsigned_byte_access) {
        // vaut 1 si la valeur de l'offset est stockée dans un registre (valeur immédiate dans le cas contraire)
        int is_register_offset = get_bit(ins, 25);
        // vaut 1 si l'accès mémoire concerne un octet non signé (mot de 32 bits dans le cas contraire)
        int is_unsigned_byte_access = get_bit(ins, 22);
        // vaut 1 l'accès mémoire est un accès en lecture (accès en écriture dans le cas contraire)
        int is_load_instruction = is_l_bit_set;

        if(is_register_offset) {
            // Rm, sur 4 bits, registre contenant la valeur de initiale l'offset
            uint8_t rm_register = get_bits(ins, 3, 0);
            // valeur initiale de l'offset
            // if(rm_register == 15) -> UNPREDICTABLE
            // if(rn_register == rm_register) -> UNPREDICTABLE
            uint32_t rm_register_value = arm_read_register(p, rm_register);
            // sur 3 bits, code de l'opérateur de décalage à appliquer à la valeur d'offset
            uint8_t shift_operator = get_bits(ins, 6, 5);
            // sur 5 bits, valeur du décalage appliqué à la valeur d'offset
            uint8_t shift_immediate_value = get_bits(ins, 11, 7);
            // vaut 1 si l'instruction est une instruction de type "média" (devrait valoir 0 s'il s'agit bien d'une instruction d'accès mémoire)
            int is_media_instruction = get_bit(ins, 4);

            if(is_media_instruction) // se produit si l'instruction n'est pas une instruction d'accès mémoire (instruction "media")
                return UNDEFINED_INSTRUCTION;
            
            switch(shift_operator) {
                case LSL: // décalage logique vers la gauche
                    offset = rm_register_value << shift_immediate_value;
                    break;
                case LSR: // décalage logique vers la droite
                    offset = (shift_immediate_value == 0) ? 0 : (rm_register_value >> shift_immediate_value);
                    break;
                case ASR: // décalage arithmétique vers la droite
                    offset = asr(rm_register_value, shift_immediate_value);
                    break;
                case ROR: { // rotation vers la droite
                    // cas spécial : si la valeur de rotation vaut 0, on applique une rotation étendue de 1 vers la droite (RRX)
                    if(shift_immediate_value == 0) {
                        // bit de retenue sortante
                        int carry_flag = get_bit(arm_read_cpsr(p), C);

                        offset = ((carry_flag << 31) | (rm_register_value >> 1));
                    } else {
                        offset = ror(rm_register_value, shift_immediate_value); 
                    }
                    break;
                }
                default: // se produit si shift_operator ne correspond à aucune instruction de décalage connue
                    return UNDEFINED_INSTRUCTION;
            }
        } else {
            // valeur immédiate de l'offset à appliquer à l'adresse
            uint32_t offset_12 = get_bits(ins, 11, 0);

            offset = offset_12;
        }

        if(is_pre_indexed_or_offset_addressing_mode) {
            // on applique l'offset à l'adresse mémoire
            address += is_offset_added ? offset : -offset;
            if(is_writeback_mode)
                // on réécrit au registre associé la nouvelle adresse par dessus l'ancienne
                // if(rn_register == 15) -> UNPREDICTABLE
                arm_write_register(p, rn_register, address);
        }

        if(is_load_instruction) { // instruction de lecture
            uint8_t destination_register = rd_register;

            if(is_unsigned_byte_access) { // instruction LDRB (unsigned byte)
                uint8_t value;

                int access_result = arm_read_byte(p, address, &value);
                if(access_result == -1) // illegal memory access
                    return DATA_ABORT;
                arm_write_register(p, destination_register, value);
            } else { // instruction LDR (word)
                uint32_t value;

                int access_result = arm_read_word(p, address, &value);
                if(access_result == -1) // illegal memory access
                    return DATA_ABORT;
                arm_write_register(p, destination_register, value);
            }
                
        } else { // instruction d'écriture
            uint8_t source_register = rd_register;
            uint32_t value = arm_read_register(p, source_register);
            int access_result;

            if(is_unsigned_byte_access) // instruction STRB (byte) 
                access_result = arm_write_byte(p, address, value);
            else // instruction STR (word)
                access_result = arm_write_word(p, address, value);

            if(access_result == -1) // illegal memory access
                    return DATA_ABORT;
        }

        // si on est en mode "post-indexed addressing"
        if(!is_pre_indexed_or_offset_addressing_mode) {
            if(is_writeback_mode) // se produit si l'instruction est LDRBT, LDRT, STRBT ou STRT (non implémentées)
                return UNDEFINED_INSTRUCTION;
            // on applique l'offset à l'adresse mémoire
            address += is_offset_added ? offset : -offset;
            // on réécrit au registre associé la nouvelle adresse par dessus l'ancienne
            // if(rn_register == 15) -> UNPREDICTABLE
            arm_write_register(p, rn_register, address);
        }
    } else {
        // vaut 1 si on l'accès mémoire concerne un demi-mot (16bits) ou bien un octet signé
        int is_halfword_or_signed_byte_access = (
                get_bits(ins, 27, 25) == 0b000 && 
                get_bit(ins, 7) == 0b1 && 
                get_bit(ins, 4) == 0b1 && 
                get_bits(ins, 31, 28) != 0b1111
            ) && !(
                !is_pre_indexed_or_offset_addressing_mode && 
                get_bits(ins, 6, 5) == 0b00
            );

        if(is_halfword_or_signed_byte_access) {
            // vaut 1 si la valeur de l'offset est une valeur immédiate (la valeur de l'offset est stockée dans un registre dans le cas contraire)
            int is_immediate_offset = get_bit(ins, 22);
            // sur 2 bits, valeur des bits s et h
            uint8_t bits_sh = get_bits(ins, 6, 5);
            // sur 3 bits, valeur des bits l, s et h (code le type d'accès mémoire qui sera effectué)
            uint8_t access_type = (is_l_bit_set << 2) | bits_sh; // LSH

            if(is_immediate_offset) {
                // sur 4 bits, partie "haute" de la valeur immédiate de l'offset
                uint8_t immedH = get_bits(ins, 11, 8);
                // sur 4 bits, partie "basse" de la valeur immédiate de l'offset
                uint8_t immedL = get_bits(ins, 3, 0);
                
                offset = (immedH << 4) | immedL;
            } else {
                // Rm, sur 4 bits, registre contenant la valeur de l'offset à appliquer à l'adresse
                uint8_t rm_register = get_bits(ins, 3, 0);
                // valeur de l'offset à appliquer à l'adresse
                // if(rm_register == 15) -> UNPREDICTABLE
                // if(rn_register == rm_register) -> UNPREDICTABLE
                uint32_t rm_register_value = arm_read_register(p, rm_register);

                offset = rm_register_value;
            }

            if(is_pre_indexed_or_offset_addressing_mode) {
                // on applique l'offset à l'adresse mémoire
                address += is_offset_added ? offset : -offset;
                if(is_writeback_mode)
                    // on réécrit au registre associé la nouvelle adresse par dessus l'ancienne
                    // if(rn_register == 15) -> UNPREDICTABLE
                    arm_write_register(p, rn_register, address);
            }

            switch(access_type) {
                case 1: { // instruction STRH (halfword)
                    uint32_t source_register = rd_register;
                    uint16_t value = arm_read_register(p, source_register);

                    int access_result = arm_write_half(p, address, value);
                    if(access_result == -1) // illegal memory access
                        return DATA_ABORT;
                    break;
                }
                case 5: { // instruction LDRH (unsigned halfword)
                    uint8_t destination_register = rd_register;
                    uint16_t value;
                    
                    int access_result =  arm_read_half(p, address, &value);
                    if(access_result == -1) // illegal memory access
                        return DATA_ABORT;
                    arm_write_register(p, destination_register, value);
                    break;
                }
                case 7: // instruction LDRSH (signed halfword)
                case 6: // instruction LDSRB (signed byte)
                default: // se produit si l'instruction n'est pas reconnue comme une instruction d'accès mémoire connue
                    return UNDEFINED_INSTRUCTION;
            }

            // si on est en mode "post-indexed addressing"
            if(!is_pre_indexed_or_offset_addressing_mode) {
                // if(is_writeback_mode) -> UNPREDICTABLE
                // on applique l'offset à l'adresse mémoire
                address += is_offset_added ? offset : -offset;
                // on réécrit au registre associé la nouvelle adresse par dessus l'ancienne
                // if(rn_register == 15) -> UNPREDICTABLE
                arm_write_register(p, rn_register, address);
            }
        } else { // se produit si l'instruction n'est pas reconnue comme une instruction d'accès mémoire
            return UNDEFINED_INSTRUCTION;
        }
    }

    return 0;
}

int arm_load_store_multiple(arm_core p, uint32_t ins) {
    /*
        On isole les parties de l'instruction qui nous intéressent pour la suite du décodage.
    */
    
    // Rn, sur 4 bits, registre contenant l'adresse mémoire à partir de laquelle aura lieu les accès en lecture/écriture
    uint8_t rn_register = get_bits(ins, 19, 16);
    // sur 16 bits, liste des registres qui vont être transférés (bit 0 = r0, bit 1 = r1 ... bit 15 = r15)
    uint16_t register_list = get_bits(ins, 15, 0);
    // vaut 1 si l'accès mémoire est un accès en lecture (accès en écriture dans le cas contraire)
    int is_load_instruction = get_bit(ins, 20);
    // bit s
    int is_s_bit_set = get_bit(ins, 22);
    // bit p
    int is_p_bit_set = get_bit(ins, 24);
    // bit u, vaut 1 si le transfert est ascendant par rapport à l'adresse de base (descendant dans le cas contraire)
    int is_upwards_transfert = get_bit(ins, 23);
    // bit w, vaut 1 si la mise à jour du contenu du registre contenant l'adresse mémoire est activée
    int is_writeback_mode = get_bit(ins, 21);

    // l'adresse mémoire à partir de laquelle aura lieu les accès en lecture/écriture
    uint32_t address = arm_read_register(p, rn_register);
    if(is_p_bit_set) // adressage pré-indexé
        address += is_upwards_transfert ? 4 : -4;

    if(!is_s_bit_set) { // prérequis pour les instructions LDM(1) et STM(1)
        if(is_load_instruction) { // instruction LDM(1)
            
            for(int r = 0; r <= 14; r++) {
                if(get_bit(register_list, r) == 1) {
                    uint32_t value;

                    int access_result = arm_read_word(p, address, &value);
                    if(access_result == -1) // illegal memory access
                        return DATA_ABORT;
                    address += is_upwards_transfert ? 4 : -4;

                    arm_write_register(p, r, value);
                }
            }

            if(get_bit(register_list, 15) == 1) {
                uint32_t value;

                int access_result = arm_read_word(p, address, &value);
                if(access_result == -1) // illegal memory access
                    return DATA_ABORT;
                address += is_upwards_transfert ? 4 : -4;
                
                arm_write_register(p, 15, value & 0xfffffffe);
            }

        } else { // instruction STM(1)

            for(int r = 0; r <= 15; r++) {
                if(get_bit(register_list, r) == 1) {
                    uint32_t value = arm_read_register(p, r);

                    int access_result = arm_write_word(p, address, value);
                    if(access_result == -1) // illegal memory access
                        return DATA_ABORT;
                    address += is_upwards_transfert ? 4 : -4;
                }
            }

        }
    } else { // se produit si l'instruction n'est pas reconnue comme une instruction d'accès mémoire connue (STM(1) ou LDM(1) seulement)
        return UNDEFINED_INSTRUCTION;
    }

    if(is_writeback_mode) {
        if(is_p_bit_set) // si on est en adressage pré-indexé l'adresse a alors un pas de trop
            address -= is_upwards_transfert ? 4 : -4;
        arm_write_register(p, rn_register, address);
    }

    return 0;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Non implémenté */
    return UNDEFINED_INSTRUCTION;
}