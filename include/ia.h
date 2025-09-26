/**
 * @file ia.h
 * @authors Groupe 8
 * @brief ia.h définit les structures de données et les prototypes pour l'Intelligence Artificielle.
 *
 * Il contient les types, constantes et fonctions publiques nécessaires
 * pour l'IA du jeu Krojanty.
 */

#ifndef IA_H
#define IA_H

#include "plateau.h"
#include <stdbool.h>
#include <stdint.h>

                /*  Types  */

/**
 * @struct Board
 * @brief Représentation interne et simplifiée du plateau de jeu pour l'IA.
 */
typedef struct {
    int pion[SIZE][SIZE];  /**< Matrice des pièces sur le plateau. */
} Board;

/**
 * @struct Move
 * @brief Représente un coup possible, avec des coordonnées de départ et d'arrivée.
 */
typedef struct {
    int r1, c1;  /**< Coordonnées de la case source (ligne, colonne). */
    int r2, c2;  /**< Coordonnées de la case de destination (ligne, colonne). */
} Move;

/**
 * @enum TTFlag
 * @brief Indicateurs pour les entrées de la table de transposition.
 */
typedef enum { TT_EMPTY=0, TT_EXACT=1, TT_LOWER=2, TT_UPPER=3 } TTFlag;

/**
 * @struct TTEntry
 * @brief Structure d'une entrée dans la table de transposition.
 */
typedef struct {
    uint64_t key;   /**< Clé de hachage Zobrist de la position. */
    int16_t  value; /**< Évaluation stockée pour cette position. */
    int8_t   depth; /**< Profondeur de recherche à laquelle la valeur a été calculée. */
    uint8_t  flag;  /**< Indicateur (EXACT, LOWER, UPPER) sur la nature de la valeur. */
    Move     best;  /**< Meilleur coup trouvé depuis cette position. */
} TTEntry;

        /*  Constantes IA  */

#define MAX_DEPTH        4     /**< Profondeur maximale de la recherche Minimax. */
#define MAX_MOVES        512    /**< Nombre maximal de coups possibles depuis une position. */
#define INF_SCORE        1000000/**< Valeur représentant l'infini pour les scores. */
#define TT_SIZE_POW2     17     /**< Taille de la table de transposition (2^17). */
#define TT_SIZE          (1u << TT_SIZE_POW2)
#define TT_MASK          (TT_SIZE - 1u)
#define EVAL_CLAMP(x)    ((x) > 30000 ? 30000 : ((x) < -30000 ? -30000 : (x))) /**< Macro pour borner les valeurs d'évaluation. */

            /*  Variables globales  */

extern TTEntry *TT;                   /**< Pointeur vers la table de transposition. */
extern uint64_t Zobrist[5][SIZE][SIZE];/**< Table des nombres aléatoires pour le hachage de Zobrist. */
extern uint64_t Z_SIDE;               /**< Clé Zobrist pour le changement de tour. */

extern Move g_last_best_move_blue;    /**< Stocke le dernier meilleur coup joué par l'IA bleue. */
extern Move g_last_best_move_red;     /**< Stocke le dernier meilleur coup joué par l'IA rouge. */

                    /*  API publique  */

/**
 * @brief Cherche le meilleur coup à jouer depuis une position donnée.
 * @param start La position de départ.
 * @param blueToPlay true si c'est au tour du joueur bleu, false sinon.
 * @return Le meilleur coup trouvé.
 */
Move search_best_move(const Board* start, bool blueToPlay);

/**
 * @brief Initialise les composants de l'IA (tables Zobrist, TT). Ne s'exécute qu'une seule fois.
 */
void ia_init_once(void);

/**
 * @brief Fait jouer l'IA pour le camp bleu.
 */
void ia_play_blue();

/**
 * @brief Fait jouer l'IA pour le camp rouge.
 */
void ia_play_red();

/**
 * @brief Vérifie s'il y a un vainqueur sur le plateau de l'IA.
 * @param b Le plateau à vérifier.
 * @return 1 si le bleu gagne, -1 si le rouge gagne, 0 sinon.
 */
int check_winner(const Board* b);

// Fonctions de jeu.h nécessaires pour l'intégration
void select_case(Case *cell);
void unselect_case(void);
void on_cell_clicked_tcp(GtkWidget *button, gpointer user_data);

#endif