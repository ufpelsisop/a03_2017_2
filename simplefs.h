#ifndef _SIMPLEFS_H_
#define _SIMPLEFS_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "simplegrade.h"

#define SUCESSO 1
#define FALHA 0

#define LEITURA 0
#define ESCRITA 1
#define LEITURAESCRITA 2

typedef int indice_arquivo_t;
typedef int indice_fs_t;

/** Inicializa o arquivo no sistema de arquivos hospedeiro.
 *
 *  Deve ser chamada apenas uma vez para um determinado sistema de arquivos.
 *  Caso o nome de arquivo já exista, deve retornar erro.
 *  Também deve retornar erro caso o número de blocos não seja o suficiente
 *  para armazenar os metadados pelo menos.
 *
 *  @param arquivo nome do arquivo no sistema de arquivos hospedeiro
 *  @param blocos número de blocos do arquivo
 *  @return SUCESSO ou FALHA
 */
int initfs(char * arquivo, int blocos);


/** Abre um sistema de arquivos.
 *
 *  @param nome do arquivo no sistema de arquivos hospedeiro
 *  @return file handle do descritor de sistema de arquivos ou FALHA
 */

indice_fs_t openfs(char * arquivo);

/** Fecha um sistema de arquivos.
 *
 *  @param file handle sistema de arquivos hospedeiro
 */

void closefs(indice_fs_t vfs);

/** Abre um arquivo.
 *
 *
 * @param fs o file handle do sistema de arquivos
 * @param nome o arquivo a ser aberto
 * @param acesso LEITURA, ESCRITA ou LEITURAESCRITA
 * @return índice do arquivo aberto, FALHA se não abrir
 */
indice_arquivo_t f_open(indice_fs_t fs, char * nome,  int acesso);

/** Fecha um arquivo.
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @return SUCESSO OU FALHA
 */
int f_close(indice_arquivo_t arquivo);

/** Lê bytes de um arquivo aberto.
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @param tamanho número de bytes a serem lidos
 * @param buffer ponteiro para buffer onde serão armazenados os bytes lidos
 * @return número de bytes lidos
 */
uint32_t f_read(indice_arquivo_t arquivo, uint32_t tamanho, char *buffer);

/** Escreve bytes em um arquivo aberto.
 *
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @param tamanho número de bytes a serem escritos
 * @param buffer ponteiro para conteúdo a ser escrito
 * @return SUCESSO ou FALHA
 */
int f_write(indice_arquivo_t arquivo, uint32_t tamanho, char *buffer);

/** Apaga um arquivo e o fecha.
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @return SUCESSO ou FALHA
 */

int f_delete(indice_arquivo_t arquivo);

/** Modifica a posição atual de leitura ou escrita do arquivo
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @param seek deslocamento em relação ao início do arquivo
 * @return SUCESSO ou FALHA
 */
int f_seek(indice_arquivo_t arquivo, uint32_t seek);

/** Retorna o tempo de criação 
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @return tempo
 */
time_t f_creation(indice_arquivo_t arquivo);

/** Retorna o tempo em que o arquivo foi modificado
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @return tempo
 */
time_t f_last_modified(indice_arquivo_t arquivo);

#endif
