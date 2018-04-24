/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	filesystem.c
 * @brief 	Implementation of the core file system funcionalities and auxiliary functions.
 * @date	01/03/2017
 */

#include "include/filesystem.h"		// Headers for the core functionality
#include "include/auxiliary.h"		// Headers for auxiliary functions
#include "include/metadata.h"		// Type and structure declaration of the file system
#include "include/crc.h"			// Headers for the CRC functionality


/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */

char bufferBlock[MAX_BLOCKSIZE];
superBloque SB;

int mkFS(long deviceSize)
{
	//Primero se comprueba que el tamanyo de disco se encuentre entre los limites permitidos
	if(deviceSize < MIN_DEVICE_SIZE || deviceSize > MAX_DEVICE_SIZE){
		perror("mkFS: El sistema de ficheros tiene que estar entrer 50KiB y 100MiB");
		return -1;
	}
	//Se crea el super bloque con sus valores por defecto
	SB.numinodos = MAX_FILES;
	SB.numbloques = deviceSize/MAX_BLOCKSIZE;
	
	int i;
	for(i = 0; i< SB.numinodos; i++){
		SB.mapinodos[i] = 0; //Se establecen los inodos a libres
//Se establecen los valores por defecto de los inodos en el mismo bucle
		SB.inodos[i].filesize = 0;
		SB.inodos[i].crc = 0;
		SB.inodos[i].firstblock = 0;
	}

	unmountFS();

	printf("El Sistema de Ficheros se ha formateado exitosamente \n");
	return 0;

}

/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)
{
	int mount = 0;

	mount = bread(DEVICE_IMAGE, 0, (char*) &SB);

	if(mount == 0){
		return mount;
	}
	if(mount == -1){
		perror("mountFS: Error al montar el Sistema de Ficheros");

	}
	return mount;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	int unmount = 0;

	unmount = bwrite(DEVICE_IMAGE, 0, (char*) &SB);

	if(unmount == 0){
		return unmount;
	}
	if(unmount == -1){
		perror("unmountFS: Error al desmontar el Sistema de Ficheros");

	}
	return unmount;

}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{

/* Comprobación del tamaño del nombre del fichero
	if(sizeof(fileName) > MAX_LONGNAME) {
		perror("createFile: Nombre del archivo demasiado grande\n"); 
		return -2;
	}
	int size = sizeof(fileName);
	printf("createFile: Longitud %i \n", size);

*/
	
	int i; 
	//Primero descartamos que el fichero con ese nombre no existe y que esa posicion del mapa de inodos no esta ocupada
	for(i=0; i<SB.numinodos;i++){ //Desde i hasta numero de inodos
		if(strcmp(SB.inodos[i].filename, fileName) == 0){ 
			perror("createFile: Ya existe un fichero con el mismo nombre\n");
			return -1;
		}
	}

	for(i=0; i<SB.numinodos;i++){ //Desde i hasta numero de inodos
		if(SB.mapinodos[i] == 0){ //Si encontramos una posicion del mapa de inodos libre, se sale del bucle
			break;
		}
	}

	//Comprobamos que había algún bloque libre

	if(i >= SB.numinodos) perror("createFile: No hay bloques libres\n");

		

	//Habiendo descartado los errores para poder crear el fichero, se procede a su creacion
	SB.inodos[i].isopen = FCLOSE; //Se marca el fichero como cerrado
	SB.inodos[i].firstblock = i+1; //bloque de metadatos 1 (metadatos)+ numero posicion libre
	strcpy(SB.inodos[i].filename, fileName); //Nombe del fichero el recibido por parametro
	SB.inodos[i].filesize = 0; //Tamanyo inicial 0
	SB.mapinodos[i] = 1; //Posicion del mapa de inodos ocupada

	bwrite(DEVICE_IMAGE, SB.inodos[i].firstblock, (char*) &bufferBlock);
	printf("createFile: Fichero %s creado con exito \n", SB.inodos[i].filename);
	return 0;

}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	return -1;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}


/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{
	return -1;
}

/*
 * @brief 	Verifies the integrity of the file system metadata.
 * @return 	0 if the file system is correct, -1 if the file system is corrupted, -2 in case of error.
 */
int checkFS(void)
{
	return -2;
}

/*
 * @brief 	Verifies the integrity of a file.
 * @return 	0 if the file is correct, -1 if the file is corrupted, -2 in case of error.
 */
int checkFile(char *fileName)
{
	return -2;
}
