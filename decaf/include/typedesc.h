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
#include <stdio.h>
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
 * @param[in] btype type dont on veut récupérer la taille
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
	enum MTYPE mtype; ///< Méta-type de l'entrée
	enum BTYPE btype; ///< Type primaire de l'entrée

	union {
			const struct typelist* arg_list; ///< Liste optionnelle d'arguments
			size_t size; ///< Taille optionnele d'un tableau
	} dist; ///< Union qui devrait être anonyme
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
 * @brief Méta-type de l'entrée
 *
 * @param td Descripteur de type
 *
 * @return Le meta-type de l'entrée
 */
enum MTYPE typedesc_meta_type(const struct typedesc* td);

/**
 * @brief Teste si le descripteur correspond à une fonction
 *
 * @param td Descripteur de type
 *
 * @return Vrai si le descripteur est une fonction, Faux sinon
 */
int typedesc_is_function(const struct typedesc* td);

/**
 * @brief Teste si le descripteur correspond à une variable
 *
 * @param td Descripteur de type
 *
 * @return Vrai si le descripteur est une variable, Faux sinon
 */
int typedesc_is_var(const struct typedesc* td);

/**
 * @brief Teste si le descripteur correspond à un tableau
 *
 * @param td Descripteur de type
 *
 * @return Vrai si le descripteur est un tableau, Faux sinon
 */
int typedesc_is_tab(const struct typedesc* td);

/**
 * @brief Type de la variable
 *
 * @param td Descripteur de type
 *
 * @pre td est une variable
 *
 * @return Le type de la variable
 */
enum BTYPE typedesc_var_type(const struct typedesc* td);

/**
 * @brief Type de retour de la fonction
 *
 * @param td Descripteur de type
 *
 * @pre td est une fonction
 *
 * @return Le type de la fonction
 */
enum BTYPE typedesc_function_type(const struct typedesc* td);

/**
 * @brief Liste des paramètres de la fonction
 *
 * @param td Descripteur de type
 *
 * @pre td est une fonction
 *
 * @return Une liste de type correspondant aux paramètres
 */
const struct typelist* typedesc_function_args(const struct typedesc* td);

/**
 * @brief Type d'un élément du tableau
 *
 * @param td Descripteur de type
 *
 * @pre td est un tableau
 *
 * @return Le type d'un élément du tableau
 */
enum BTYPE typedesc_tab_type(const struct typedesc* td);


/**
 * @brief Nombre d'éléments du tableau
 *
 * @param td Descripteur de type
 *
 * @pre td est un tableau
 *
 * @return Le nombre d'éléments du tableau
 */
size_t typedesc_tab_size(const struct typedesc* td);

/**
 * @brief Fonction d'affichage pour struct typedesc
 * 
 * @param fd[out] fichier de sortie d'affichage
 * 
 * @param td[in] le descripteur de type
 * 
 */
void td_fprintf(FILE* fd, const struct typedesc* td);

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
