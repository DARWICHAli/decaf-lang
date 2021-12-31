/**
 * @file
 * @author Léo Chéneau
 * @brief Suite de test
 *
 * Fichiers contenant les headers d'une suite de test
 */
#ifndef INCLUDE__TEST__SUITE_H
#define INCLUDE__TEST__SUITE_H

#include "stdio.h"

/**
 * @defgroup Tests Tests
 * @brief Module de tests
 *
 * @addtogroup Tests
 * @{
 */

/// @brief nombre de tests max
#define MAX_TESTS 64

/// @brief Taille max du nom d'un test
#define MAX_NAME 64

/// @brief couleur rouge
#define RED "\033[0;31m"

/// @brief couleur verte
#define GRE "\033[0;32m"

/// @brief couleur par défaut
#define DEF "\033[0;0m"

#include <stddef.h> //size_t

/**
 * @macro ASSERT_TRUE
 * @brief Fait return 0 si condition fausse et affiche la condition
 * @param exp Expression booléenne
 * @warning Fait un "return 0" si exp faux !!
 */
#define ASSERT_TRUE(exp) do {if (!(exp)) {fprintf(stderr, "Test failed : \"" #exp "\"\n"); return 0;}}while(0)

/**
 * @macro ASSERT_EQ
 * @brief Fait return 0 si lhs != rhs et affiche les opérandes
 * @param lhs Membre de gauche
 * @param rhs Membre de droite
 * @warning Fait un "return 0" si lhs != rhs !!
 */
#define ASSERT_EQ(lhs, rhs) do {if ((rhs) != (lhs)) {fprintf(stderr, "Test failed : \"" #lhs "\" != \"" #rhs "\"\n"); return 0;}}while(0)

/**
 * @macro ASSERT_EQSTR
 * @brief Fait return 0 si deux cstring sont différentes et les affiche
 * @param lhs c-string de gauche
 * @param rhs c-string de droite
 * @warning Fait un "return 0" si strcmp(lhs, rhs) != 0 !!
 */
#define ASSERT_EQSTR(lhs, rhs) do {if (strcmp((rhs), (lhs)) != 0) {fprintf(stderr, "Test failed : \"%s\" != \"%s\"\n", lhs, rhs); return 0;}}while(0)


/**
 * @brief Structure représentant une suite de test
 */
struct test_suite {
	char nom[MAX_NAME]; ///< Nom des tests
	int (*tests[MAX_TESTS])(void*); ///< Tests
	int (*test_call[MAX_TESTS])(int (*fn)(void*), void*); ///< Test caller
	int results[MAX_TESTS]; ///< résultats des tests
	char nom_test[MAX_TESTS][MAX_NAME]; ///< Noms des tests
	int (*setup)(void**); ///< Fonction à exécuter avant chaque test
	int (*teardown)(void**); ///< Fonction à exécuter après chaque test
	void* data; ///< Données utilisées
	size_t nb_tests; ///< Nombre de tests
};

/**
 * @brief Crée une suite de test
 *
 * @param name Nom de la suite de test
 * @param setup Fonction à exécuter avant chaque test
 * @param teardown Fonction à exécuter après chaque test
 *
 * @return Une suite de test vide
 */
struct test_suite make_ts(const char* name, int (*setup)(void**), int (*teardown)(void**));

/**
 * @brief Ajoute un test à la suite
 *
 * @param ts Suite de test
 * @param test Fonction de test
 * @param nom Nom du test
 */
void add_test(struct test_suite* ts, int (*test)(void*), const char* nom);


/**
 * @brief Ajoute un test qui devrait déclencher une assertion
 *
 * @param ts Suite de test
 * @param test Fonction devant déclencher assert
 * @param nom Nom du test
 */
void add_test_assert(struct test_suite* ts, int (*test)(void*), const char* nom);


/**
 * @brief Exécute une suite de test
 *
 * @param ts Suite à exécuter
 *
 * @return 1 si Tout s'est bien passé, 0 sinon.
 */
int exec_ts(struct test_suite* ts);

/**
 * @}
 */

#endif
