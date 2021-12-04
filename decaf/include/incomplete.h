/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Gestion de listes de quadruplets incomplets
 * @ingroup IR
 */
#ifndef INCLUDE__INCOMPLETE__H
#define INCLUDE__INCOMPLETE__H

#include "quadops.h"
#include "gencode.h"

/**
 * @addtogroup IR
 * @{
 *
 * @defgroup quadlist Liste de quadruplets incomplets
 * @brief Structure et fonctions facilitant la maintenance de listes de quadruplets incomplets

 * @addtogroup quadlist
 * @{
 */

///< Taille maximale d'une liste
#define QUADLIST_MAX_SIZE 100

/**
 * @brief Liste de quadruplets
 * @warning à ne pas utiliser directement !
 *
 * crée à partir de ::qlist_new
 */
struct quad_list {
	struct quad quads[QUADLIST_MAX_SIZE]; ///< Liste réelle
	unsigned long long used; ///<  Nombre de places utilisées
};

/**
 * @brief Crée une liste de quadruplets vide
 */
struct quad_list qlist_new();

/**
 * @brief Ajoute un quadruplet à la fin de la liste
 * @param[inout] qlst Liste à laquelle ajouter le quadruplet
 * @param qid Identifiant du quadruplet à ajouter
 * @return pointeur sur la liste passée en paramètre.
 * @warning plante si impossible d'ajouter un quadruplet
 */
struct quad_list* qlist_append(struct quad_list* qlst, quad_id_t qid);

/**
 * @brief complète tous les quadruplets incomplets par l'identifiant du quadruplet passé en paramètre
 * @param[in] qlst Liste à patcher
 * @param qid identifiant du quadruplet vers lequel compléter
 *
 * @warning Si la liste contient des quadruplets non patchables, le programme plantera.
 */
void qlist_complete(struct quad_list* qlst, quad_id_t qid);

/**
 * @}
 * @}
 */
#endif
