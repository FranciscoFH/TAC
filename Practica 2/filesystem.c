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

char bufferBlock[BLOCKSIZE];
superBloque SB;
iNodo inodos[MAX_FILES];

//Devuelve el primer bloque libre que encuentre
int reservarBloqueLibre(void){
	int biteLibre;
	for(int i = 0; i< SB.numbloques; i++){
			biteLibre = bitmap_getbit(SB.blockBitMap,i);
			if(biteLibre == 0){
				bitmap_setbit(SB.blockBitMap,i,1);
				return i+1;
			}	
	}
	return -1;
}
int reservarInodoLibre(void){
	int inodoLibre;
	for(int i = 0; i< SB.numinodos; i++){
			inodoLibre = bitmap_getbit(SB.inodosBitMap,i);
			if(inodoLibre == 0){
				bitmap_setbit(SB.inodosBitMap,i,1);
				return i+1;
			}	
	}
	return -1;
}

int mkFS(long deviceSize)
{
	//Primero se comprueba que el tamanyo de disco se encuentre entre los limites permitidos
	if(deviceSize < MIN_DEVICE_SIZE || deviceSize > MAX_DEVICE_SIZE){
		perror("mkFS: El sistema de ficheros tiene que estar entrer 50KiB y 100MiB");
		return -1;
	}
	//Se crea el super bloque con sus valores por defecto
	SB.numinodos = MAX_FILES;
	//FUNCION TECHO DE ESTO
	SB.numbloques = deviceSize/BLOCKSIZE;	
	int i;
	for(i = 0; i< SB.numinodos; i++){
//Se establecen los valores por defecto de los inodos en el mismo bucle
		inodos[i].filesize = 0;
		inodos[i].crc = 0;
		inodos[i].punteroBloque = 0;
		inodos[i].bloquesEnInodo = 0;
		inodos[i].puntero = 0;
		inodos[i].crc = 0;
		inodos[i].bloquesEnInodo = 0;
		SB.blockBitMap[i] = 0; //Ponemos todos los bloques a libre
	}
	//Continuamos iniciando a 0 los bloques por si con el bucle anterior no se ha completado
	for(int j = i; j < MAX_DEVICE_SIZE/BLOCKSIZE/8; j++){
		SB.blockBitMap[j] = 0;
	}
	SB.blockBitMap[0] = 1; //El primer bloque pertenece al superbloque, por lo que lo reservamos

	//Iniciamos a 0 el mapa de inodos. Lo hacemos en un bucle aparte para reducir en 8 el tamaño 
	for(int p=0; p< MAX_FILES/8; p++){
		SB.inodosBitMap[p] = 0;
	}

/*
	//Inicializaremos a cero todas las posiciones de los futuros bloques asociados a un inodo
	for(int i=0; i<SB.numinodos; i++){
		for(int j=0; i<SB.numbloques; j++){
			SB.inodos[i].blocksAsocidosBitMap[j] = 0;
		}
	}
*/
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

	int mountSB = bread(DEVICE_IMAGE, 0, (char*) &SB);
	if(mountSB == 0){
		return 0;
	}
	else{
		perror("unmountFS: Error al desmontar el Sistema de Ficheros");
		return -1;
	}
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	//Primero comprobamos que todos los ficheros estan cerrados.
	for(int i = 0; i< SB.numinodos; i++){
		if(SB.inodos[i].isopen == FOPEN){
			perror("unmountFS: Hay ficheros abiertos");
			return -1;
		}
	}

	int unmountSB = bwrite(DEVICE_IMAGE, 0, (char*) &SB);
	if(unmountSB == 0){
		return 0;
	}
	else{
		perror("unmountFS: Error al desmontar el Sistema de Ficheros");
		return -1;
	}
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
	//Primero descartamos que el fichero con ese nombre no existe
	for(i=0; i<SB.numinodos;i++){ //Desde i hasta numero de inodos
		if(strcmp(SB.inodos[i].filename, fileName) == 0){ 
			perror("createFile: Ya existe un fichero con el mismo nombre\n");
			return -1;
		}
	}

	int inodo = reservarInodoLibre();
	if(inodo == -1) perror("createFile: No hay inodos libres\n");

	int bloque = reservarBloqueLibre();
	if(bloque == -1) perror("createFile: No hay bloques libres\n");
	
	//Habiendo descartado los errores para poder crear el fichero, se procede a su creacion
	SB.inodos[inodo].isopen = FCLOSE; //Se marca el fichero como cerrado
	SB.inodos[inodo].punteroBloque = bloque; //el puntero se encuentra en el primer bloque
	strcpy(SB.inodos[inodo].filename, fileName); //Nombre del fichero el recibido por parametro
	SB.inodos[inodo].crc = 26897; //Valor correspondiente a un crc de un fichero vacio
	SB.inodos[inodo].bloquesEnInodo = 1;
	SB.inodos[inodo].blocksAsocidos[0] = bloque;

	bwrite(DEVICE_IMAGE, SB.inodos[i].blocksAsocidos[0], (char*) &bufferBlock);
	printf("createFile: Fichero %s creado con exito \n", SB.inodos[i].filename);
	return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	//Busco el fichero, reinicio sus atributos y restauro las posiciones en el imap e iblock
	for(int i=0; i<SB.numinodos;i++){ 
		if(strcmp(SB.inodos[i].filename, fileName) == 0){ 
			if(SB.inodos[i].isopen == FOPEN){
				perror("removeFile: El fichero no puede ser borrado porque está abierto \n");
				return -2;
			}

			//Borramos el nombre y los atributos
			memset(&(SB.inodos[i].filename), 0, MAX_LONGNAME);
			SB.inodos[i].filesize = 0; 
			SB.inodos[i].isopen = 0; 
			SB.inodos[i].crc = 0; 
			SB.inodos[i].punteroBloque = 0; 
			SB.inodos[i].puntero = 0; 

			//Marcamos como libre su posición en el mapa de inodos
			SB.inodosBitMap[i] = 0;

			//Recorremos los bloques que lo forman para borrar su información y poner un 0 en el mapa de bits
			//Creamos un buffer vacio para llenar el fichero con espacios en blanco
			char * buffer [BLOCK_SIZE];
			memset(buffer, "", BLOCK_SIZE);
			for(int t = 0; t<SB.inodos[i].bloquesEnInodo; t++){
				bitmap_setbit(SB.blockBitMap,SB.inodos[i].blocksAsocidos[t] - 1,0);
				//Vaciamos los ficheros y los metemos en el disco
				writeFile(SB.inodos[i].blocksAsocidos[t], buffer, BLOCKSIZE);
				bwrite(DEVICE_IMAGE, SB.inodos[i].blocksAsocidos[t], (char*) bufferBlock);
			}
			SB.inodos[i].bloquesEnInodo = 0;

			

			printf("removeFile: Fichero %s borrado con exito \n", SB.inodos[i].filename);
			return 0;
		}
	}
	perror("removeFile: El fichero no puede ser borrado porque no existe \n");
	return -1;
	//Mirar más errores posibles
}


