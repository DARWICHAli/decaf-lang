/**
 * @file
 * @author Léo Chéneau <leo.cheneau@etu.unistra.fr
 * @version 1.0
 * @brief Génération du code assembleur
 * @ingroup ASM
 */
#ifndef INCLUDE__GENASM__H
#define INCLUDE__GENASM__H

#include <stddef.h> // size_t
#include <stdio.h> // FILE (obligatoire, pas possible de FD)

#include "quadops.h"

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

/**
 * @brief Paramètres de génération de l'assembleur
 */
struct asm_params {
	int generate_entrypoint; ///< Est-ce qu'on doit générer un point d'entrée ?
};


// forward declarations
struct quad;
struct context;

/**
 * @brief traduit du code intermédiaire vers du code assembleur dans un fichier
 * @param to_lang Langage assembleur vers lequel traduire
 * @param qlist Liste linéaire de quadruplets
 * @param liste_size Taille de la liste
 * @param[out] outfile descripteur de fichier en sortie
 * @param genp Paramètres de génération
 *
 * @pre outfile ouvert
 * @pre genp != NULL
 * @pre qlist != NULL
 * @pre liste_size > 0
 *
 * @note Pour voir la liste des langages toujours disponible voir ::ASM_LANG
 */

void genasm(const char* to_lang, const struct quad* qlist, size_t liste_size, FILE* outfile, const struct asm_params* genp);

/**
 * @defgroup ASM_helpers ASM_helpers
 * @brief Aide à la génération ASM
 * @addtogroup ASM_helpers
 * @{
 */

int contains_func(const struct context* ctx);

int is_in(enum Q_OP op, enum Q_OP* tab, size_t size);

int check_quad(struct quad q);

const struct context* ctx_argsfun(const struct context* ctx);

const struct context* ctx_root_ctx();

int is_in(enum Q_OP op, enum Q_OP* tab, size_t size);

///@}
///@}

#endif
