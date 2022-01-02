/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Entrées dans la table des symboles et fonctions associées
 * @ingroup CTX
 */
#ifndef INCLUDE__ENTRY__H
#define INCLUDE__ENTRY__H

#include "typedesc.h"

/**
 * @addtogroup CTX
 * @{
 *
 * @defgroup Entry Entry
 * @brief Entrées dans la table des symboles

 * @addtogroup Entry
 * @{
 */


/**
 * @brief Taille maximale d'un identifiant
 */
#define MAX_IDENTIFIER_SIZE 128

// forward declaration
struct context;

/**
 * @struct entry
 * @brief Entrée dans la table des symboles
 * @warning id est en lecture seule !!
 *
 * Représente une entrée dans la table des symboles créée via :ctx_newname ou :ctx_make_temp
 */
struct entry {
	char id[MAX_IDENTIFIER_SIZE]; ///< Identifiant de l'entrée
	struct typedesc type; ///< Type de l'entrée
	const struct context* ctx; ///< Contexte dans lequel apparaît l'entrée
};


/**
 * @}
 * @}
 */
#endif