/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
	//Verificamos el estado del fichero
	int verificacion = checkFile(fileName);
	if(verificacion == -1) perror("openFile: El fichero esta corrupto \n"); return -2;
	if(verificacion == -2) perror("openFile: El fichero esta abierto y no se puede verificar \n"); return -2;

	//Buscamos el fichero y lo abrimos.

	for(int i=0; i<SB.numinodos;i++){ //Desde i hasta numero de inodos
		if(strcmp(SB.inodos[i].filename, fileName) == 0){ 
			//Preguntar que si el archivo ya estaba abierto es un error
			if(SB.inodos[i].isopen == FOPEN){
				perror("openFile: El archivo ya estaba abierto \n");
				return -2;
			}
			else{
				//Abrimos el fichero y ponemos su puntero de lectura al principio
				SB.inodos[i].isopen = FOPEN;
				SB.inodos[i].puntero = 0;
				SB.inodos[i].punteroBloque = SB.inodos[i].blocksAsocidos[0];
				return SB.inodos[i].blocksAsocidos[0];
			}
		}
	}	
	perror("openFile: El fichero no puede ser abierto porque no existe en el sistema de ficheros \n");
	return -1;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	int posicion = -1;
	//Comparamos el descriptor de fichero con el recibido por parámetro hasta encontrar el inodo correspondiente
	for(int i=0; i<SB.numinodos;i++){ //Desde i hasta numero de inodos
		if(SB.inodos[i].blocksAsocidos[0] == fileDescriptor){
			posicion = i;
			break;
		}
	}
	if(posicion == -1) perror("closeFile: No se ha encontrado el descriptor del fichero\n"); return -1;

	if(SB.inodos[posicion].isopen == FCLOSE){
		perror("openFile: El archivo ya estaba cerrado \n");
		return -1;
	}
	else{
				//Cerramos el fichero 
		SB.inodos[posicion].isopen = FCLOSE;
		return 0;
	}
	
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	int bytesLeidos = 0;
	//Comprobamos los parámetros
	if(fileDescriptor <= -1) perror("readFile: Descriptor de fichero negativo\n"); return -1;	
	if(numBytes <= -1) perror("readFile: Número de bytes a leer negativo\n"); return -1;
	if(numBytes == 0) return 0;

	int posicion = -1;
	//Comparamos el descriptor de fichero con el recibido por parámetro hasta encontrar el inodo correspondiente
	for(int i=0; i<SB.numinodos;i++){ //Desde i hasta numero de inodos
		if(SB.inodos[i].blocksAsocidos[0] == fileDescriptor){
			posicion = i;
			break;
		}
	}

	//Realizamos las comprobaciones necesarias

	if(posicion == -1) perror("readFile: No se ha encontrado el descriptor del fichero\n"); return -1;
	if(SB.inodos[posicion].isopen == FCLOSE) perror("readFile: El fichero no se puede leer ya que no está abierto\n"); return -1;

	uint16_t punteroBloque = SB.inodos[posicion].punteroBloque; //Bloque en el que se encuentra el puntero
	uint16_t bloquesEnInodo = SB.inodos[posicion].bloquesEnInodo; //Numero de bloques que tiene el inodo
	uint16_t puntero = SB.inodos[posicion].puntero; //Posición del puntero
	uint16_t bloquesEnInodoRestantes = bloquesEnInodo - punteroBloque + 1;

	//Si estamos en el ultimo bloque del fichero, y el puntero está l final
	if((bloquesEnInodoRestantes == 1) && (puntero ==  SB.inodos[posicion].filesize)) return 0;

	//Creamos un buffer que va a agregar todos los bloques que quedan por leer

	char *bufferLeer[BLOCKSIZE];

	int bytesPosibleBloque;

	for(punteroBloque; punteroBloque <= bloquesEnInodo ; punteroBloque++){
		bread(DEVICE_IMAGE, SB.inodos[posicion].blocksAsocidos[punteroBloque-1], bufferLeer);
		bytesPosibleBloque = BLOCKSIZE - puntero;
		//Si entramos aquí signfica que hemos leido todos
		if(bytesPosibleBloque >= numBytes){
			for(int p=0; p < numBytes; p++){
				buffer += bufferLeer[puntero];
				//Vamos reduciendo los bytes que nos quedan por leer y aumentando el puntero
				numBytes--;
				puntero++;
				bytesLeidos++;
			}
			SB.inodos[posicion].punteroBloque = punteroBloque;
			SB.inodos[posicion].puntero = puntero;
			return bytesLeidos;
		}
		
		for(int p=0; p < bytesPosibleBloque; p++){
			buffer += bufferLeer[puntero];
			//Vamos reduciendo los bytes que nos quedan por leer y aumentando el puntero
			numBytes--;
			puntero++;
			bytesLeidos++;
		}
		//Acabamos de leer un bloque y ponemos el puntero al inicio del siguiente
		puntero = 0;

	}
	SB.inodos[posicion].punteroBloque = punteroBloque - 1; //-1 ya que se ha salido del bucle porque no habia más bloques
	SB.inodos[posicion].puntero = BLOCK_SIZE; //Ponemos el puntero al final ya que hemos leido hasta el final del fichero
	return bytesLeidos;

}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{

		//Comprobamos los parámetros
	if(fileDescriptor <= -1) perror("readFile: Descriptor de fichero negativo\n"); return -1;	
	if(numBytes <= -1) perror("readFile: Número de bytes a leer negativo\n"); return -1;
	if(numBytes == 0) return 0;

	int posicion = -1;
	//Comparamos el descriptor de fichero con el recibido por parámetro hasta encontrar el inodo correspondiente
	for(int i=0; i<SB.numinodos;i++){ //Desde i hasta numero de inodos
		if(SB.inodos[i].blocksAsocidos[0] == fileDescriptor){
			posicion = i;
			break;
		}
	}

	//Realizamos las comprobaciones necesarias

	if(posicion == -1) perror("readFile: No se ha encontrado el descriptor del fichero\n"); return -1;
	if(SB.inodos[posicion].isopen == FCLOSE) perror("readFile: El fichero no se puede leer ya que no está abierto\n"); return -1;
	//Si estamos al final del fichero, ya no podemos leer más
	if(SB.inodos[posicion].puntero == SB.inodos[posicion].filesize) return 0;

	//Duda sobre la extensión de ficheros

	if(SB.inodos[posicion].puntero + numBytes > SB.inodos[posicion].filesize) {
		numBytes = SB.inodos[posicion].filesize - SB.inodos[posicion].puntero;
	}
	//Miramos donde se encuentra el puntero y vamos recorriendo los bits
		//bread(DEVICE_IMAGE, SB.inodos[posicion].blocksAsocidos[i], buffer);
	return -1;
}


