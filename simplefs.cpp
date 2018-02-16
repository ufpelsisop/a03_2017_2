#include "simplefs.h"
#include <iostream>
#include <cmath>
#include <fstream> 
#include <vector>
#include <cstdlib>
#include <cstring>
using namespace std;

struct dataBlocks{
    //vector<char>bytes; // bytes do bloco 
    char *bytes;
    uint32_t index;  // ponteiro para o proximo bloco, se precisar 
    int MAXBytes;   // tam bloco - 4 bytes
    int storedBytes;
};

// 4*6 = 26 bytes
struct super_block{
    uint32_t total_blocks; // inclui todos os blocos do sistema de arquivo
    uint32_t bitmap_blocks; // numero de blocos alocados para i-node bitmaps
    uint32_t directory_blocks; // numero de blocos alocados para directory
    uint32_t data_blocks; // numero de blocos alocados para data
    uint32_t dirty_flag; // 0 se o sistema de arquivos foi fechado corretamente, 1 se nao
    uint32_t block_size; // tamanho do bloco (deve ser potencia de 2 e maior ou igual a 512)
};

// 256+16+8 = 280 bytes
struct fd{
    char name[256]; // nome do arquivo com no maximo 255 caracteres ISO-8859-1 (todos em zero se a entrada nao estiver sendo usada)
    time_t created; // timestamp em que foi criado
    time_t modified; // timestamp da ultima modificacao
    uint32_t size; // tamanho do arquivo
    uint32_t index_block; //id do primeiro bloco de índices
    uint32_t permission; // escrita leitura leituraescrita (auxiliar para função f_open)
};

// 512+26+?+280+? = ?
struct fileSys{
    string name;
    int booBlock[512]; //: 512 bytes que conteria o _bootloader_ e que não deve ser usado;
    super_block sb; //: 1 bloco que contém informações sobre o sistema de arquivos (informação deve estar no início do bloco);   
    int *bitmap; //: blocos contendo um bit para cada bloco de dados dizendo se estão livres (0) ou ocupados (1);
    fd *dir; //blocos contendo 256 entradas com descritores de arquivos;
    dataBlocks *data; //: blocos usados para armazenar dados de arquivos
};

fileSys *fileSystem;
vector<fd> openFiles;
vector<fd> closedFiles;

/** Inicializa o arquivo no sistema de arquivos hospedeiro.
 *
 *  Deve ser chamada apenas uma vez para um determinado sistema de arquivos.
 *  Caso o nome de arquivo já exista, deve retornar erro.
 *  Também deve retornar erro caso o número de blocos não seja o suficiente
 *  para armazenar os metadados pelo menos.
 *
 *  @param arquivo nome do arquivo no sistema de arquivos hospedeiro
 *  @param blocos número de blocos do arquivo
 *  @param tam_bloco tamanho de um bloco (deve ser pot. de 2 e maior ou igual a 512)
 *  @return SUCESSO ou FALHA
 */
int initfs(char * arquivo, int blocos, int tam_bloco){
/** Abre um sistema de arquivos.
 *
 *  @return file handle do descritor de sistema de arquivos ou FALHA
 *  @param nome do arquivo no sistema de arquivos hospedeiro
 */

    if (tam_bloco<512) // blocos insuficientes
        return FALHA;

    fileSys *fSys = (fileSys*)malloc(sizeof(fileSys));
    super_block *superBlock = (super_block*)malloc(sizeof(super_block));
    superBlock->total_blocks = blocos;
    superBlock->directory_blocks = ceil((256*sizeof(fd))/tam_bloco);
    superBlock->bitmap_blocks = ceil(tam_bloco/blocos);
    superBlock->data_blocks = tam_bloco - 2 - superBlock->directory_blocks - superBlock->bitmap_blocks;
    superBlock->dirty_flag = 0; //nao foi fechado ainda
    superBlock->block_size = tam_bloco;
    
    fSys->name = arquivo;
    fSys->sb = *superBlock;   
    fileSystem = fSys;
    return SUCESSO;
}

    
/** Abre um sistema de arquivos.
 *  @param nome do arquivo no sistema de arquivos hospedeiro
 *  @return file handle do descritor de sistema de arquivos ou FALHA
 *
 */
