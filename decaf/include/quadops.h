/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Quadruplets, structures et fonctions associées
 * @ingroup IR
 */
#ifndef INCLUDE__QUADOPS__H
#define INCLUDE__QUADOPS__H

#include <stddef.h> // size_t

/**
 * @addtogroup IR
 * @{
 *
 * @defgroup Quadruplets Quadruplets
 * @brief Quadruplets

 * @addtogroup Quadruplets
 * @{
 */

/// @brief Type de l'dentifiant unique associé à chaque quadruplet
typedef size_t quad_id_t;

/**
 * @enum CMP_OP
 * @brief Opérateurs de comparaisons dans le if-goto
 */
enum CMP_OP {
	CMP_LT, ///< Inférieur strict
	CMP_GT, ///< Supérieur strict
	CMP_EQ, ///< Égalité
	CMP_LE, ///< Inférieur ou égal
	CMP_GE, ///< Supérieur ou égal
};

/**
 * @enum Q_OP
 * @brief Opérateurs autorisés pour les quadruplets
 * Les opérateurs peuvent être à trois opérandes, ou 2.
 */
enum Q_OP {
	Q_ADD, ///< Addition, 3 opérandes
	Q_SUB, ///< Soustraction, 3 opérandes
	Q_MUL, ///< Multiplication, 3 opérandes
	Q_DIV, ///< Division, 3 opérandes
	Q_MOD, ///< Modulo, 3 opérandes
	Q_NEG, ///< Négation, 2 opérandes
	Q_IFG, ///< If E then goto Y, 2 opérandes
	Q_GOT, ///< Goto simple 1 opérande
	Q_AFF, ///< Affectation, 2 opérandes
	Q_CST, ///< Affectation de constante
	Q_PAR, ///< Argument de fonction
	Q_PRO, ///< Appel de procédure
	Q_CAL,  ///< Appel de fonction
	Q_RET, ///< Retour de fonction
	Q_END ///< Fin de procédure
};

// forward declarations
struct entry;
struct context;

/**
 * @struct quad
 * @brief Type représentant un quadruplet
 *
 * @warning Pour les opérateurs à 2 opérandes, res = NULL.
 */
struct quad {
	const struct context* ctx; ///< Contexte dans lequel ce quadruplet a été produit
	enum Q_OP op; ///< Opérateur
	const struct entry* lhs; ///< Opérande gauche
	const struct entry* rhs; ///< Opérande droite
	const struct entry* res; ///< Variable dans laquelle affecter
	quad_id_t dst; ///< Destination d'un éventuel goto
	enum CMP_OP cmp; ///< Opérateur de comparaison du if-goto
	int val; ///< Valeur pour l'affectation constante
};

/**
 * @defgroup helpers helpers
 * @brief Aide à la création de quadruplets
 *
 * @addtogroup helpers
 * @{
 */

/**
 * @brief Crée un quadruplet arithmétique
 *
 * @param res variable résultat
 * @param lhs Opérande de gauche
 * @param qop Opérateur arithmétique
 * @param rhs Opérande de droite
 *
 * @pre qop doit être arithmétique
 * @pre res, lhs, rhs doivent être des variable
 * @pre res ne doit pas être globale
 *
 * @return Un quadruplet arithmétique du type "res := lhs QOP rhs"
 */
struct quad quad_arith(const struct entry* res, const struct entry* lhs, enum Q_OP qop, const struct entry* rhs);

/**
 * @brief Crée un quadruplet d'affectation
 *
 * @param res Variable résultat
 * @param val Variable affectée
 *
 * @pre res, val sont des variables
 * @pre res n'est pas globale
 *
 * @return Un quadruplet de la forme "res := val"
 */
struct quad quad_aff(const struct entry* res, const struct entry* val);

/**
 * @brief Crée un quadruplet de négation
 *
 * @param res Variable résultat
 * @param val Variable affectée
 *
 * @pre res, val sont des variables
 * @pre res n'est pas globale
 *
 * @return Un quadruplet de la forme "res := -val"
 */
struct quad quad_neg(const struct entry* res, const struct entry* val);

/**
 * @brief Crée un quadruplet goto
 *
 * @param qid Destination du goto
 *
 * @return Un quadruplet du type "goto qid"
 */
struct quad quad_goto(quad_id_t qid);

/**
 * @brief Crée un quadruplet if-goto
 *
 * @param lhs Opérande gauche de la comparaison
 * @param cmp_op Opérateur de comparaison
 * @param rhs Opérande droite de la comparaison
 * @param qid Destination du goto
 *
 * @pre cmp_op est un opérateur de comparaison
 *
 * @return Un quadruplet de la forme "if lhs COP rhs then goto qid"
 */
struct quad quad_ifgoto(const struct entry* lhs, enum CMP_OP cmp_op, const struct entry* rhs, quad_id_t qid);

/**
 * @brief Crée un quadruplet param
 *
 * @param var Paramètre de fonction
 *
 * @return Un quadruplet de la forme "param var"
 */
struct quad quad_param(const struct entry* var);

/**
 * @brief Crée un quadruplet d'appel de fonction
 *
 * @param res Variable dans laquelle stocker le résultat
 * @param fct Fonction à appeller
 *
 * @pre res n'est pas globale
 * @pre fct est une fonction non-void
 *
 * @return Un quadruplet de la forme "res := call fct"
 */
struct quad quad_call(const struct entry* res, const struct entry* fct);

/**
 * @brief Crée un quadruplet d'appel de procédure
 *
 * @param proc Procédure à appeller
 *
 * @warning si proc est une fonction non-void
 *
 * @return Un quadruplet de la forme "call proc"
 */
struct quad quad_proc(const struct entry* proc);

/**
 * @brief Crée un quadruplet d'affectation constante
 *
 * @param var Variable de destination
 * @param cst Valeur à affecter
 *
 * @return Un quadruplet de la forme "var := cst"
 */
struct quad quad_cst(const struct entry* var, int cst);

/**
 * @brief Crée un quadruplet de retour de fonction
 *
 * @param ret Retour de la fonction
 *
 * @return Un quadruplet de la forme "return ret"
 */
struct quad quad_return(const struct entry* ret);

/**
 * @brief Crée un quadruplet de fin de procédure
 *
 * @return Un quadruplet de la forme "return;"
 */
struct quad quad_endproc();

///@}
///@}
///@}

#endif
