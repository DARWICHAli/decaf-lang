/**
 * @file opticode.h
 * @author 
 * @brief Fonctions d’optimisation sur le code intermédiaire.
 * @version 0.1
 *
 * @ingroup Optimisation
 */

#ifndef INCLUDE__OPTICODE__H
#define INCLUDE__OPTICODE__H

#include <stddef.h> // size_t

/**
 * @addtogroup Optimisation
 * @{
 *
 * @defgroup opticode opticode
 * @brief Optimisation de code intermediaire

 * @addtogroup opticode
 * @{
 */

/**
 * @brief Structure représentant les optimisations et paramètres d’optimisations possibles
 */
struct optim_param {
    int max_passes; ///< Nombre maximum de passes, 0=infini
};

// forward declarations
struct quad;
struct optim_params;

/**
 * @brief Fonction qui optimise le code intermédiaire d’après les optimisations demandées dans params
 * 
 * @param[in] liste_in le tableau des quads d'entré. 
 * @param[in] liste_size_in la taille du tableau d'entré. 
 * @param[out] liste_out le tableau des quads de sortie.
 * @param[out] liste_size_out la taille du tableau de sortie.
 * @param[in] params structure contenant des options d'optimisation.
 * 
 * @return Production d'un nouveau code intermediaire optimisé.
 */
void opticode(const struct quad* liste_in, size_t liste_size_in, struct quad* liste_out, size_t* liste_size_out, const struct optim_params* params);

/**
 * @}
 * @}
 */

#endif