indice_fs_t openfs(char * arquivo){
    if(fileSystem->name != arquivo)
        return FALHA;

    fileSystem->bitmap = (int*)calloc(fileSystem->sb.bitmap_blocks, sizeof(int));
    fileSystem->dir = (fd*)calloc(fileSystem->sb.directory_blocks, sizeof(fd));
    fileSystem->data = (dataBlocks*)calloc(fileSystem->sb.data_blocks, sizeof(dataBlocks));
    fileSystem->name = arquivo;
    for(int i=0;i<fileSystem->sb.data_blocks;++i){
        fileSystem->data[i].bytes = (char*)calloc(fileSystem->sb.block_size, sizeof(char)); //aloca espaço para bytes
        fileSystem->data[i].MAXBytes = fileSystem->sb.block_size - 4;  // 4 bytes pra ponteiro
        fileSystem->data[i].index = 0; // no more blocks 
        fileSystem->data[i].storedBytes = 0; // dados escritos
    }

    //fileSystem->bitmap[0] = 1; //boot
    //fileSystem->bitmap[1] = 1; //superblock
    
    return SUCESSO; // ???????????  i dont know 
}

/** Fecha um sistema de arquivos.
 *
 *  @param file handle sistema de arquivos hospedeiro
 */
void closefs(indice_fs_t vfs){
    //for(int i=0;i<fileSystem->sb.data_blocks;++i)
    //    free(&fileSystem->data[i]);
    free(fileSystem->data);
    free(fileSystem->bitmap);
    //for(int i=0;i<fileSystem->sb.data_blocks;++i)
    //    free(&fileSystem->dir[i]);
    free(fileSystem->dir);
    free(fileSystem);
}

/** Abre um arquivo.
 *
 *
 * @param fs o file handle do sistema de arquivos
 * @param nome o arquivo a ser aberto
 * @param acesso LEITURA, ESCRITA ou LEITURAESCRITA
 * @return índice do arquivo aberto, FALHA se não abrir
 */
indice_arquivo_t f_open(indice_fs_t fs, char * nome,  int acesso){
    fd newFile;
    int aux, dirIndex=-1;
    // cria arquivo /////////
    strcpy(newFile.name,(char*)nome);
    newFile.created = time(NULL);
    newFile.modified = time(NULL);
    newFile.size = 0;
    newFile.permission = acesso;
    /////////////////////
    ////// verifica se há blocos pra ele /////
    aux = firstFreeBlock(); //primeiro bloco livre
    if(aux!=-1)
        newFile.index_block = aux;
    else 
        return FALHA;
    
    //files.push_back(newFile); // fila de arquivos
    openFiles.push_back(newFile); // fila de arquivos abertos
    for(int i=0;i<fileSystem->sb.directory_blocks;++i){
        if(fileSystem->dir[i].created == 0){ //nao foi criado
            dirIndex=i;
            break;
        }
    }
    fileSystem->dir[dirIndex] = newFile; // add descritor no diretorio
    fileSystem->bitmap[aux] = 1; // seta bitmap
}

/** Fecha um arquivo.
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @return SUCESSO OU FALHA
 */
int f_close(indice_arquivo_t arquivo){ 
    if(arquivo > openFiles.size())
        return FALHA;

    closedFiles.push_back(openFiles[arquivo]);
    openFiles.erase(openFiles.begin()+arquivo);
    return SUCESSO;
}
/** Lê bytes de um arquivo aberto.
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @param tamanho número de bytes a serem lidos
 * @param buffer ponteiro para buffer onde serão armazenados os bytes lidos
 * @return número de bytes lidos
 */
