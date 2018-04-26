/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	test.c
 * @brief 	Implementation of the client test routines.
 * @date	01/03/2017
 */

#include <stdio.h>
#include <string.h>
#include "include/filesystem.h"


// Color definitions for asserts
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE   "\x1b[34m"

#define N_BLOCKS	25					// Number of blocks in the device
#define DEV_SIZE 	N_BLOCKS * BLOCK_SIZE	// Device size, in bytes
#define BIG_DEVICE 10485760


int main() {

	//char buffer[BLOCK_SIZE];

	//Las pruebas que se suceden en este codigo estan comentadas ya que hemos necesitado ir una por una para probar funcionalidad por funcionalidad
	
	/*mkFS(BIG_DEVICE);
	mountFS();
	createFile("test.txt");
	removeFile("test.txt");

	createFile("pruebaCorruptos.txt");
	checkFile("pruebaCorruptos.txt");
	openFile("test.txt");
	closeFile(0);

	*/

	// mkFS(2); //prueba limite inferior de bytes

	/*createFile("prueba.txt");
	createFile("prueba.txt");//Forzamos fallo por archivo existente

	*/

	//mkFS(BIG_DEVICE);
	//createFile("estoEsUnaPruebaQueExcedeLosLimitesDeCaractessssssssssssss.txt"); //Longitud de caracteres mas alta de la permitida

	//mkFS(BIG_DEVICE);
	//createFile("prueba.png"); //Forzamos crear un formato incorrecto

	//mkFS(BIG_DEVICE);
	//removeFile("pruebaNoExiste.txt");

	//Abrir dos veces el mismo fichero
	//mkFS(BIG_DEVICE);
	//createFile("test.txt");
	//openFile("test.txt");
	//openFile("test.txt");


	//mkFS(BIG_DEVICE);
	//createFile("test.txt");
	//openFile("test.txt");
	//writeFile(0,buffer,2000);


	





	///////

	return 0;
}
