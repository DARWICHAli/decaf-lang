/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Génération de code et quadruplets
 * @ingroup IR
 */
#ifndef INCLUDE__QUADOPS__H
#define INCLUDE__QUADOPS__H

#include "quadops.h"

/**
 * @addtogroup IR
 * @{
 *
 * @defgroup gencode Code intermédiaire
 * @brief Génération du code intermédiaire

 * @addtogroup gencode
 * @{
 */

/// @brief Type de l'dentifiant unique associé à chaque quadruplet
typedef unsigned long quad_id_t;

/**
 * @brief récupère un tableau linéaire de tous les quadruplets pour la conversion vers assembleur
 * @param[in] size Taille du tableau renvoyé.
 * @return Un tableau de quadruplet
 *
 * Permet de préserver l'opacité de l'implémentation
 */
struct quad* get_all_quads(unsigned long long* size);

/**
 * @brief Récupère l'identifiant du prochain quadruplet qui SERA généré.
 * @return Un identifiant qui restera toujours valide et pointant toujours sur le même quadruplet.
 */
quad_id_t nextquad();

/**
 * @brief Récupère un quadruplet à partir de son identifiant
 * @param id Identifiant unique de quadruplet
 * @return un pointeur sur le quadruplet voulu.
 * @warning Si l'identifiant est valide, crashe le programme en debug et renvoie NULL.
 */
struct quad* getquad(quad_id_t id);

/**
 * @brief Ajoute un nouveau quadruplet APRÈS tous les autres
 * @param quadop quadruplet à ajouter
 * @return Identifiant du quadruplet généré
 * @note Il est conseillé d'utiliser la notation simplifiée pour générer le quadruplet et alléger le code.
 */
quad_id_t gencode(struct quad quadop);

///@}
///@}
#endif
