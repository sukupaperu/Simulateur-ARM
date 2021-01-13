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
	char shifter_carry_out = (char) get_bit(arm_read_cpsr(p), C);

	// shifter operand, sur 12 bits, représente soit une valeur immédiate soit un registre, sert de seconde opérande
	// 1 valeur immédiate
	uint32_t ins_shifter;
	if (ins_I == 1) {
		uint8_t rotation = ((ins >> 8) & 0xf) * 2;
		ins_shifter = ror(ins & 0xff,rotation);

		if (rotation != 0) {
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
				ins_shifter = (shifter_carry_out << 31) | (rm >> 1);
				shifter_carry_out = get_bit(rm, 0);
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
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = shifter_carry_out;
				// V is unaffected

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
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
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = shifter_carry_out;
				// V is unaffected

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
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
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = !borrow_from(arm_read_register(p, ins_rn), ins_shifter);
				flag_V = overflow_from(SOUSTRACTION, arm_read_register(p, ins_rn), ins_shifter);

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
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
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = !borrow_from(ins_shifter, arm_read_register(p, ins_rn));
				flag_V = overflow_from(SOUSTRACTION, ins_shifter, arm_read_register(p, ins_rn));

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
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
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = carry_from(arm_read_register(p, ins_rn), ins_shifter);
				flag_V = overflow_from(ADDITION, arm_read_register(p, ins_rn), ins_shifter);

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
			}
			break;
			

		// ADC addition avec carry page 154
		case 0x5:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) + ins_shifter + get_bit(arm_read_cpsr(p), C));

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = carry_from_tri(arm_read_register(p, ins_rn), ins_shifter, get_bit(arm_read_cpsr(p), C));
				flag_V = overflow_from_tri(ADDITION, arm_read_register(p, ins_rn), ins_shifter, get_bit(arm_read_cpsr(p), C));

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
			}
			break;
			

		// SBC soustraction avec carry page 275
		case 0x6:
			arm_write_register(p, ins_rd, arm_read_register(p, ins_rn) - ins_shifter - !get_bit(arm_read_cpsr(p), C));

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = !borrow_from_tri(arm_read_register(p, ins_rn), ins_shifter, !get_bit(arm_read_cpsr(p), C));
				flag_V = overflow_from_tri(SOUSTRACTION, arm_read_register(p, ins_rn), ins_shifter, !get_bit(arm_read_cpsr(p), C));

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
			}
			break;
			

		// RSC soustraction avec carry et opérandes inversées page 267
		case 0x7:
			arm_write_register(p, ins_rd, ins_shifter - arm_read_register(p, ins_rn) - !get_bit(arm_read_cpsr(p), C));

			if (ins_S == 1 && ins_rd == 0xf) {
				if (arm_current_mode_has_spsr(p)) {
					arm_write_cpsr(p, arm_read_spsr(p));
				} // else { UNPREDICTABLE }

			} else if (ins_S == 1) {
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = !borrow_from_tri(ins_shifter, arm_read_register(p, ins_rn), !get_bit(arm_read_cpsr(p), C));
				flag_V = overflow_from_tri(SOUSTRACTION, ins_shifter, arm_read_register(p, ins_rn), !get_bit(arm_read_cpsr(p), C));

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
			}
			break;
			

		// TST test de deux valeurs page 380
		case 0x8: {
			uint32_t alu_out = arm_read_register(p, ins_rn) & ins_shifter;
			
			flag_N = get_bit(alu_out, 31);
			flag_Z = (alu_out == 0 ? 1 : 0);
			flag_C = shifter_carry_out;
			// flag_V unaffected
			
			write_flags(flag_N, flag_Z, flag_C, flag_V, p);
			break;
		}
			

		// TEQ test d'équivalence page 378
		case 0x9: {
			uint32_t alu_out = arm_read_register(p, ins_rn) ^ ins_shifter;
			
			flag_N = get_bit(alu_out, 31);
			flag_Z = (alu_out == 0 ? 1 : 0);
			flag_C = shifter_carry_out;
			// flag_V unaffected
			
			write_flags(flag_N, flag_Z, flag_C, flag_V, p);
			break;
		}
			

		// CMP comparaison page 178
		case 0xa: {
			uint32_t alu_out = arm_read_register(p, ins_rn) - ins_shifter;
			
			flag_N = get_bit(alu_out, 31);
			flag_Z = (alu_out == 0 ? 1 : 0);
			flag_C = !borrow_from(arm_read_register(p, ins_rn), ins_shifter);
			flag_V = overflow_from(SOUSTRACTION, arm_read_register(p, ins_rn), ins_shifter);
			
			write_flags(flag_N, flag_Z, flag_C, flag_V, p);
			break;
		}
			

		// CMN comparaison en complément à deux page 176
		case 0xb: {
			uint32_t alu_out = arm_read_register(p, ins_rn) + ins_shifter;
			
			flag_N = get_bit(alu_out, 31);
			flag_Z = (alu_out == 0 ? 1 : 0);
			flag_C = carry_from(arm_read_register(p, ins_rn), ins_shifter);
			flag_V = overflow_from(ADDITION, arm_read_register(p, ins_rn), ins_shifter);
			
			write_flags(flag_N, flag_Z, flag_C, flag_V, p);
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
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = shifter_carry_out;
				// V is unaffected

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
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
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = shifter_carry_out;
				// V is unaffected

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
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
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = shifter_carry_out;
				// V is unaffected

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
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
				flag_N = get_bit(arm_read_register(p, ins_rd), 31);
				flag_Z = (arm_read_register(p, ins_rd) == 0 ? 1 : 0);
				flag_C = shifter_carry_out;
				// V is unaffected

				write_flags(flag_N, flag_Z, flag_C, flag_V, p);
			}
			break;
	}

	// Pour l'instant les exceptions ne sont pas gérées, on renvoie donc 0 (bonne exécution si on atteint ce point)
	return 0;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}

