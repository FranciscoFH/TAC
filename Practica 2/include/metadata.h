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
#define MIN_DEVICE_SIZE 51200 //NUMERO MINIMO DE BYTES DEL DISPOSITIVO
#define MAX_DEVICE_SIZE 104857600 //NUMERO MAXIMO DE BYTES DEL DISPOSITIVO
#define FOPEN 1	//Indicar que el fichero está abierto
#define FCLOSE 0 //Indicar que el fichero está cerrado

/*ESCTRUCTURAS*/
typedef struct{
	char filename[MAX_LONGNAME]; //Longitud maxima 32 caracteres
	uint32_t filesize; //tamanyo actual del fichero
	char isopen; //Indica si el fichero esta abierto o cerrado
	uint16_t crc; //crc16
	uint16_t bloquePuntero; //Indica el bloque en el que se encuentra el puntero
	uint16_t bloquesEnInodo; //número de bloques asociados al inodo
	uint16_t punteroLectura; //puntero que apunta al lugar del archivo donde nos encontramos
	uint16_t blocksAsocidos[MAX_FILE_SIZE]; //Número de los bloques asociados
}iNodo;

typedef struct{
	uint8_t numinodos; //Numero de ficheros actualmente
	char inodosBitMap[MAX_FILES/8]; //Dividimos en 8 porque en cada char nos caben 8 bits que contendran el estado de 8 ficheros. 
	char blockBitMap[MAX_FILE_SIZE/8]; //Mapa que nos guarda el estado de los bloques
	iNodo inodos[MAX_FILES];
	uint16_t numbloques;
}superBloque;





