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
#ifndef __REGISTERS_H__
#define __REGISTERS_H__
#include <stdint.h>

// La sp�cification et documentation pour tout ce qui concerne les registres se trouve � la section A2.3 Registers (page 42 du PDF) du Reference Manual

#define MODE_USER 0x10
#define MODE_SYSTEM 0x1F
#define MODE_FIQ 0x11
#define MODE_IRQ 0x12
#define MODE_SUPERVISOR 0x13
#define MODE_ABORT 0x17
#define MODE_UNDEFINED 0x1B

// la structure registers, qui est une interface (l'utilisateur n'a pas besoin de savoir comment elle est impl�ment�e, il n'a qu'� s'en servir gr�ce aux fonctions donn�es)
typedef struct registers_data *registers;

/* 
	Entr�e : void
	Sortie : un groupe des registres, initialis�
	Effets de bord : alloue de la m�moire pour les registres
*/
registers registers_create();

/*
	Entr�e : un groupe de registres
	Sorties : void
	Effets de bord : lib�re la m�moire allou�e aux registres donn�s, les efface
	N'a aucun effet si r == NULL
*/
void registers_destroy(registers r);

/*
	Entr�e : un groupe de registres
	Sortie : un uint8_t dont la valeur correspond aux cinq bits [4:0] du registre CPSR. Ils repr�sentent un des sept modes possibles
*/
uint8_t get_mode(registers r);

/*
	Entr�e : un groupe de registres
	Sortie : 0 si le mode des registres est User ou System, 1 sinon
*/
int current_mode_has_spsr(registers r);

/*
	Entr�e : un groupe de registres
	Sortie : 0 si le mode des registres est User, 1 sinon
*/
int in_a_privileged_mode(registers r);

/*
	Entr�e : un groupe de registres, un entier correspondant au registre � lire (de 0 � 15)
	si reg n'est pas dans la plage [0..15], la fonction renvoie 0
	Sortie : le contenu du registre cible
*/
uint32_t read_register(registers r, uint8_t reg);

/*
	Entr�e : un groupe de registres, un entier correspondant au registre � lire (de 0 � 15)
	si reg n'est pas dans la plage [0..15], la fonction renvoie 0
	Sortie : le contenu du registre cible, lu comme si le mode �tait user
*/
uint32_t read_usr_register(registers r, uint8_t reg);

/*
	Entr�e : un groupe de registres
	Sortie : le contenu du registre CPSR
*/
uint32_t read_cpsr(registers r);

/*
	Entr�e : un groupe de registres
	Sortie : le contenu du registre SPSR
*/
uint32_t read_spsr(registers r);

/*
	Entr�e : un groupe de registres, le num�ro d'un registre cible, une valeur � �crire dans ce registre
	si reg n'est pas dans la plage [0..15], la fonction ne fait rien
	Sortie : void
	Effets de bords : la valeur value est �crite dans le registre de num�ro reg
	ATTENTION : n'emp�che pas l'�criture d'une adresse dans R15 avec [1:0] != 00
*/
void write_register(registers r, uint8_t reg, uint32_t value);

/*
	Entr�e : un groupe de registres, le num�ro d'un registre cible, une valeur � �crire dans ce registre
	si reg n'est pas dans la plage [0..15], la fonction ne fait rien
	Sortie : void
	Effets de bords : la valeur value est �crite dans le registre de num�ro reg, comme si le mode �tait user
	ATTENTION : n'emp�che pas l'�criture d'une adresse dans R15 avec [1:0] != 00
*/
void write_usr_register(registers r, uint8_t reg, uint32_t value);

/*
	Entr�e : un groupe de registres, une valeur � �crire dans le registre CPSR
	Sortie : void
	Effets de bords : la valeur value est �crite dans CPSR, en ignorant les �critures interdites par le mode actuel
	ATTENTION : cette fonction n'emp�che pas l'�criture d'un code de mode invalide dans CPSR[4:0] !
	Si un code invalide est �crit dans CPSR, le comportement du processeur devient impr�visible !
*/
void write_cpsr(registers r, uint32_t value);

/*
	Entr�e : un groupe de registres, une valeur � �crire dans le registre SPSR
	Sortie : void
	Effets de bords : la valeur value est �crite dans SPSR, en ignorant les �critures interdites par le mode actuel
*/
void write_spsr(registers r, uint32_t value);

#endif
