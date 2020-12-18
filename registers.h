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
#ifndef __REGISTERS_H__
#define __REGISTERS_H__
#include <stdint.h>

// La spécification et documentation pour tout ce qui concerne les registres se trouve à la section A2.3 Registers (page 42 du PDF) du Reference Manual

#define MODE_USER 0x10
#define MODE_SYSTEM 0x1F
#define MODE_FIQ 0x11
#define MODE_IRQ 0x12
#define MODE_SUPERVISOR 0x13
#define MODE_ABORT 0x17
#define MODE_UNDEFINED 0x1B

// la structure registers, qui est une interface (l'utilisateur n'a pas besoin de savoir comment elle est implémentée, il n'a qu'à s'en servir grâce aux fonctions données)
typedef struct registers_data *registers;

/* 
	Entrée : void
	Sortie : un groupe des registres, initialisé
	Effets de bord : alloue de la mémoire pour les registres
*/
registers registers_create();

/*
	Entrée : un groupe de registres
	Sorties : void
	Effets de bord : libère la mémoire allouée aux registres donnés, les efface
	N'a aucun effet si r == NULL
*/
void registers_destroy(registers r);

/*
	Entrée : un groupe de registres
	Sortie : un uint8_t dont la valeur correspond aux cinq bits [4:0] du registre CPSR. Ils représentent un des sept modes possibles
*/
uint8_t get_mode(registers r);

/*
	Entrée : un groupe de registres
	Sortie : 0 si le mode des registres est User ou System, 1 sinon
*/
int current_mode_has_spsr(registers r);

/*
	Entrée : un groupe de registres
	Sortie : 0 si le mode des registres est User, 1 sinon
*/
int in_a_privileged_mode(registers r);

/*
	Entrée : un groupe de registres, un entier correspondant au registre à lire (de 0 à 15)
	si reg n'est pas dans la plage [0..15], la fonction renvoie 0
	Sortie : le contenu du registre cible
*/
uint32_t read_register(registers r, uint8_t reg);

/*
	Entrée : un groupe de registres, un entier correspondant au registre à lire (de 0 à 15)
	si reg n'est pas dans la plage [0..15], la fonction renvoie 0
	Sortie : le contenu du registre cible, lu comme si le mode était user
*/
uint32_t read_usr_register(registers r, uint8_t reg);

/*
	Entrée : un groupe de registres
	Sortie : le contenu du registre CPSR
*/
uint32_t read_cpsr(registers r);

/*
	Entrée : un groupe de registres
	Sortie : le contenu du registre SPSR
*/
uint32_t read_spsr(registers r);

/*
	Entrée : un groupe de registres, le numéro d'un registre cible, une valeur à écrire dans ce registre
	si reg n'est pas dans la plage [0..15], la fonction ne fait rien
	Sortie : void
	Effets de bords : la valeur value est écrite dans le registre de numéro reg
	ATTENTION : n'empêche pas l'écriture d'une adresse dans R15 avec [1:0] != 00
*/
void write_register(registers r, uint8_t reg, uint32_t value);

/*
	Entrée : un groupe de registres, le numéro d'un registre cible, une valeur é écrire dans ce registre
	si reg n'est pas dans la plage [0..15], la fonction ne fait rien
	Sortie : void
	Effets de bords : la valeur value est écrite dans le registre de numéro reg, comme si le mode était user
	ATTENTION : n'empêche pas l'écriture d'une adresse dans R15 avec [1:0] != 00
*/
void write_usr_register(registers r, uint8_t reg, uint32_t value);

/*
	Entrée : un groupe de registres, une valeur à écrire dans le registre CPSR
	Sortie : void
	Effets de bords : la valeur value est écrite dans CPSR, en ignorant les écritures interdites par le mode actuel
	ATTENTION : cette fonction n'empêche pas l'écriture d'un code de mode invalide dans CPSR[4:0] !
	Si un code invalide est écrit dans CPSR, le comportement du processeur devient imprévisible !
*/
void write_cpsr(registers r, uint32_t value);

/*
	Entrée : un groupe de registres, une valeur à écrire dans le registre SPSR
	Sortie : void
	Effets de bords : la valeur value est écrite dans SPSR, en ignorant les écritures interdites par le mode actuel
*/
void write_spsr(registers r, uint32_t value);

#endif
