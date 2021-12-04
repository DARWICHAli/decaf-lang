/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Quadruplets, structures et fonctions associées
 * @ingroup IR
 */
#ifndef INCLUDE__QUADOPS__H
#define INCLUDE__QUADOPS__H

/**
 * @addtogroup IR
 * @{
 *
 * @defgroup Quadruplets Quadruplets
 * @brief Quadruplets

 * @addtogroup Quadruplets
 * @{
 */

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

	Q_IFG, ///< If E then goto Y, 2 opérandes
	Q_AFF  ///< Affectation, 2 opérandes
};

// forward declarations
struct entry;

/**
 * @struct quad
 * @brief Type représentant un quadruplet
 *
 * @warning Pour les opérateurs à 2 opérandes, res = NULL.
 */
struct quad {
	struct entry* lhs; ///< Opérande gauche
	struct entry* rhs; ///< Opérande droite
	enum Q_OP op; ///< Opérateur
	struct entry* res; ///< variable dans laquelle affecter
};


///@}
///@}

#endif
