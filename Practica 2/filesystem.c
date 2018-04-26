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



char bufferBlock[BLOCKSIZE];
superBloque SB;
iNodo inodos[MAX_FILES];
mapas mapa;

/*
Vamos a recorrer el mapa de bits del mapa de bloques hasta encontrar un 0, el cual será el primer bloque libre
En el momento en que lo encuentra, devuelve su posición + 1, ya que la posicion 3 del mapa corresponde al bloque 4
Si no encuentra nada significa que no hay bloques libres, por lo que devuelve un error
*/
int reservarBloqueLibre(void){
	//Antes de nada comprobamos que quedan bloques libres
	if(SB.numBloquesLibres == 0) {
		return -1;
	}
	int biteLibre;
	for(int i = 0; i< SB.numBloques; i++){
			biteLibre = bitmap_getbit(mapa.blockBitMap,i);
			if(biteLibre == 0){
				bitmap_setbit(mapa.blockBitMap,i,1);
				SB.numBloquesLibres --;
				return i+1;
			}	
	}
	return -1;
}

/*
Vamos a recorrer el mapa de bits del mapa de inodos hasta encontrar un 0, el cual será el primer inodo libre
En el momento en que lo encuentra, devuelve su posición + 1, ya que la posicion 3 del mapa corresponde al inodo 4
Si no encuentra nada significa que no hay inodos libres, por lo que devuelve un error
*/
int reservarInodoLibre(void){
	int inodoLibre;
	for(int i = 0; i< SB.numInodos; i++){
			inodoLibre = bitmap_getbit(mapa.inodosBitMap,i);
			if(inodoLibre == 0){
				bitmap_setbit(mapa.inodosBitMap,i,1);
				return i+1;
			}	
	}
	return -1;
}

/*
Vamos a recorrer el mapa de bits del mapa de inodos hasta encontrar un 0, el cual será el primer inodo libre
En el momento en que lo encuentra, devuelve su posición + 1, ya que la posicion 3 del mapa corresponde al inodo 4
Si no encuentra nada significa que no hay inodos libres, por lo que devuelve un error
*/
int buscarFichero(int fileDescriptor){
	for(int i=0; i<SB.numInodos;i++){ 
		if(inodos[i].bloquesAsociados[0] == fileDescriptor){
			return i;
		}
	}
	return -1;
}

/*
Funcion que recibe una serie de parametros y se encarga de escribir en disco los bloques en los que vaya escribiendo
*/
int escribirFichero(int inodo, int puntero, int punteroBloque, int numBytes, int bloquesEnInodo, void *buffer, int bytesEscritos){
	//Creamos un buffer que va a agregar todos los bloques que quedan por escribir

	char *bufferEscribir[BLOCKSIZE];

	int bytesPosibleBloque;

	for(int i = punteroBloque; i <= bloquesEnInodo ; i++){
		//Nos traemos el bloque en el que tenemos que escribir
		bread(DEVICE_IMAGE, inodos[inodo].bloquesAsociados[i-1], *bufferEscribir);
		//Guardamos los bytes que podemos escribir en ese fichero. Si el puntero esta en el byte 50 y el bloque es de 100, solo podremos escribir 50
		bytesPosibleBloque = BLOCKSIZE - puntero;
		//Si entramos aquí signfica que hemos vamos a poder escribir todos los bytes
		if(bytesPosibleBloque >= numBytes){
			memcpy(bufferEscribir+puntero, buffer + bytesEscritos ,bytesPosibleBloque);
			//Vamos reduciendo los bytes que nos quedan por escribir y aumentando el puntero
			puntero += numBytes;
			bytesEscritos += numBytes;
			inodos[inodo].punteroBloque = i;
			inodos[inodo].puntero = puntero;
			return bytesEscritos;
		}
		
		//Escribimos en el buffer que nos hemos traido, el contenido del buffer pasado por parametro
		memcpy(bufferEscribir+puntero, buffer + bytesEscritos ,bytesPosibleBloque);
		//Vamos actualizando el puntero y los contadores
		numBytes -= bytesPosibleBloque;
		puntero += bytesPosibleBloque;
		bytesEscritos += bytesPosibleBloque;
		//Una vez hemos acabado con un bloque, lo escribimos en el disco
		bwrite(DEVICE_IMAGE, inodos[inodo].bloquesAsociados[i-1], (char*) &bufferEscribir);
		//Acabamos de escribir un bloque y ponemos el puntero al inicio del siguiente
		puntero = 0;
		punteroBloque++;

	}
	return bytesEscritos;
}

