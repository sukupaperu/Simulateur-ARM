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

// Les pages 68 � 71 sont d�di�es � l'espace m�moire et la gestion des adresses

#include <stdlib.h>
#include "memory.h"
#include "util.h"

struct memory_data {
    // Type de boutisme de la m�moire
    int is_big_endian_type;
    // Taille de la m�moire (en octets)
    int size;
    // Espace m�moire (sous forme d'entiers non sign�s sur 8 bits)
    uint8_t* data;
};

memory memory_create(size_t size, int is_big_endian) {
    memory mem = malloc(sizeof(memory));

    if (mem != NULL && size > 0) {
        mem->data = calloc(size, sizeof(uint8_t));
        mem->size = size;
        mem->is_big_endian_type = is_big_endian;
    }

    // Si l'allocation �choue ou que la taille est invalide on lib�re les espaces m�moires pr�c�demment allou�s
    if (mem->data == NULL) {
        free(mem->data);
        free(mem);
        mem = NULL;
    }

    return mem;
}

size_t memory_get_size(memory mem) {
    return mem->size;
}

void memory_destroy(memory mem) {
    free(mem->data);
    free(mem);
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    // si on s�lectionne une adresse en dehors de l'espace adressable
    if(address > mem->size)
        return -1;

    *value = mem->data[address];

    return 0;
}

int memory_read_half(memory mem, uint32_t address, uint16_t *value) {
    // si l'adresse n'est pas align�e sur un half ou que l'on s�lectionne une adresse en dehors de l'espace adressable
    if(get_bit(address, 0) != 0 || address + 1 > mem->size)
        return -1;

    // en fonction du type de boutisme de mem
    if(mem->is_big_endian_type)
        *value = (mem->data[address] << 8) | mem->data[address + 1];
    else
        *value = (mem->data[address + 1] << 8) | mem->data[address];

    return 0;
}

int memory_read_word(memory mem, uint32_t address, uint32_t *value) {
    // si l'adresse n'est pas align�e sur un word ou que l'on s�lectionne une adresse en dehors de l'espace adressable
    if(get_bits(address, 1, 0) != 0 || address + 3 > mem->size)
        return -1;

    // en fonction du type de boutisme de mem
    if(mem->is_big_endian_type)
        *value = (mem->data[address] << 24) 
            | (mem->data[address + 1] << 16) 
            | (mem->data[address + 2] << 8) 
            | mem->data[address + 3];
    else
        *value = (mem->data[address + 3] << 24) 
            | (mem->data[address + 2] << 16) 
            | (mem->data[address + 1] << 8) 
            | mem->data[address];

    return 0;
}

int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    // si on s�lectionne une adresse en dehors de l'espace adressable
    if(address > mem->size)
        return -1;

    mem->data[address] = value;

    return 0;
}

int memory_write_half(memory mem, uint32_t address, uint16_t value) {
    // si l'adresse n'est pas align�e sur un half ou que l'on s�lectionne une adresse en dehors de l'espace adressable
    if(get_bit(address, 0) != 0 || address + 1 > mem->size)
        return -1;

    // en fonction du type de boutisme de mem
    if(mem->is_big_endian_type) {
        mem->data[address] = value >> 8;
        mem->data[address + 1] = value;
    } else {
        mem->data[address + 1] = value >> 8;
        mem->data[address] = value;
    }

    return 0;
}

int memory_write_word(memory mem, uint32_t address, uint32_t value) {
    // si l'adresse n'est pas align�e sur un word ou que l'on s�lectionne une adresse en dehors de l'espace adressable
    if(get_bits(address, 1, 0) != 0 || address + 3 > mem->size)
        return -1;

    // en fonction du type de boutisme de mem
    if(mem->is_big_endian_type) {
        mem->data[address] = value >> 24;
        mem->data[address + 1] = value >> 16;
        mem->data[address + 2] = value >> 8;
        mem->data[address + 3] = value;
    } else {
        mem->data[address + 3] = value >> 24;
        mem->data[address + 2] = value >> 16;
        mem->data[address + 1] = value >> 8;
        mem->data[address] = value;
    }

    return 0;
}
