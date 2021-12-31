/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Types et gestion du typage
 * @ingroup CTX
 */
#ifndef INCLUDE__TYPEDESC__H
#define INCLUDE__TYPEDESC__H

#include <stddef.h> // size_t

/**
 * @addtogroup CTX
 * @{
 *
 * @defgroup Types Types
 * @brief Types et typage

 * @addtogroup Types
 * @{
 */

/**
 * @enum MTYPE
 * @brief méta-type d'un objet
 *
 * Méta-type associé à un objet
 */
enum MTYPE {
	MT_FUN, ///< Fonction
	MT_VAR, ///< Variable
	MT_TAB  ///< Tableau
};

/**
 * @enum BTYPE
 * @brief Types primaires
 *
 * Types primaires au sens types de base
 */
enum BTYPE {
	BT_BOOL, ///< Type booléen
	BT_INT   ///< Type entier signé
};


/**
 * @brief Taille réelle d'un type primaire
 *
 * @param[in] BTYPE type dont on veut récupérer la taille
 *
 * @return La taille en OCTETS du type primaire
 */
size_t bt_sizeof(const enum BTYPE btype);

// forward declaration
struct typelist;

/**
 * @struct typedesc
 * @brief Structure opaque représentant un type associé à un objet
 * @warning Ne pas utiliser les champs de cette structure directement !!!
 */
struct typedesc {
	enum MTYPE mtype;
	enum BTYPE btype;

	union {
			struct typelist* arg_list;
			size_t size;
	} dist;
};

/**
 * @brief Taille en octets d'un typê complexe
 * @param td Descripteur de type
 * @return La taille en octets associée au descripteur
 */
size_t td_sizeof(const struct typedesc* td);

/**
 * @brief Crée un descripteur de type correspondant à une fonction
 *
 * @param[in] ret_type Type de retour de la fonction
 * @param[in] arg_list Paramètres de la fonction
 *
 * @return Un descripteur de type bien formé
 */
struct typedesc typedesc_make_function(enum BTYPE ret_type, const struct typelist* arg_list);

/**
 * @brief Crée un descripteur de type correspondant à un tableau de types simples
 *
 * @param[in] elem_type Type d'un élément du tableau
 * @param[in] nb_elem Nombre de CASES
 * @note C'est un nomre de cases, pas d'octets
 *
 * @return Un descripteur de type bien formé
 */
struct typedesc typedesc_make_tab(enum BTYPE elem_type, size_t nb_elem);

/**
 * @brief Crée un descripteur de type correspondant à une simple variable
 *
 * @param[in] btype Type primitif de la variable
 *
 * @return Un descripteur de type bien formé
 */
struct typedesc typedesc_make_var(enum BTYPE btype);


/**
 * @brief Compare si deux descripteurs de types sont égaux
 *
 * @param lhs Descripteur de type
 * @param rhs Descripteur de type
 *
 * @return vrai si lhs <=> rhs, faux sinon
 */
int typedesc_equals(const struct typedesc* lhs, const struct typedesc* rhs);

/**
 * @brief Descripteur représentant une variable entière
 */
extern const struct typedesc td_var_int;

/**
 * @brief Descripteur représentant une variable booléene
 */
extern const struct typedesc td_var_bool;


/**
 * @}
 * @}
 */
#endif
