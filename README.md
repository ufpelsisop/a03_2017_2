# Trabalho 3 - Sistemas Operacionais

## Declaração

Declaramos que o presente trabalho contém código desenvolvido exclusivamente por nós e que não foi compartilhado de nenhuma forma com terceiros a não ser o professor da disciplina. Compreendo que qualquer violação destes princípios será punida rigorosamente de acordo com o Regimento da UFPEL.

(Preencha com seus dados)

- Nome completo: 
- Username do Github: 
- Email @inf: 

(Em caso de dupla)
- Nome completo: 
- Username do Github: 
- Email @inf: 

## Descrição

Para o trabalho 3 um sistema de arquivos será implementado em nível de usuário para sistemas de acordo com POSIX, especialmente Linux. 

Normalmente, para implementar um sistema de arquivos é necessário ter acesso direito aos dispositivos. Para simplificar a implementação e depuração, um arquivo grande sobre um outro sistema de arquivos será usado. Este arquivo será usado para armazenar os descritores de arquivos e os blocos que os compõem.

## Estrutura do sistema de arquivos

O sistema de arquivos está organizado da seguinte forma:

- _Boot block_: 512 bytes que conteria o _bootloader_ e que não deve ser usado;
- _Super block_: 1 bloco que contém informações sobre o sistema de arquivos (informação deve estar no início do bloco);   
- _i-node bitmap_: blocos contendo um bit para cada bloco de dados dizendo se estão livres (0) ou ocupados (1);
- _directory_: blocos contendo 256 entradas com descritores de arquivos;
- _data_: blocos usados para armazenar dados de arquivos

# Super block

        struct super_block{
            uint32_t total_blocks; // inclui todos os blocos do sistema de arquivo
            uint32_t bitmap_blocks; // numero de blocos alocados para i-node bitmaps
            uint32_t directory_blocks; // numero de blocos alocados para directory
            uint32_t data_blocks; // numero de blocos alocados para data
            uint32_t dirty_flag; // 0 se o sistema de arquivos foi fechado corretamente, 1 se nao
            uint32_t block_size; // tamanho do bloco (deve ser potencia de 2 e maior ou igual a 512)
        };


# Diretório

A implementação deverá prover um diretório de um nível com 256 entradas. Os descritores deverão estar armazenados em formato binário, i.e., um descritor pode estar dividido em dois blocos. Cada descritor de arquivo deverá conter:

        struct fd{
            char name[256]; // nome do arquivo com no maximo 255 caracteres ISO-8859-1 (todos em zero se a entrada nao estiver sendo usada)
            time_t created; // timestamp em que foi criado
            time_t modified; // timestamp da ultima modificacao
            uint32_t size; // tamanho do arquivo
            uint32_t index_block; //id do primeiro bloco de índices
        };

Cada bloco de índice, armazenados junto da seção _data_, é composto por um vetor de índices de 32 bits. O último índice do bloco aponta para o próximo bloco de índices do arquivo, ou é zero. 

Toda a implementação da biblioteca do aluno deverá estar contida em um arquivo chamado _simplefs.c_ (ou _simplefs.cpp_).

## Funções a serem implementadas

Detalhes dos parâmetros e da semântica das funções estão descritas na documentação do arquivo crypfs.h. Outras funções podem ser implementadas, mas não devem ser visíveis aos usuários finais. O arquivo crypfs.h **NÃO** deve ser modificado.

### initfs()

Função para inicializar um arquivo no sistema de arquivos hospedeiro com os blocos necessários.

Aloca os blocos, incluindo os blocos de metadados da estrutura do diretório. Deve falhar se não houver espaço suficiente no sistema de arquivo para todos os blocos de meta-dados mais um para dados.

### openfs()

Função usada para abrir um sistema de arquivos já inicializado por initfs().


### closefs()

Função usada para fechar um sistema de arquivos.


### f_open()

Abre um arquivo. Deve ser chamada antes de qualquer outra operação com o arquivo e especificar se o arquivo é aberto para leitura, escrita ou ambos.
Retorna um file handler, NULL se houve erro.

### f_read()

Lê **n** bytes de um arquivo. Retorna o número de bytes lidos.

### f_write()

Escreve **n** bytes. Retorna **1** se sucesso, **0** se erro.

### f_delete()

Remove o arquivo e o fecha. O arquivo deve estar aberto.  Retorna **1** se sucesso, **0** se erro.

### f_close()

Fecha o arquivo.  Retorna o número da versão. 

## f_seek()

Muda a posição atual no acesso ao arquivo. Retorna **1** se sucesso, **0** se erro.

## f_creation(), f_last_modified()

Retornam o tempo em que o arquivo foi criado e escrito pela última vez, respectivamente. 

## Casos de Teste

O caso de teste deve criar um sistema de arquivos de nome **fff.bin**.

Os casos de teste deverão demonstrar todas as funcionalidades implementadas. Para tanto, deve ser usado a biblioteca **simplegrade**.

As flags **-std=c99 -Wall -Werror -Wextra** são obrigatórias para C, **-std=c++11 -Wall -Werror -Wextra** para C++. O código deve compilar e executar no Travis. 

O Makefile deverá incluir uma regra _grade_ que compila e executa o teste do professor (test.c) e uma regra _teste_ que compila e executa o teste do aluno (mytest.c ou mytest.cpp). O Travis deverá ser configurado para executar a regra _grade_. 


## Avaliação

Serão pontuados:

- Implementação: **8 pontos**
- Casos de teste dos alunos: **2 pontos** (proporcionais à nota da implementação)



## Cronograma

- Dúvidas em aula: **06/02/2018**
- Entrega no Github e Travis: **16/02/2018**


