
/************************************************************
 *
 *  FONCTIONS DE LECTURE/ECRITURE DANS UN FICHIER
 *
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sgf-header.h"
#include "sgf-impl.h"

void sgf_read_block(OFILE *file, int block_number);

/************************************************************
 *  Initialisation du SGF
 ************************************************************/

void init_sgf(void)
{
  init_sgf_fat();
  init_sgf_dir();
}

/************************************************************
 Lire un caractère dans un fichier ouvert. Cette fonction
 renvoie -1 si elle trouve la fin du fichier.

 Description :
   - si la fin du fichier est atteinte (comparaison entre ptr
     et la taille), renvoyer -1
   - si le buffer est vide (test à faire sur ptr % BLOCK_SIZE)
     appeler la fonction sgf_read_block pour le remplir
   - renvoyer le caractère dans la bonne position du buffer
     (position à calculer en fonction de ptr).
 ************************************************************/

int sgf_getc(OFILE *file){
  if (file->ptr >= file->inode.size){
    return -1;
  }
  if (file->ptr%BLOCK_SIZE == 0){
    sgf_read_block(file, file->ptr/BLOCK_SIZE);
  }
  return file->buffer[file->ptr++%BLOCK_SIZE];
}

/************************************************************
 Déplacer le pointeur associé au fichier ouvert en lecture
 (ptr donc) en utilisant le paramètre pos.

 Description :
   - vérifier le mode d'ouverture du fichier
   - tester le débordement (renvoyer -1 en cas d'échec)
   - si la position n'est pas un multiple de la taille des blocs
      - charger le bon bloc dans le buffer (fonction sgf_read_block)
   - affecter le pointeur courant
   - renvoyer 0

 Attention : prenez soin de limiter la lecture de bloc
 notamment quand la nouvelle position concerne le même bloc
 que la position précédente.
 ************************************************************/

int sgf_seek(OFILE *file, int pos)
{
  fprintf(stderr, "fonction sgf_seek() à terminer.\n");
  exit(EXIT_FAILURE);
}

/************************************************************
 Lire dans le "buffer" le bloc logique "block_number"
 dans le fichier ouvert "file".

 Description :
   - il faut calculer l'adresse du bloc physique qui contient
     le bloc logique de numéro "block_number"
   - pour ce faire, il faut partir de first (adresse physique
     du premier bloc disponible dans l'INODE) et parcourir
     le chainage de la FAT block_number fois.
   - une fois l'adresse trouvée, il faut lire le bloc
     dans le buffer
 ************************************************************/

void sgf_read_block(OFILE *file, int block_number){

  //file->buffer[block_number] =  file->inode.first+block_number;

  sgf_read_block_impl(file, block_number);
}

/************************************************************
 Ajouter le bloc contenu dans le tampon au fichier
 ouvert décrit par "file".

 Description :
 - allouer un bloc libre (fonction alloc_block)
 - écrire le buffer dans ce bloc
 - écrire la valeur EOF dans la FAT pour ce bloc
 - si ce bloc est le premier (test sur first/last),
     - mettre à jour first et last de l'INODE
     - écrire l'INODE sur disque
 - sinon
     - il y a donc déjà un dernier bloc (devenu l'avant dernier)
     - mettre à jour la FAT pour que l'avant dernier bloc
       pointe sur le dernier
     - mettre à jour last de l'INODE
     - écrire l'INODE sur disque
 ************************************************************/

void sgf_append_block(OFILE *file)
{
  sgf_append_block_impl(file);
}

/************************************************************
 Ecrire le caractère "c" dans le fichier ouvert
 décrit par "file".

 Description :
 - il y a toujours de la place dans le buffer (postulat)
 - placer le caractère dans le buffer (en fonction de ptr)
 - si le buffer est plein (test sur ptr % BLOCK_SIZE),
   ajouter le bloc au fichier (sgf_append_block)
 ************************************************************/

void sgf_putc(OFILE *file, char c)
{
  sgf_putc_impl(file, c);
}

/************************************************************
 Écrire la chaîne de caractère "s" dans un fichier ouvert
 en écriture décrit par "file".
 ************************************************************/

void sgf_puts(OFILE *file, char *s)
{
  assert(file->mode == WRITE_MODE);

  for (; (*s != '\0'); s++)
  {
    sgf_putc(file, *s);
  }
}

/************************************************************
 Détruire un fichier.

 Description :
 - lire l'INODE et le libérer (FAT_FREE dans la FAT)
 - parcours le chaînage et libérer tous les blocs
 ************************************************************/

void sgf_remove(int adr_inode)
{
  sgf_remove_impl(adr_inode);
}

/************************************************************
 Créer un inode vide, initialiser l'INODE passé en paramètre
 et renvoyer l'adresse. (renvoie -1 en cas d'erreur)

 Description :
 - allouer un bloc libre (fonction alloc_block)
 - initialiser les champs de l'INODE (0 et FAT_EOF)
 - sauver l'INODE et mettre à jour la FAT
 ************************************************************/

static int create_inode(INODE *inode)
{
  return create_inode_impl(inode);
}

/************************************************************
 Ouvrir un fichier en écriture seulement (NULL si échec).
 ************************************************************/

OFILE *sgf_open_write(const char *nom)
{
  int adr_inode, oldinode;
  INODE inode;
  OFILE *file;

  /* Rechercher un bloc libre sur disque */
  adr_inode = create_inode(&inode);
  if (adr_inode < 0)
    return NULL;

  /* Allouer une structure OFILE */
  file = malloc(sizeof(OFILE));
  if (file == NULL)
    return (NULL);

  /* mettre à jour le répertoire */
  oldinode = add_inode(nom, adr_inode);
  if (oldinode > 0)
    sgf_remove(oldinode);

  file->inode = inode;
  file->adr_inode = adr_inode;
  file->mode = WRITE_MODE;
  file->ptr = 0;

  return (file);
}

/************************************************************
 Ouvrir un fichier en lecture seulement (NULL si échec).
 ************************************************************/

OFILE *sgf_open_read(const char *nom)
{
  int adr_inode;
  INODE inode;
  OFILE *file;

  /* Chercher le fichier dans le répertoire */
  adr_inode = find_inode(nom);
  if (adr_inode < 0)
    return (NULL);

  /* lire l'INODE */
  inode = read_inode(adr_inode);

  /* Allouer une structure OFILE */
  file = malloc(sizeof(OFILE));
  if (file == NULL)
    return (NULL);

  file->inode = inode;
  file->adr_inode = adr_inode;
  file->mode = READ_MODE;
  file->ptr = 0;

  return (file);
}

/************************************************************
 Fermer un fichier ouvert.

 Description :
 - si le fichier est ouvert en écriture et que le buffer
   n'est pas vide (ptr % BLOCK_SIZE), ajouter le dernier bloc
   au fichier (fonction sgf_append_block)
 ************************************************************/

void sgf_close(OFILE *file)
{
  sgf_close_impl(file);
}