int borrow_from(uint32_t a, uint32_t b) {
	// a - b = c cause un Borrow si le résultat est inférieur à 0, donc si b > a
	return b > a;
}

int borrow_from_tri(uint32_t a, uint32_t b, uint32_t c) {
	// a - b - c = d cause un Borrow si le résultat est inférieur à 0, donc si b > a
	return a < b + c || b > a - c || c > a - b;
}

int carry_from(uint32_t a, uint32_t b) {
	// a + b = c cause un Carry si le résultat réel est supérieur à 2^32-1
    // a + b > 2^32-1 soit a > 2^32-1 - b OR b > 2^32-1 - a
	return a > 0xffffffff - b || b > 0xfffffff - a;
}

int carry_from_tri(uint32_t a, uint32_t b, uint32_t c) {
	// a + b + c = d cause un Carry si le résultat réel est supérieur à 2^32-1
	return a > 0xffffffff - b - c || b > 0xfffffff - a - c || c > 0xfffffff - a - b;
}

int overflow_from(int op, uint32_t a, uint32_t b) {
	uint32_t c = 0;

	if (op == ADDITION) {
		c = a + b;
		// a + b = c cause un Overflow si a[31] == b[31] && a[31] != c[31]
		return get_bit(a, 31) == get_bit(b, 31) && get_bit(a, 31) != get_bit(c, 31);

	} else if (op == SOUSTRACTION) {
		c = a - b;
		// a - b = c cause un Overflow si a[31] != b[31] && a[31] != c[31]
		return get_bit(a, 31) != get_bit(b, 31) && get_bit(a, 31) != get_bit(c, 31);

	} else {
		// code d'opération invalide, on renvoie false
		return 0;
	}
}

int overflow_from_tri(int op, uint32_t a, uint32_t b, uint32_t c) {
	return overflow_from(op, a, b) || overflow_from(op, a, c) || overflow_from(op, b, c) || overflow_from(op, a+b, c);
}

void write_flags(uint32_t flag_N, uint32_t flag_Z, uint32_t flag_C, uint32_t flag_V, arm_core p) {
	// décalage des bits à leur place
	flag_N = flag_N << N;
	flag_Z = flag_Z << Z;
	flag_C = flag_C << C;
	flag_V = flag_V << V;

	// on réunit les flags pour obtenir OxX0000000, X étant les 4 bits des flags
	flag_N = (flag_N | flag_Z | flag_C | flag_V);

	// flag_N prend la valeur à écrire dans cpsr, qui est la valeur actuelle mais seuls les flags changents (bits de poids fort)
	flag_N = (arm_read_cpsr(p) & 0x0fffffff) | flag_N;
	// on écrit le résultat
	arm_write_cpsr(p, flag_N);
}