/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{

	int posicion = -1;
	//Comparamos el descriptor de fichero con el recibido por parámetro hasta encontrar el inodo correspondiente
	for(int i=0; i<SB.numinodos;i++){ //Desde i hasta numero de inodos
		if(SB.inodos[i].blocksAsocidos[0] == fileDescriptor){
			posicion = i;
			break;
		}
	}
	if(posicion == -1) perror("lseekFile: No se ha encontrado el descriptor del fichero\n"); return -1;

	//Comprobamos los 3 valores posibles del whence

	if( (whence != FS_SEEK_BEGIN) | (whence != FS_SEEK_END) | (whence != FS_SEEK_CUR)) return -1;
	//BEGIN ponemos el puntero al principio del archivo, END ponemos el puntero al final del archivo. 
	if (whence == FS_SEEK_BEGIN) {
		SB.inodos[posicion].puntero = 0;
		return 0;
	}
	if (whence == FS_SEEK_END) {
		SB.inodos[posicion].puntero = SB.inodos[posicion].filesize;
		return 0;
	}

	//Movemos el puntero el valor que tenga offset
	if (whence == FS_SEEK_CUR) {
		int final = SB.inodos[posicion].puntero + offset;
		if ((final < 0) | (final > SB.inodos[posicion].filesize)) return -1;
		SB.inodos[posicion].puntero = final;
		return 0;		
	}

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
	//Falta comprobar segun tamaño del fichero que crc usar
	char* buffer = "";
	for(int i=0; i<SB.numinodos;i++){ //Desde i hasta numero de inodos
		if(strcmp(SB.inodos[i].filename, fileName) == 0){ 
			//Se considera error si se intenta realizar esta funcion con el archivo abierto
			if(SB.inodos[i].isopen == FOPEN){
				perror("checkFile: El archivo está abierto \n");
				return -2;
			}
			uint16_t prev_crc = SB.inodos[i].crc;

			uint16_t crc =CRC16((unsigned char*) buffer, sizeof(&buffer), prev_crc);

			if(prev_crc == crc){
				printf("checkFile: El archivo ha pasado el control de integridad\n");
				return 0;
			}else{
				printf("checkFile: El no ha pasado el control de integridad\n");
				return -1;
			}

		}
	}	
	

	return 0;
}
