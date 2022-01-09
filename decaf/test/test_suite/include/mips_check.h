/**
 * @file
 * @author Léo Chéneau
 * @brief Fonction pour tester des fichiers MIPS
 *
 * Fonctions utiles pour les tests de hau-niveau
 */
#ifndef INCLUDE__MIPS_CHECK_H
#define INCLUDE__MIPS_CHECK_H

#include "stdio.h"

/**
 * @addtogroup Tests
 * @{
 */

/**
 * @defgroup MIPS MIPS
 * @brief Tests de fichiers MIPS
 *
 * Fonctions utiles pour vérifier la conformité de fichiers MIPS
 *
 * @addtogroup MIPS
 * @{
 */


/**
 * @brief Vérifie qu'un fichier MIPS est conforme aux attentes
 *
 * Les commentaires sont ignorés dans le fichier testé
 *
 * @param f Fichier MIPS à tester
 * @param exp Contenu du fichier attendu
 *
 * @return 1 si le fichier est conforme, 0 sinon
 */
int check_file(FILE* f, const char* exp);

/**
 * @brief Affiche un fichier
 *
 * @param f Fichier à affivher
 *
 * @return 1 si tout est ok, 0 sinon
 */
int print_file(FILE* f);

/**
 * @}
 * @}
 */

#endif

