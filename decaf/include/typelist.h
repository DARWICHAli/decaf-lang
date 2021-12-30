/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Listes de types
 * @ingroup CTX
 */
#ifndef INCLUDE__TYPELIST__H
#define INCLUDE__TYPELIST__H

#include "typedesc.h"

/**
 * @addtogroup CTX
 * @{
 *
 * @defgroup Typelist Typelist
 * @brief Listes de types

 * @addtogroup Typelist
 * @{
 */

/**
 * @brief Nombre maximal d'éléments dans une :typelist
 */
#define MAX_TYPELIST_SIZE 64

/**
 * @struct typelist
 * @brief Structure représentant une liste de types
 * @warning à ne pas utiliser directement !!
 *
 * Voir :typelist_new et typelist_append
 */
struct typelist {
	enum BTYPE btypes[MAX_TYPELIST_SIZE]; ///< éléments de la liste
	size_t used; ///< Nomre de places utilisées
};

/**
 * @brief crée une nouvelle liste de types vide
 * @return Une typelist vide
 */
struct typelist typelist_new();

/**
 * @brief Ajoute un type primitif à la FIN de la liste
 * @param[inout] lst Liste en entrée, modifiée in-place
 * @param[in] new_type Nouveau type à ajouter à la fin de la liste
 * @return La liste avec un élément de plus
 * @note \@ret == \@lst
 * @warning Modifications en place
 * @warning Si échec, plantage
 */
struct typelist* typelist_append(struct typelist* lst, enum BTYPE new_type);

/**
 * @brief Teste si deux listes de types sont égales
 *
 * @param lhs Liste de types
 * @param rhs Liste de types
 *
 * @return Vrai si les listes sont égales, faux sinon.
 */
int typelist_equals(const struct typelist* lhs, const struct typelist* rhs);


/**
 * @}
 * @}
 */

#endif