uint32_t f_read(indice_arquivo_t arquivo, uint32_t tamanho, char *buffer){ //refazer
    uint32_t nBytes = 0;
    int i=0;
    //char *bufferAux = (char*)malloc(sizeof(char)*tamanho);
    fd *auxDir = &openFiles[arquivo]; // descritor do arquivo
    dataBlocks *auxData = &fileSystem->data[auxDir->index_block]; // bloco do arquivo 
    
    if(auxDir->permission == ESCRITA) //nao tem permissao para leitura
        return FALHA;

    for(int i=0;i<tamanho;++i){
        if(i>auxData->storedBytes)
            break;

        if(nBytes>auxData->MAXBytes){
            auxData = &fileSystem->data[auxData->index]; //proximo bloco
            i=0;
        }
        *buffer = auxData->bytes[i];
        ++buffer;
        ++nBytes;
        ++i;
    }
    // while(true){
    //     if(*buffer == '\n' || *buffer == '\0')
    //         break;
    //     if(nBytes > auxDir->size) //nao tem mais bytes pra ler
    //         break;    

    //     if(nBytes>auxData->MAXBytes){
    //         auxData = &fileSystem->data[auxData->index]; //proximo bloco
    //     }
    //     *buffer = auxData->bytes[i];
    //     ++buffer;
    //     ++nBytes;
    //     ++i;
    // }    
        
    return nBytes;
}

/** Escreve bytes em um arquivo aberto.
 *
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @param tamanho número de bytes a serem escritos
 * @param buffer ponteiro para conteúdo a ser escrito
 * @return SUCESSO ou FALHA
 */
int f_write(indice_arquivo_t arquivo, uint32_t tamanho, char *buffer){
    uint32_t nBytes = 0;
    int i=0, nextBlock;
    bool flag=false;
    fd *auxDir = &openFiles[arquivo]; // descritor do arquivo
    dataBlocks *auxData = &fileSystem->data[auxDir->index_block]; // bloco do arquivo 
    
    if(auxDir->permission==LEITURA) //nao tem permissao para escrita
        return FALHA;

    if(tamanho>auxData->MAXBytes*2) //se número requerido de bytes ultrapassa limite de 2 blocos
        return FALHA;

    if(arquivo>openFiles.size())
        return FALHA;

    while(nBytes <= tamanho){ //escreve tamanho de bytes que é passado pela função
        if(*buffer == '\n' || *buffer == '\0') // acabou buffer de entrada
            break;

        //////////////////////////////
        if(nBytes >= auxData->MAXBytes){
            if(flag==true) //esta procurando um terceiro bloco do arquivo
                return FALHA;

            if(auxData->index==0){
                nextBlock = firstFreeBlock();
                if(nextBlock != -1) //verifica se há blocos livres
                    auxData->index = nextBlock;
                else  //não há blocos livres
                    return FALHA;
            }
            auxData = &fileSystem->data[nextBlock]; //proximo bloco
            i=0;
            flag=true; //seta flag indicando que entrou no segundo bloco
        }
        ///////////////////////////

        auxData->bytes[i] = *buffer;
        //auxData->bytes.push_back(*buffer);
        ++buffer;
        ++nBytes;
        ++i;
        auxDir->size = nBytes;
        auxData->storedBytes = nBytes;
    }
    return SUCESSO;
}

/** Apaga um arquivo e o fecha.
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @return SUCESSO ou FALHA
 */
int f_delete(indice_arquivo_t arquivo){ //verificar essa caralha
    if(arquivo>openFiles.size())
        return FALHA;

    fd *removeDir = &fileSystem->dir[arquivo];
    dataBlocks *removeData = &fileSystem->data[removeDir->index_block];
    if(removeDir->size>removeData->MAXBytes){ //2 blocos
        cout << "essa fera ai meu" << endl;
        fileSystem->bitmap[removeDir->index_block] = 0; //indica que esta livre agora
        fileSystem->bitmap[removeData->index] = 0; //segundo bloco
    }else{ //1 bloco só
        //free(removeDir);
        //free(removeData);
        fileSystem->bitmap[removeDir->index_block] = 0; //indica que esta livre agora
    }

    closedFiles.push_back(openFiles[arquivo]);
    openFiles.erase(openFiles.begin()+arquivo);
    return SUCESSO;
}

