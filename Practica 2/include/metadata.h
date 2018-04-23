/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */
#include <stdint.h>
#include <string.h>

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
	if (val_)
		bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
	else
		bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

//A partir de aqui esto es definido por el alumno

/*CONSTANTES*/

#define MAX_BLOCKSIZE 2048 //Maximo tamanyo del bloque en bytes

#define MAX_LONGNAME 32 //Longitud maxima del fichero

#define MAX_FILES 40 //Maximo numero de ficheros

/*ESCTRUCTURAS*/
typedef struct{
	char filename[MAX_LONGNAME]; //Longitud maxima 32 caracteres
	uint32_t filesize; //tamanyo actual del fichero
	char isopen; //Indica si el fichero esta abierto o cerrado
	uint16_t crc; //crc16
	uint16_t firstblock; //primer bloque del fichero, representable con 2 bytes
}iNodo;

typedef struct{
	uint8_t numinodos; //Numero de ficheros actualmente
	char mapinodos[MAX_FILES]; //Mapa de inodos, si es 1 es que estan siendo usados, sino 0
	iNodo inodos[MAX_FILES];
}superBloque;




