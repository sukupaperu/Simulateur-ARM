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
#ifndef __ARM_DATA_PROCESSING_H__
#define __ARM_DATA_PROCESSING_H__
#include <stdint.h>
#include "arm_core.h"

#define ADDITION 0
#define SOUSTRACTION 1

/*
	Le module arm_data_processing va permettre l'exécution des instructions suivantes :
	AND, EOR, SUB, RSB, ADD, ADC, SBC, RSC, TST,TEQ, CMP, CMN, ORR, MOV, BIC, MVN
	Ses fonctions sont appelées par arm_instruction qui se charge des premières étapes du décodage
*/

/*
	Entrée : un arm_core p, un uint32_t correspondant à l'instruction binaire
	Sortie : 0 si l'instruction se termine correctement, un code d'erreur (voir arm_constants.h) sinon
*/
int arm_data_processing_shift(arm_core p, uint32_t ins);

/*
	Non implémentée pour le moment, renvoie toujours UNDEFINED_INSTRUCTION
*/
int arm_data_processing_immediate_msr(arm_core p, uint32_t ins);


/* Opérations de calcul des flags C et V 
   ATTENTION : la spécification indique que normalement, le calcul ne doit pas être refait lors du calcul des flags
   Donc pour l'instant ces fonctions ne répondent pas correctement car elles doivent refaire les calculs
   À voir si cela peut être amélioré
*/

/*
	Entrée : deux uint32_t a et b
	Sortie : 1 si l'opération (a - b) provoque un emprunt, c'est à dire si le résultat réel est inférieur à 0
			 0 sinon
*/
int borrow_from(uint32_t a, uint32_t b);

/*
	Entrée : trois uint32_t a, b, c
	Sortie : 1 si l'opération (a - b - c) provoque un emprunt, c'est à dire si le résultat réel est inférieur à 0
			 0 sinon
*/
int borrow_from_tri(uint32_t a, uint32_t b, uint32_t c);

/*
	Entrée : deux uint32_t a et b
	Sortie : 1 si l'opération (a + b) provoque une retenue, c'est à dire si le résultat réel est supérieur à 2^32 - 1
			 0 sinon
*/
int carry_from(uint32_t a, uint32_t b);

/*
	Entrée : trois uint32_t a, b, c
	Sortie : 1 si l'opération (a + b + c) provoque une retenue, c'est à dire si le résultat réel est supérieur à 2^32 - 1
			 0 sinon
*/
int carry_from_tri(uint32_t a, uint32_t b, uint32_t c);

/*
	Entrée : deux uint32_t a et b, un code d'opération op (ADDITION ou SOUSTRACTION)
	Sortie : 1 si l'opération (a op b) provoque un overflow, c'est à dire si le signe du résultat est incohérent
			 0 sinon
			 0 si le code d'opération est invalide
*/
int overflow_from(int op, uint32_t a, uint32_t b);

/*
	Entrée : trois uint32_t a, b et c, un code d'opération op (ADDITION ou SOUSTRACTION)
	Sortie : 1 si l'opération (a op b op c) provoque un overflow, c'est à dire si le signe du résultat est incohérent
			 0 sinon
			 0 si le code d'opération est invalide
*/
int overflow_from_tri(int op, uint32_t a, uint32_t b, uint32_t c);

/*
	Entrée : quatre flags (des uint32_t dont le bit significatif doit être le bit faible)
	Sortie : void
	Effets de bord : met à jour CPSR selon les flags indiqués
*/
void write_flags(uint32_t flag_N, uint32_t flag_Z, uint32_t flag_C, uint32_t flag_V);

#endif