/** Modifica a posição atual de leitura ou escrita do arquivo
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @param seek deslocamento em relação ao início do arquivo
 * @return SUCESSO ou FALHA
 */
int f_seek(indice_arquivo_t arquivo, uint32_t seek){
    if(arquivo>openFiles.size())
        return FALHA;

    fd *auxDir = &openFiles[arquivo];
    dataBlocks *auxData = &fileSystem->data[auxDir->index_block];
    
    if(seek>auxData->MAXBytes*2) //seek alem do arquivo
        return FALHA;

    if(seek>auxData->MAXBytes-auxData->storedBytes){
        auxData = &fileSystem->data[auxData->index];
        if(seek>auxData->MAXBytes-auxData->storedBytes){
            return FALHA;
        }else{
            return seek;
        }
    }else{
        return seek;
    }
}

/** Retorna o tempo de criação 
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @return tempo
 */
time_t f_creation(indice_arquivo_t arquivo){
    return openFiles[arquivo].created;
}

/** Retorna o tempo em que o arquivo foi modificado
 *
 * @param arquivo índice para a tabela de arquivos abertos
 * @return tempo
 */
time_t f_last_modified(indice_arquivo_t arquivo){
    return openFiles[arquivo].modified;
}

//  INUTIL //
//void readTest(char*arquivo, int tam_bloco){
//    ifstream file(arquivo, ios::in|ios::out|ios::binary);
//    super_block *sb = new super_block();
//    file.seekg(tam_bloco);
//    file.read((char*)&sb,sizeof(super_block));
//    cout << sb->total_blocks << endl;
//    cout << sb->block_size << endl;
//    file.close();
//}

int firstFreeBlock(){
    int numBlocks = fileSystem->sb.bitmap_blocks;
    for(int i=0;i<numBlocks;++i){
        if(fileSystem->bitmap[i]==0)
            return i;
    }
    return -1; // nao ha blocos disponiveis
}

//open files vector in each file system 
int main() {
    char *buffer = (char*)malloc(sizeof(char)*3);
    initfs((char*)"teste.txt",2,1024);    
    cout << "info:" << endl;
    cout << "total blocks " << fileSystem->sb.total_blocks << endl;
    cout << "bitmap blocks " << fileSystem->sb.bitmap_blocks << endl;
    cout << "directory blocks " << fileSystem->sb.directory_blocks << endl;
    cout << "data blocks " << fileSystem->sb.data_blocks << endl;
    cout << "dirty flag " << fileSystem->sb.dirty_flag << endl;
    cout << "block size " << fileSystem->sb.block_size << endl;
    cout << "fim" << endl;
    openfs((char*)"teste.txt");
    f_open(0,(char*)"catatau",LEITURAESCRITA);
    f_open(0,(char*)"banana",LEITURAESCRITA);
    cout << openFiles[0].name << endl;
    cout << openFiles[0].index_block << endl;
    cout << fileSystem->dir[1].name << endl;
    cout << fileSystem->dir[1].index_block << endl;
    f_write(0, 3, (char*)"ABCDEFGHIJKLMNOPQRSTUVXYZ");
    cout << "ok" << endl;
    cout << fileSystem->data[openFiles[0].index_block].index << endl;
    cout << fileSystem->data[openFiles[0].index_block].MAXBytes << endl;
    cout << fileSystem->data[openFiles[0].index_block].bytes[0] << endl;
    cout << f_read(0, 5, buffer) << endl;
    cout << "ok" << endl;
    cout << buffer << endl;
    cout << buffer[0] << endl;
    cout << buffer[1] << endl;
    cout << buffer[2] << endl;
    cout << "testa delete" << endl;
    cout << f_delete(0) << endl;
    cout << "fim teste" << endl;
    cout << openFiles.size() << endl; 
    cout << f_creation(0) << endl;
    closefs(0);
        
    return 0;
}


