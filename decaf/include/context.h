/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Tables des symboles
 * @ingroup CTX
 */
#ifndef INCLUDE__CTX_CONTEXT__H
#define INCLUDE__CTX_CONTEXT__H

#include "entry.h"

/**
 * @addtogroup CTX
 * @{
 *
 * @defgroup Context Context
 * @brief Tables des symboles

 * @addtogroup Context
 * @{
 */

/**
 * @brief Nombre d'entrées par contexte sans extensions
 */
#define DEF_ENTRIES_PER_CTX 64

/**
 * @struct context
 * @brief Structure représentant une table des symboles
 * @warning à ne pas manipuler directement
 *
 * Cette structure contient les entrées dans la table
 */
struct context {
	struct entry entries[DEF_ENTRIES_PER_CTX]; ///< Entrées internes
	size_t used; ///< Nombre d'entrées utilisées en interne
	const struct context* parent; ///< Context parent, NULL si racine
	struct context* next; ///< Context de même niveau stockant plus d'entrées
};


/**
 * @brief Empile un nouveau contexte vierge
 *
 * @return Le :context empilé
 */
struct context* ctx_pushctx();

/**
 * @brief Dépile le dernier contexte empilé
 *
 * @return Le nouveau sommet de pile
 */
struct context* ctx_popctx();

/**
 * @brief Récupère le contexte courant, sommet de pile
 *
 * @return Le sommet de la pile des contextes
 */
struct context* ctx_currentctx();

/**
 * @brief Ajoute une entrée dans le contexte courant
 *
 * @param[in] id Identifiant de l'entrée
 *
 * @return L'entrée ajoutée au contexte
 *
 * @warning Ne pas oublier de renseigner les autres informations sur l'entrée plus tard !
 */
struct entry* ctx_newname(const char id[MAX_IDENTIFIER_SIZE]);

/**
 * @brief Crée un entrée dans le contexte courant
 *
 * @return Une entrée valide et unique
 *
 * @note L'entrée n'est PAS supprimée automatiquement
 *
 * Les identifiants sont générés automatiquements et sont uniques.
 */
struct entry* ctx_make_temp();

/**
 * @brief Recherche un identifiant depuis la table des symoles courante
 *
 * @param id Identifiant à rechercher
 *
 * @return NULL si l'entrée n'existe pas, l'entrée sinon.
 *
 * @warning Ne pas oublier de tester la valeur de retour de cette fonction
 */
struct entry* ctx_lookup(const char id[MAX_IDENTIFIER_SIZE]);

/**
 * @brief Compte le nombre d'entrées dans le contexte demandé
 *
 * @param ctx Contexte à examiner
 *
 * @return Le nombre d'entrée
 *
 * @warning N'examine pas les fils
 */
size_t ctx_count_entries(const struct context* ctx);

/**
 * @brief Compte le nombre d'octets à allouer pour le contexte demandé
 *
 * @param ctx Contexte à examiner
 *
 * @return Le nombbre d'octets à allouer
 *
 * @warning N'examine pas les fils
 */
size_t ctx_count_bytes(const struct context* ctx);

/**
 * @brief Récupère la n-ième entrée du contexte
 *
 * @param ctx Contexte dans lequel rechercher
 * @param n Numéro de l'entrée
 *
 * @pre n < Nombre d'éléments
 * @warning Premier élément -> n=0
 *
 * @return La n-ième entrée
 */
const struct entry* ctx_nth(const struct context* ctx, size_t n);

/**
 * @brief Récupère la n-ième entrée du contexte qui soit une fonction
 *
 * @param ctx Contexte dans lequel rechercher
 * @param n Numéro de l'entrée
 *
 * @warning si n < Nombre de fonctions, renvoie NULL
 * @warning Premier élément -> n=0
 *
 * @return La n-ième entrée qui soit une fonction, sinon NULL
 */
const struct entry* ctx_nth_function(const struct context* ctx, size_t n);

/**
 * @brief Index octal de l'entrée dans sa table
 *
 * @param ent Entrée dans une table
 *
 * @return L'index octal de l'entrée
 *
 * Imaginons la tabble suivante :
 * a = 4o
 * b = 16o
 * c = 8o
 * d = 64o
 * e = 4o
 * Alors :
 * l'index octal de `d` est : `4+16+8`. **sans la taille de d**.
 * l'index octal de `e` est : `4+16+8+64`.
 */
size_t ctx_byte_idx(const struct entry* ent);

/**
 * @brief Plus long chemin partant du contexte passé en paramètre
 *
 * @param ctx Contexte parent
 *
 * @note La longueur d'un chemin est donné par la taille octale d'une table
 * @return Le plus long chemin en nombre d'octets
 *
 * Pour plus d'informations sur le calcul, voir la spécification technique
 */
size_t ctx_longest_path(const struct context* ctx);

/**
 * @brief Cherche un identifiant dans un contexte seulement
 *
 * Ne vas pas voir les parents
 *
 * @param str Identifiant
 * @param ctx Contexte
 *
 * @return NULL si introuvable, l'entrée sinon
 */
struct entry* ctx_search(const char* str, const struct context* ctx);

/**
 * @brief Cpnverti une chaîne en identifiant
 *
 * @param str Chaîne C à convertir
 *
 * @return Une chaîne pour utilisation temporaire.
 */
const char* tokenize(const char* str);

/**
 * @brief Affiche la table ctx et tout ses fils. 
 * 
 */
void ctx_fprintf(FILE* fd, const struct context* ctx);

/**
 * @}
 * @}
 */
#endif
