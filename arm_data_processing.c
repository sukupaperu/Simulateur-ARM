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
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"

/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
	/* 
		on va isoler chaque composante intéressante de l'instruction (page 116 de la spécification)
		toutes ces valeurs ne sont pas utiles pour toutes les instructions, mais cela simplifie le code
		cette fonction peut être largement factorisée et simplifiée, mais pour le moment on se concentre
		sur la fonctionnalité
	*/

	// 4 bits qui correspondent au code de l'instruction
	uint32_t ins_opcode = (ins >> 21) & 0xf;

	// bit I (indique si la shifter operand est un registre ou une valeur immédiate)
	uint32_t ins_I = get_bit(ins, 25);

	// bit S (indique si l'instruction doit mettre à jour les flags NZCV)
	uint32_t ins_S = get_bit(ins, 20);
	uint32_t flag_N = 0;
	uint32_t flag_Z = 0;
	uint32_t flag_V = 0;
	uint32_t flag_C = 0;
	// Rn, sur 4 bits, représente le registre source (première opérande)
	uint32_t ins_rn = (uint8_t) (ins >> 16) & 0xf;

	// Rd, sur 4 bits, représente le registre où stocker le résultat de l'opération
	uint32_t ins_rd = (uint8_t) (ins >> 12) & 0xf;

	// un carry, utilisé par le shifter
	char shifter_carry_out = (char) get_bit(arm_read_cpsr(p), 29);

	// shifter operand, sur 12 bits, représente soit une valeur immédiate soit un registre, sert de seconde opérande
	// 1 valeur immédiate
	uint32_t ins_shifter;
	if (ins_I == 1) {
		uint8_t rotation = ((ins >> 8) && 0xf) * 2;
		ins_shifter = ror(ins & 0xff,rotation);

		if ((ins >> 8) && 0xf != 0) {
			shifter_carry_out = get_bit(ins_shifter, 31);
		}

	// valeur venant d'un registre, ici 10 cas possibles
	} else {
		uint8_t shifter_code = (uint8_t) (ins >> 4) & 0xff;
		uint8_t shifter_value = shifter_code >> 3;
		uint32_t rm = arm_read_register(p, ins & 0xf);
		uint32_t rs = 0;

		// 2 registre
		if (shifter_code == 0x00) {
			ins_shifter = rm;

		// 3 logical shift left by immediate
		} else if ((shifter_code & 0x7) == 0x0) {
			ins_shifter = rm << shifter_value;
			if (shifter_value > 0) {
				shifter_carry_out = get_bit(rm, 32 - shifter_value);
			}

		// 4 logical shift left by register
		} else if ((shifter_code & 0xf) == 1) {
			rs = arm_read_register(p, shifter_value >> 1) & 0xff;
			if (rs == 0) {
				ins_shifter = rm;
			} else if (rs < 32) {
				ins_shifter = rm << rs;
				shifter_carry_out = get_bit(rm, 32 - rs);
			} else if (rs == 32) {
				ins_shifter = 0;
				shifter_carry_out = rm & 1;
			} else {
				ins_shifter = 0;
				shifter_carry_out = 0;
			}
		
		// 5 logical shift right by immediate
		} else if ((shifter_code & 0x7) == 0x2) {
			if (shifter_value == 0) {
				ins_shifter = 0;
				shifter_carry_out = get_bit(rm, 31);
			} else {
				ins_shifter = rm >> shifter_value;
				shifter_carry_out = get_bit(rm, shifter_value - 1);
			}

		// 6 logical shift right by register
		} else if ((shifter_code & 0xf) == 0x3) {
			rs = arm_read_register(p, shifter_value >> 1) & 0xff;
			if (rs == 0) {
				ins_shifter = rm;
			} else if (rs < 32) {
				ins_shifter = rm >> rs;
				shifter_carry_out = get_bit(rm, rs - 1);
			} else if (rs == 32) {
				ins_shifter = 0;
				shifter_carry_out = get_bit(rm, 31);
			} else {
				ins_shifter = 0;
				shifter_carry_out = 0;
			}

		// 7 arithmetic shift right by immediate
		} else if ((shifter_code & 0x7) == 0x4) {
			if (shifter_value == 0) {
				if (get_bit(rm, 31) == 0) {
					ins_shifter = 0;
					shifter_carry_out = 0;
				} else {
					ins_shifter = 0xffffffff;
					shifter_carry_out = 1;
				}
			} else {
				ins_shifter = asr(rm, shifter_value);
				shifter_carry_out = get_bit(rm, shifter_value - 1);
			}

		// 8 arithmetic shift right by register
		} else if ((shifter_code & 0xf) == 0x5) {
			rs = arm_read_register(p, shifter_value >> 1) & 0xff;
			if (rs == 0) {
				ins_shifter = rm;
			} else if (rs < 32) {
				ins_shifter = asr(rm, rs);
				shifter_carry_out = get_bit(rm, rs - 1);
			} else {
				if (get_bit(rm, 31) == 0) {
					ins_shifter = 0;
					shifter_carry_out = 0;
				} else {
					ins_shifter = 0xffffffff;
					shifter_carry_out = 1;
				}
			}

		// 9 rotate right by immediate
		} else if ((shifter_code & 0x7) == 0x6) {
			if (shifter_value == 0) {
				// todo rrx
			} else {
				ins_shifter = asr(rm, shifter_value);
				shifter_carry_out = get_bit(rm, shifter_value - 1);
			}

		// 10 rotate right by register
		} else if ((shifter_code & 0xf) == 0x7) {
			rs = arm_read_register(p, shifter_value >> 1) & 0xff;
			if (rs == 0) {
				ins_shifter = rm;
			} else if ((rs & 0xf) == 0) {
				ins_shifter = rm;
				shifter_carry_out = get_bit(rm, 31);
			} else {
				ins_shifter = asr(rm, rs & 0xf);
				shifter_carry_out = get_bit(rm, (rs & 0xf) - 1);
			}

		// 11 rotate right with extend
		} else if (shifter_code == 0x06) {
			ins_shifter = (shifter_carry_out << 31) | (rm >> 1);
			shifter_carry_out = get_bit(rm, 0);

		// tous les autres cas n'appartiennent pas aux instructions de traitement des données, on ne devrait pas être ici
		} else {
			return UNDEFINED_INSTRUCTION;
		}
	}

	switch(ins_opcode) {


		// AND et logique (page 158)
		case 0x0:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) & ins_shifter);

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				flag_C = shifter_carry_out << C;
				// V is unaffected

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;


		// EOR ou exclusif logique page 182
		case 0x1:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) ^ ins_shifter);

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				flag_C = shifter_carry_out << C;
				// V is unaffected

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;


		// SUB soustraction page 358
		case 0x2:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) - ins_shifter);

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				flag_C = (ins_shifter > arm_read_register(p, ins_rn) ? 0 : 1) << C;
				// flag_V = OverflowFrom(arm_read_register(p, ins_rn) - ins_shifter) TODO

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
			

		// RSB soustraction avec inversion des opérandes page 265
		case 0x3:
			arm_write_register(p, ins_rd, ins_shifter - arm_read_register(p, ins_rn));

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				flag_C = (arm_read_register(p, ins_rn) > ins_shifter ? 0 : 1) << C;
				// flag_V = OverflowFrom(ins_shifter - arm_read_register(p, ins_rn)) TODO

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
			

		// ADD addition page 156
		case 0x4:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) + ins_shifter);

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				// flag_C = CarryFrom(arm_read_register(p, ins_rn) + ins_shifter) TODO
				// flag_V = OverflowFrom(arm_read_register(p, ins_rn) + ins_shifter) TODO

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
			

		// ADC addition avec carry page 154
		case 0x5:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) + ins_shifter + get_bit(arm_read_cpsr(p), 29));

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				// flag_C = CarryFrom(arm_read_register(p, ins_rn) + ins_shifter + get_bit(arm_read_cpsr(p), 29)) TODO
				// flag_V = OverflowFrom(arm_read_register(p, ins_rn) + ins_shifter + get_bit(arm_read_cpsr(p), 29)) TODO

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
			

		// SBC soustraction avec carry page 275
		case 0x6:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) - ins_shifter - ~get_bit(arm_read_cpsr(p), 29));

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				// flag_C = CarryFrom(arm_read_register(p, ins_rn) - ins_shifter - ~get_bit(arm_read_cpsr(p), 29)) TODO
				// flag_V = OverflowFrom(arm_read_register(p, ins_rn) - ins_shifter - ~get_bit(arm_read_cpsr(p), 29)) TODO

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
			

		// RSC soustraction avec carry et opérandes inversées page 267
		case 0x7:
			arm_write_register(p, ins_rd, ins_shifter - arm_read_register(p, ins_rn) - ~get_bit(arm_read_cpsr(p), 29));

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				// flag_C = CarryFrom(ins_shifter - arm_read_register(p, ins_rn) - ~get_bit(arm_read_cpsr(p), 29)) TODO
				// flag_V = OverflowFrom(ins_shifter - arm_read_register(p, ins_rn) - ~get_bit(arm_read_cpsr(p), 29)) TODO

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
			

		// TST test de deux valeurs page 380
		case 0x8: {
			uint32_t alu_out = arm_read_register(p, ins_rn) & ins_shifter;
			
			flag_N = get_bit(alu_out, 31) << N;
			flag_Z = (alu_out == 0 ? 1 : 0) << Z;
			flag_C = shifter_carry_out << C;
			// flag_V unaffected
			
			flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
			flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
			arm_write_cpsr(p, flag_N);
			break;
		}
			

		// TEQ test d'équivalence page 378
		case 0x9: {
			uint32_t alu_out = arm_read_register(p, ins_rn) ^ ins_shifter;
			
			flag_N = get_bit(alu_out, 31) << N;
			flag_Z = (alu_out == 0 ? 1 : 0) << Z;
			flag_C = shifter_carry_out << C;
			// flag_V unaffected
			
			flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
			flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
			arm_write_cpsr(p, flag_N);
			break;
		}
			

		// CMP comparaison page 178
		case 0xa: {
			uint32_t alu_out = arm_read_register(p, ins_rn) - ins_shifter;
			
			flag_N = get_bit(alu_out, 31) << N;
			flag_Z = (alu_out == 0 ? 1 : 0) << Z;
			// flag_C = CarryFrom(arm_read_register(p, ins_rn) - ins_shifter) TODO
			// flag_V = OverflowFrom(arm_read_register(p, ins_rn) - ins_shifter) TODO
			
			flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
			flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
			arm_write_cpsr(p, flag_N);
			break;
		}
			

		// CMN comparaison en complément à deux page 176
		case 0xb: {
			uint32_t alu_out = arm_read_register(p, ins_rn) + ins_shifter;
			
			flag_N = get_bit(alu_out, 31) << N;
			flag_Z = (alu_out == 0 ? 1 : 0) << Z;
			// flag_C = CarryFrom(arm_read_register(p, ins_rn) + ins_shifter) TODO
			// flag_V = OverflowFrom(arm_read_register(p, ins_rn) + ins_shifter) TODO
			
			flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
			flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
			arm_write_cpsr(p, flag_N);
			break;
		}
			

		// ORR ou logique page 234
		case 0xc:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) | ins_shifter);

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				flag_C = shifter_carry_out << C;
				// V is unaffected

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
			

		// MOV chargement d'une valeur dans un registre page 218
		case 0xd:
			arm_write_register(p, ins_rd, ins_shifter);

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				flag_C = shifter_carry_out << C;
				// V is unaffected

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
			

		// BIC effacement de bit (non et logique) page 162
		case 0xe:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) & ~ins_shifter);

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				flag_C = shifter_carry_out << C;
				// V is unaffected

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
			

		// MVN chargement d'une valeur en complément à un dans un registre page 232
		case 0xf:
			arm_write_register(p, ins_rd, ~ins_shifter);

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31) << N;
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0) << Z;
				flag_C = shifter_carry_out << C;
				// V is unaffected

				flag_N = (flag_N | flag_Z | flag_C | flag_V) | 0x0fffffff;
				flag_N = (arm_read_cpsr(p) | 0x0fffffff) & flag_N;
				arm_write_cpsr(p, flag_N);
			}
			break;
	}

	// Pour l'instant les exceptions ne sont pas gérées, on renvoie donc 0 (bonne exécution si on atteint ce point)
	return 0;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}
