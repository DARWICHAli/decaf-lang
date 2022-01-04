/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Quadruplets, structures et fonctions associées
 * @ingroup ASM
 */
#ifndef INCLUDE__GENASM__H
#define INCLUDE__GENASM__H

#include <stddef.h> // size_t
#include <stdio.h> // FILE (obligatoire, pas possible de FD)

/**
 * @addtogroup ASM
 * @{
 *
 */

/**
 * @brief Liste des langages vers lesquels la conversion est possible. (Statique)
 */
enum ASM_LANG {
	ASM_MIPS ///< Assembleur MIPS : "MIPS"
};


// forward declarations
struct quad;

/**
 * @brief traduit du code intermédiaire vers du code assembleur dans un fichier
 * @param to_lang Langage assembleur vers lequel traduire
 * @param qlist Liste linéaire de quadruplets
 * @param liste_size Taille de la liste
 * @param[out] outfile descripteur de fichier en sortie
 *
 * @note Pour voir la liste des langages toujours disponible voir ::ASM_LANG
 */

void genasm(const char* to_lang, const struct quad* qlist, size_t liste_size, FILE* outfile);

///@}

#endif
