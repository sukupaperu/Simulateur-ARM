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
#ifndef __ARM_DATA_PROCESSING_H__
#define __ARM_DATA_PROCESSING_H__
#include <stdint.h>
#include "arm_core.h"

/*
	Le module arm_data_processing va permettre l'ex�cution des instructions suivantes :
	AND, EOR, SUB, RSB, ADD, ADC, SBC, RSC, TST,TEQ, CMP, CMN, ORR, MOV, BIC, MVN
	Ses fonctions sont appel�es par arm_instruction qui se charge des premi�res �tapes du d�codage
*/

/*
	Entr�e : un arm_core p, un uint32_t correspondant � l'instruction binaire
	Sortie : 0 si l'instruction se termine correctement, un code d'erreur (voir arm_constants.h) sinon
*/
int arm_data_processing_shift(arm_core p, uint32_t ins);

/*
	Non impl�ment�e pour le moment, renvoie toujours UNDEFINED_INSTRUCTION
*/
int arm_data_processing_immediate_msr(arm_core p, uint32_t ins);

#endif
