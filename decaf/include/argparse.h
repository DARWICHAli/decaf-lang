/**
 * @file
 * @brief Parsing des arguments de la ligne de commande
 */

#ifndef INCLUDE__ARGPARSER__H
#define INCLUDE__ARGPARSER__H

/**
 * @brief Paramètres extraits de la ligne de commande
 */
struct params {
	int print_table; ///< Demande d'affichage de la table des symboles
	int debug_mode; ///< Mode debug demandé
	const char* output_file; ///< Fichier de sortie
};

/**
 * @brief Argument table des symboles
 *
 * @param p paramètres
 * @param args args
 *
 * @return Vrai si tout s'est bien passé, 0 sinon
 */
int arg_tos(struct params* p, char* const args[]);

/**
 * @brief Argument version
 *
 * Affiche les noms et la version du programme
 *
 * @note quitte le programme.
 *
 * @param p paramètres
 * @param args args
 *
 * @return Vrai si tout s'est bien passé, 0 sinon
 */
int arg_version(struct params* p, char* const args[]);

/**
 * @brief Argument d'aide
 *
 * Affiche l'aide du programme et quitte
 *
 * @note quitte le programme.
 *
 * @param p paramètres
 * @param args args
 *
 * @return Vrai si tout s'est bien passé, 0 sinon
 */
int arg_help(struct params* p, char* const args[]);

/**
 * @brief Argument fichier de sortie
 *
 * @param p paramètres
 * @param args args
 *
 *
 * @return Vrai si tout s'est bien passé, 0 sinon
 */
int arg_out(struct params* p, char* const args[]);

/**
 * @brief Argument mode debug
 *
 * @param p paramètres
 * @param args args
 *
 * @return Vrai si tout s'est bien passé, 0 sinon
 */

int arg_debug(struct params* p, char* const args[]);

/**
 * @brief Parse les arguments de la ligne de commande
 *
 * @param argc Nombre d'arguments
 * @param argv[] Liste des arguments
 *
 * @note Cette fonction peut faire quitter le programme.
 *
 * @return Les paramètres déduits des arguments
 */
struct params parse_args(int argc, char* const argv[]);

/**
 * @brief Arguments par défaut du programme
 *
 * @return Les arguments par défaut
 */
struct params default_args();

#endif