/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */

int mkFS(long deviceSize)
{
	//Primero se comprueba que el tamañoo de disco se encuentre entre los limites permitidos
	if(deviceSize < MIN_DEVICE_SIZE || deviceSize > MAX_DEVICE_SIZE){
		perror("mkFS: El sistema de ficheros tiene que estar entrer 50KiB y 10MiB");
		return -1;
	}
	//Vamos a inicializar los atributos del super bloque
	SB.numInodos = MAX_FILES;										//El número de inodos será el máximo de números de ficheros
	SB.numBloques = (deviceSize + (BLOCK_SIZE - 1))/ BLOCK_SIZE -1;	//Calculamos el numero de bloques reales del disco y realizamos la función techo
	SB.primerBloqueINodo = 2;										//El primer bloque contiene el superBloque y los mapas
	SB.numBloquesLibres = SB.numBloques;							//Al principio, todos los bloques están libres
		
	/*
	Inicializamos los atributos de la estructura inodos. También metemos el mapa de bloques para ahorrar tiempo
	No inicializamos bloquesAsociados para no perder tiempo, ya que tenemos 'bloquesEnINodo' que nos indica los valores útiles
	*/
	int i;
	for(i = 0; i< SB.numInodos; i++){
		inodos[i].filesize = 0;
		inodos[i].crc = 0;
		inodos[i].punteroBloque = 0;
		inodos[i].bloquesEnInodo = 0;
		inodos[i].puntero = 0;
		mapa.blockBitMap[i] = 0; 
	}
	//Continuamos iniciando a 0 los bloques por si con el bucle anterior no se han completado
	for(int j = i; j < MAX_DEVICE_SIZE/BLOCKSIZE/8; j++){
		mapa.blockBitMap[j] = 0;
	}
	
	//Iniciamos a 0 el mapa de inodos. Lo hacemos en un bucle aparte para reducir en 8 el tamaño 
	for(int p=0; p< MAX_FILES/8; p++){
		mapa.inodosBitMap[p] = 0;
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

	//Accedemos a disco y leemos el super bloque
	int mountSB = bread(DEVICE_IMAGE, 1, (char*) &SB);
	if(mountSB == -1){
		perror("unmountFS: Error al montar el Sistema de Ficheros"); 
		return -1;
	}

	/* Ahora procedemos a escribir en disco todos los inodos. Cada inodo ocupa un bloque y empezamos desde el 2 ya que el 1 es del superbloque
	   Además, vamos realizando las compobaciones de que se están escribiendo bien
	*/
	for(int i=2; i <= SB.numInodos+1; i++){
		int mountSB = bread(DEVICE_IMAGE, i, (char*) &inodos[i-2]);
		if(mountSB == -1){
			perror("unmountFS: Error al montar el Sistema de Ficheros"); 
			return -1; 
		}
	}

	printf("mountFile: Montaje del dispositivo realizado correctamente \n");
	return 0;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	//Primero comprobamos que todos los ficheros estan cerrados.
	for(int i = 0; i< SB.numInodos; i++){
		if(inodos[i].isopen == FOPEN){
			perror("unmountFS: Hay ficheros abiertos");
			return -1;
		}
	}

	//El primer bloque pertenece al superbloque y los mapas, por lo que lo reservamos
	int primero = reservarBloqueLibre(); 

	if(primero != 1){
		perror("mkFS: El superBloque y los mapas no están en el primero");
		return -1;
	}
	//Escribimos en disco las estructuras y comprobamos que se han escrito correctamente
	int unmountSB = bwrite(DEVICE_IMAGE, 1, (char*) &SB);
	if(unmountSB == -1){
		perror("unmountFS: Error al desmontar el Sistema de Ficheros"); 
		return -1;
	}
	unmountSB = bwrite(DEVICE_IMAGE, 1, (char*) &mapa);
	if(unmountSB == -1){
		perror("unmountFS: Error al desmontar el Sistema de Ficheros"); 
		return -1;
	}
	/* Ahora procedemos a escribir en disco todos los inodos. Cada inodo ocupa un bloque y empezamos desde el 2 ya que el 1 es del superbloque
	   Además, vamos realizando las compobaciones de que se están escribiendo bien
	*/
	for(int i=2; i <= SB.numInodos+1; i++){
		unmountSB = bwrite(DEVICE_IMAGE, i, (char*) &inodos[i-2]);
		if(unmountSB == -1){
		perror("unmountFS: Error al desmontar el Sistema de Ficheros"); 
		return -1;
	}
	}

	printf("unmountFile: Desmonte del dispositivo realizado correctamente \n");
	return 0;
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

	//Primero descartamos que el fichero con ese nombre no existe
	for(int i=0; i<SB.numInodos;i++){ 
		if(strcmp(inodos[i].filename, fileName) == 0){ 
			perror("createFile: Ya existe un fichero con el mismo nombre\n");
			return -1;
		}
	}

	//Resersavamos el primer inodo libre que encontremos. Nos devuelve la posición del primer inodo libre 

	int inodo = reservarInodoLibre();
	if(inodo == -1) {
		perror("createFile: No hay inodos libres\n");
		return -1;
	}

	//Resersavamos el primer bloque libre que encontremos. Nos devuelve la posición del primer bloque libre

	int bloque = reservarBloqueLibre();
	if(bloque == -1) {
		perror("createFile: No hay bloques libres\n");
		return -1;
	}
	
	//Habiendo descartado los errores para poder crear el fichero, se procede a su inicialización
	inodos[inodo].isopen = FCLOSE; //Se marca el fichero como cerrado
	inodos[inodo].punteroBloque = bloque; //el puntero se encuentra en el primer bloque
	strcpy(inodos[inodo].filename, fileName); //Nombre del fichero el recibido por parametro
	inodos[inodo].crc = 26897; //Valor correspondiente a un crc de un fichero vacio
	inodos[inodo].bloquesEnInodo = 1; //Al crearlo, solo está formado por 1 bloque
	inodos[inodo].bloquesAsociados[0] = bloque;	//Asociamos el bloque al inodo. El bloque de la posición 0 funcionará como descriptor de fichero

	//Escribimos en el disco el nuevo fichero creado
	bwrite(DEVICE_IMAGE, inodos[inodo].bloquesAsociados[0], (char*) &bufferBlock);
	printf("createFile: Fichero %s creado con exito \n", inodos[inodo].filename);
	return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	//Busco el fichero, reinicio sus atributos y restauro las posiciones en el imap e iblock
	for(int i=0; i<SB.numInodos;i++){ 
		if(strcmp(inodos[i].filename, fileName) == 0){ 
			if(inodos[i].isopen == FOPEN){
				perror("removeFile: El fichero no puede ser borrado porque está abierto \n");
				return -2;
			}

			//Limpiamos el contenido del inodo
			memset(&(inodos[i].filename), 0, MAX_LONGNAME);
			inodos[i].filesize = 0; 
			inodos[i].isopen = -1; 
			inodos[i].crc = 0; 
			inodos[i].punteroBloque = 0; 
			inodos[i].puntero = 0; 

			//Marcamos como libre su posición en el mapa de inodos
			bitmap_setbit(mapa.inodosBitMap,i,0);

			/*Recorremos los bloques que lo forman para borrar su información y poner un 0 en el mapa de bits de bloque
			Además, para asegurnos que ninguna persona ajena acceda a la información olvidada en el bloque (ya que con poner el bloque a libre no se borra el contenido,
			le enviamos un buffer vacio */

			for(int t = 0; t<inodos[i].bloquesEnInodo; t++){
				bitmap_setbit(mapa.blockBitMap,inodos[i].bloquesAsociados[t] - 1,0);			
				bwrite(DEVICE_IMAGE, inodos[i].bloquesAsociados[0], (char*) &bufferBlock);
				SB.numBloquesLibres ++;
			}
			inodos[i].bloquesEnInodo = 0;

			printf("removeFile: Fichero %s borrado con exito \n", inodos[i].filename);
			return 0;
		}
	}
	perror("removeFile: El fichero no pudo ser borrado porque no existe \n");
	return -1;
}


/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
	//Verificamos el estado del fichero
	int verificacion = checkFile(fileName);
	if(verificacion == -1) {
		perror("openFile: El fichero esta corrupto \n"); 
		return -2;
	}
	if(verificacion == -2) {
		perror("openFile: El fichero esta abierto y no se puede verificar \n"); 
		return -2;
	}
	//Buscamos el fichero y lo abrimos.

	for(int i=0; i<SB.numInodos;i++){ 
		if(strcmp(inodos[i].filename, fileName) == 0){ 
			//Consideramos que abrir un archivo ya abierto es un error
			if(inodos[i].isopen == FOPEN){
				perror("openFile: El archivo ya estaba abierto \n");
				return -2;
			}
			else{
				//Abrimos el fichero y ponemos su puntero de lectura al principio
				inodos[i].isopen = FOPEN;
				inodos[i].puntero = 0;
				inodos[i].punteroBloque = inodos[i].bloquesAsociados[0];
				return inodos[i].bloquesAsociados[0];
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
	//Buscamos el inodo correspondiente a ese descriptor
	int inodo = buscarFichero(fileDescriptor);
	if(inodo == -1) {
		perror("closeFile: No se ha encontrado el descriptor del fichero\n"); 
		return -1;
	}
	//Comprobamos si el archivo ya estaba cerrado
	if(inodos[inodo].isopen == FCLOSE){
		perror("openFile: El archivo ya estaba cerrado \n");
		return -1;
	}
	else{
		//Cerramos el fichero 
		inodos[inodo].isopen = FCLOSE;
		return 0;
	}
	
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	//Variable que va a llevar la cuenta de los bytes leidos reales
	int bytesLeidos = 0;
	//Comprobamos los parámetros
	if(fileDescriptor <= -1) {
		perror("readFile: Descriptor de fichero negativo\n"); 
		return -1;	
	}
	if(numBytes <= -1) {
		perror("readFile: Número de bytes a leer negativo\n"); 
		return -1;
	}
	if(numBytes == 0) {
		return 0;
	}
	//Buscamos el inodo correspondiente
	int inodo = buscarFichero(fileDescriptor);

	//Realizamos las comprobaciones necesarias

	if(inodo == -1) {
		perror("readFile: No se ha encontrado el descriptor del fichero\n"); 
		return -1;
	}
	if(inodos[inodo].isopen == FCLOSE){
		perror("readFile: El fichero no se puede leer ya que no está abierto\n"); 
		return -1;
	}

	//Guardamos los atributos que vamos a necesitar
	uint16_t punteroBloque = inodos[inodo].punteroBloque; //Bloque en el que se encuentra el puntero
	uint16_t bloquesEnInodo = inodos[inodo].bloquesEnInodo; //Numero de bloques que tiene el inodo
	uint16_t puntero = inodos[inodo].puntero; //Posición del puntero
	uint16_t bloquesEnInodoRestantes = bloquesEnInodo - punteroBloque + 1; //Calculamos los bloques que todavia no hemos leido

	//Si estamos en el ultimo bloque del fichero, y el puntero está al final
	int posicionFinal = inodos[inodo].filesize % BLOCK_SIZE;
	if((bloquesEnInodoRestantes == 1) && (puntero ==  posicionFinal)) {
		return 0;
	}
	//Creamos un buffer donde guardaremos el bloque traido de disco
	char *bufferLeer[BLOCKSIZE];

	//Variable que guarda los bytes que podemos copiar de un bloque
	int bytesPosibleBloque;

	for(int i = punteroBloque; i <= bloquesEnInodo ; i++){
		//Nos traemos el bloque que vamos a leer al bufferLeer
		bread(DEVICE_IMAGE, inodos[inodo].bloquesAsociados[i-1], *bufferLeer);
		//Guardamos los bytes que podemos leer en ese fichero. Si el puntero esta en el byte 50 y el bloque es de 100, solo podremos leer 50
		bytesPosibleBloque = BLOCKSIZE - puntero;
		//Si estamos en el último bloque, solo podemos leer hasta el final del fichero y no hasta el final del bloque
		if(i == bloquesEnInodo) bytesPosibleBloque = posicionFinal - puntero;
		//Si entramos aquí signfica que hemos vamos a poder leer todos los bytes
		if(bytesPosibleBloque >= numBytes){
			memcpy(buffer,bufferLeer + puntero ,numBytes);
			//Vamos reduciendo los bytes que nos quedan por leer y aumentando el puntero
			puntero += numBytes;
			bytesLeidos += numBytes;
			inodos[inodo].punteroBloque = i;
			inodos[inodo].puntero = puntero;
			return bytesLeidos;
		}
		
		//Escribimos en el buffer todos los bytes del bloque restantes desde el puntero 
		memcpy(buffer,bufferLeer + puntero ,bytesPosibleBloque);
		//Vamos actualizando el puntero y los contadores
		numBytes -= bytesPosibleBloque;
		puntero += bytesPosibleBloque;
		bytesLeidos += bytesPosibleBloque;

		//Acabamos de leer un bloque y ponemos el puntero al inicio del siguiente
		puntero = 0;
		punteroBloque++;

	}
	//Si llegamos aquí es que nos poden leer más bytes de los que tenemos
	inodos[inodo].punteroBloque = punteroBloque; 
	inodos[inodo].puntero = posicionFinal; //Ponemos el puntero al final ya que hemos leido hasta el final del fichero
	return bytesLeidos;

}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	//Variable que guardará los bytes escritos reales
	int bytesEscritos = 0;
	//Comprobamos los parámetros
	if(fileDescriptor <= -1) {
		perror("writeFile: Descriptor de fichero negativo\n"); 
		return -1;	
	}
	if(numBytes <= -1) {
		perror("writeFile: Número de bytes a escribir negativo\n"); 
		return -1;
	}
	if(numBytes == 0) {
		return 0;
	}

	int inodo = buscarFichero(fileDescriptor);

	//Realizamos las comprobaciones necesarias

	if(inodo == -1) {
		perror("writeFile: No se ha encontrado el descriptor del fichero\n"); 
		return -1;
	}
	if(inodos[inodo].isopen == FCLOSE) {
		perror("writeFile: El fichero no se puede escribir ya que no está abierto\n"); 
		return -1;
	}

	uint16_t punteroBloque = inodos[inodo].punteroBloque; //Bloque en el que se encuentra el puntero
	uint16_t bloquesEnInodo = inodos[inodo].bloquesEnInodo; //Numero de bloques que tiene el inodo
	uint16_t puntero = inodos[inodo].puntero; //Posición del puntero
	uint16_t bloquesEnInodoRestantes = bloquesEnInodo - punteroBloque + 1; //Calculamos los bloques que todavia no hemos leido

	//Si estamos en el ultimo bloque del fichero, y el puntero está al final
	int posicionFinal = inodos[inodo].filesize % BLOCK_SIZE;
	if((bloquesEnInodoRestantes == 1) && (puntero ==  posicionFinal)) {
		return 0;
	}
	//Llamamos a la funcion escribir fichero la cual nos va guardando en el disco cada vez que terminemos de escribir en un bloque
	//Nos devuelve los bytesEscritos reales en el buffer
	bytesEscritos = escribirFichero(inodo, puntero, punteroBloque, numBytes, bloquesEnInodo, buffer, bytesEscritos);

	//Si se cumple esta condición es que hemos escrito todos los bytes sin necesidar de aumentar el tamaño
	if(bytesEscritos == numBytes) {
		return bytesEscritos;
	}
	//Si llegamos aquí significa que hay mas bytes por escribir pero no tenemos más bloques asignados, por lo que tenemos que asignar más.

	int bloque;

	//Vamos a realizar un buble que no pare hasta que hayamos escrito todos los bytes,hasta que no queden mas bloques por asignar o hasta que el fichero no acepte más bloques
	while((SB.numBloquesLibres != 0) | (inodos[inodo].bloquesEnInodo <= MAX_FILE_SIZE/BLOCK_SIZE)){ 
		//Reservamos un bloque en inodo y aumentamos en uno su atributo
		bloque = reservarBloqueLibre();
		bloquesEnInodo ++;
		//Guardamos el nuevo bloque en el array de bloques del inodo
		inodos[inodo].bloquesAsociados[bloquesEnInodo-1] = bloque;
		//Como la variable numBytes no se ha actualizado dentro de la función escribir, ponemos su nuevo valor real
		numBytes -= bytesEscritos;
		//Si estamos en el bucle es porque hemos creado un nuevo bloque, por lo que el puntero de posicion va a 0 y el punteroBloque es igual que el bloquesEnInodo
		bytesEscritos = escribirFichero(inodo, 0, bloquesEnInodo, numBytes, bloquesEnInodo, buffer, bytesEscritos);
		//Si hemos escrito todos los bytes, salimos de la función
		if(bytesEscritos == numBytes) {
			return bytesEscritos;
		}
	}
	return bytesEscritos;
}


/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{

	int inodo = buscarFichero(fileDescriptor);

	//Realizamos las comprobaciones necesarias

	if(inodo == -1) {
		perror("lseekFile: No se ha encontrado el descriptor del fichero\n"); 
		return -1;
	}
	//Comprobamos los 3 valores posibles del whence

	if( (whence != FS_SEEK_BEGIN) | (whence != FS_SEEK_END) | (whence != FS_SEEK_CUR)) {
		return -1;
	}
	//BEGIN ponemos el puntero al principio del archivo, END ponemos el puntero al final del archivo. 
	//Hay que modificar el atributo puntero a Bloque también.
	if (whence == FS_SEEK_BEGIN) {
		inodos[inodo].punteroBloque = inodos[inodo].bloquesAsociados[0];
		inodos[inodo].puntero = 0;
		return 0;
	}
	if (whence == FS_SEEK_END) {
		inodos[inodo].punteroBloque = inodos[inodo].bloquesAsociados[inodos[inodo].bloquesEnInodo-1];
		inodos[inodo].puntero = inodos[inodo].filesize % BLOCK_SIZE;
		return 0;
	}

	//Movemos el puntero el valor que tenga offset
	if (whence == FS_SEEK_CUR) {
		if(offset == 0) {
			return 0;
		}

		//Si es positivo y mayor que el espacio actual, significa que no hay que movernos del bloque. De lo contrario sí
		int espacioActual = BLOCK_SIZE - inodos[inodo].puntero;
		//Bytes que quedan en el fichero por leer
		int quedan = inodos[inodo].filesize - (inodos[inodo].puntero - (BLOCK_SIZE * (inodos[inodo].punteroBloque - 1)));
		if(offset >=0){
			//Comprobamos los bytes que nos quedan por leer para comprobar que no posicionamos el puntero fuera de los límites
			if(quedan < offset) {
				perror("lseekFile: offset fuera de los límites\n"); 
				return -1;
			}
			if (espacioActual >= offset){
				inodos[inodo].puntero += offset;
				return 0;	
			}
			//Significa que tenemos que avanzar 1 o más bloques
			else{
				offset -= espacioActual+1;
				while(BLOCK_SIZE <= offset){
					offset -= BLOCK_SIZE;
					inodos[inodo].punteroBloque++;
				}
				inodos[inodo].puntero = offset;
				return 0;
			}
		}
		//Si es negativo pero el puntero es mayor, significa que seguimos en el mismo bloque. Sino, hay que cambiarlo
		else{
			//Comprobamos los bytes que hemos leido para comprobar que no posicionamos el puntero fuera de los límites
			int leido = inodos[inodo].filesize - quedan;
			if(leido < offset) {
				perror("lseekFile: offset fuera de los límites\n"); 
				return -1;
			}
			espacioActual = inodos[inodo].puntero + offset;
			if(espacioActual >= 0){
				inodos[inodo].puntero += offset;
				return 0;	
			}	
			else{
				//Leemos los bytes que habían en ese fichero y retrocedemos al anterior. Añadiendo un 1 hacemos como que el puntero está al final del bloque anterior
				offset -= espacioActual+1;
				while(BLOCK_SIZE <= offset){
					offset -= BLOCK_SIZE;
					inodos[inodo].punteroBloque--;
				}
				inodos[inodo].puntero = BLOCK_SIZE - offset;
				return 0;
			}


		}

	}
	perror("lseekFile: Ha habido un error en algo\n"); 
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


int checkFile(char *fileName){
/*
	//Falta comprobar segun tamaño del fichero que crc usar
	char* buffer = "";
	for(int i=0; i<SB.numInodos;i++){ //Desde i hasta numero de inodos
		if(strcmp(inodos[i].filename, fileName) == 0){ 
			//Se considera error si se intenta realizar esta funcion con el archivo abierto
			if(inodos[i].isopen == FOPEN){
				perror("checkFile: El archivo está abierto \n");
				return -2;
			}
			uint16_t prev_crc = inodos[i].crc;

			uint16_t crc = CRC16((unsigned char*) buffer, sizeof(&buffer), prev_crc);

			if(prev_crc == crc){
				printf("checkFile: El archivo ha pasado el control de integridad\n");
				return 0;
			}else{
				printf("checkFile: El no ha pasado el control de integridad\n");
				return -1;
			}

		}
	}	
	

*/
	return 0;
}
