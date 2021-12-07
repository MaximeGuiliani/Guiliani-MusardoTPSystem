
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "sgf-header.h"
#include "sgf-impl.h"

static DIRECTORY directory;

/************************************************************
 Initialiser le module.
 ************************************************************/

void init_sgf_dir()
{
    assert(sizeof(DIRECTORY) <= sizeof(BLOCK));
    read_dir_block(&directory);
}

/************************************************************
 rechercher et renvoyer l'adresse du descripteur d'un fichier.
 Cette fonction renvoie -1 en cas d'erreur.
 ************************************************************/

int find_inode(const char *name)
{
    unsigned j;

    for (j = 0; j < DIR_SIZE; j++)
    {
        if (directory.files[j].adr_inode > 0)
        {
            if (strcmp(directory.files[j].name, name) == 0)
            {
                return (directory.files[j].adr_inode);
            }
        }
    }

    return (-1);
}

/************************************************************
 Ajouter un couple <name,inode> au répertoire. Si un couple
 existe déjà, la fonction renvoie l'adresse du descripteur
 et 0 dans le cas contraire.
 Elle renvoie en -1 en cas d'erreur.
 ************************************************************/

int add_inode(const char *name, int inode)
{
    int j, nj;

    if ((strlen(name) + 1) > FILENAME_SIZE)
    {
        return (-1);
    }

    nj = -1;
    for (j = 0; j < DIR_SIZE; j++)
    {
        if (directory.files[j].adr_inode > 0)
        {
            if (strcmp(directory.files[j].name, name) == 0)
            {
                int oldinode = directory.files[j].adr_inode;
                directory.files[j].adr_inode = inode;
                write_dir_block(&directory);
                return (oldinode);
            }
        }
        else
        {
            nj = j;
        }
    }

    if (nj >= 0)
    {
        directory.files[nj].adr_inode = inode;
        strcpy(directory.files[nj].name, name);
        write_dir_block(&directory);
        return (0);
    }

    panic("Impossible de créer %s : répertoire plein\n", name);

    return (-1);
}

/************************************************************
 Effacer un couple <name,inode> au répertoire.
 ************************************************************/

void delete_inode(const char *name)
{
    int j;

    for (j = 0; j < DIR_SIZE; j++)
    {
        if (directory.files[j].adr_inode > 0)
        {
            if (strcmp(directory.files[j].name, name) == 0)
            {
                directory.files[j].adr_inode = 0;
                write_dir_block(&directory);
                return;
            }
        }
    }
}

/************************************************************
 Formater le disque et créer un répertoire vide.
 ************************************************************/

void create_empty_directory()
{
    int j;

    /* vider le bloc du répertoire et le sauver */
    directory.signature = SIGNATURE_FS;
    for (j = 0; j < DIR_SIZE; j++)
    {
        directory.files[j].adr_inode = 0;
        strcpy(directory.files[j].name, "");
    }
    write_dir_block(&directory);

    printf("create empty directory\n");
}

/************************************************************
 Lister les fichiers du répertoire avec leur taille.
 Informations :
  - inspirez vous de la fonction find_inode
  - pour chaque entrée, lire l'INODE et afficher le
    nom et la taille du fichier.
 ************************************************************/

void list_directory(void)
{
    unsigned j;
    for (j = 0; j < DIR_SIZE; j++)
    {
        if (directory.files[j].adr_inode > 0)
        {
            printf("\nAdresse  :  %d\n", directory.files[j].adr_inode);
            printf("Name  :  %s\n", directory.files[j].name);
            printf("Size  :  %d\n", read_inode(directory.files[j].adr_inode).size);
        }
    }
    
    //list_directory_impl();
}
