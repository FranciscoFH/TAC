/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	auxiliary.h
 * @brief 	Headers for the auxiliary functions required by filesystem.c.
 * @date	01/03/2017
 */
int reservarBloqueLibre();
int reservarINodoLibre();
int buscarFichero(int fileDescriptor);
int escribirFichero(int inodo, int puntero, int punteroBloque, int numBytes, int bloquesEnInodo, void *buffer, int bytesEscritos);