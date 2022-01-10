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

#include <stddef.h> // size_t

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

/// @brief valeur de destination des quadruplets incomplets
#define INCOMPLETE_QUAD_ID ((quad_id_t)-1)

/**
 * @brief Liste de quadruplets
 * @warning à ne pas utiliser directement !
 *
 * crée à partir de ::qlist_new
 */
struct quad_list {
	quad_id_t quads[QUADLIST_MAX_SIZE]; ///< Liste réelle
	size_t used; ///<  Nombre de places utilisées
};

/**
 * @brief Crée une liste de quadruplets vide
 */
struct quad_list* qlist_empty();

struct quad_list* qlist_new(quad_id_t qid);


struct quad_list* qlist_concatn(size_t n, struct quad_list* ql, ...);

/**
 * @brief Ajoute un quadruplet à la fin de la liste
 * @param[inout] qlst Liste à laquelle ajouter le quadruplet
 * @param qid Identifiant du quadruplet à ajouter
 * @return pointeur sur la liste passée en paramètre.
 * @warning plante si impossible d'ajouter un quadruplet
 */
void qlist_append(struct quad_list* qlst, quad_id_t qid);

/**
 * @brief Concatène deux quad_list et renvoie la nouvelle liste
 * @param[in] ql1 Première liste à ajouter à la liste finale
 * @param[in] ql2 Deuxième liste à ajouter à la liste finale
 * @return Une nouvelle liste de quadruplet contenant les quadruplet de ql1 et ql2
 * @warning plante si impossible d'ajouter un quadruplet et si les tailles finales ne correspondent pas
 */
struct quad_list* qlist_concat(const struct quad_list* lhs, const struct quad_list* rhs);


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
