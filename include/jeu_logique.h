/**
 * @file jeu_logique.h
 * @authors Groupe 8
 * @brief jeu_logique.h définit les structures et fonctions pour une logique de jeu pure, sans dépendance à GTK.
 *
 * Il est conçu pour être découplé de l'interface graphique afin de
 * permettre des tests unitaires des règles du jeu.
 */

#ifndef JEU_LOGIQUE_H
#define JEU_LOGIQUE_H

#include "plateau.h" 
#include <stdbool.h>

/**
 * @struct GameState
 * @brief Structure de données pure représentant l'état complet d'une partie.
 *
 * C'est cette structure qui est manipulée et vérifiée par les tests.
 */
typedef struct {
    int pion[SIZE][SIZE];       /**< L'état de chaque case du plateau. */
    int couleur[SIZE][SIZE];    /**< La couleur de contrôle de chaque case. */
    int dead_red_count;         /**< Compteur de soldats rouges morts. */
    int dead_blue_count;        /**< Compteur de soldats bleus morts. */
    int tour;                   /**< Numéro du tour actuel. */
    int game_over_status;       /**< 0: en cours, 1: rouge gagne, 2: bleu gagne, 3: égalité. */
} GameState;

// Fonctions publiques de la bibliothèque de logique 

/**
 * @brief Initialise un GameState à sa configuration de départ.
 * @param state Pointeur vers l'état de jeu à initialiser.
 */
void logique_init_game(GameState *state);

/**
 * @brief Applique la logique de capture par poussée ("Seultou").
 * @param state Pointeur vers l'état du jeu à modifier.
 * @param r Ligne de la pièce qui vient de bouger.
 * @param c Colonne de la pièce qui vient de bouger.
 * @param mouvement Direction du mouvement ("haut", "bas", "gauche", "droite").
 */
void logique_capture(GameState *state, int r, int c, const char* mouvement);

/**
 * @brief Applique la logique de capture par sandwich ("Linca").
 * @param state Pointeur vers l'état du jeu à modifier.
 * @param r Ligne de la pièce qui vient de bouger.
 * @param c Colonne de la pièce qui vient de bouger.
 */
void logique_prise(GameState *state, int r, int c);

/**
 * @brief Vérifie les conditions de victoire qui ne sont pas liées à une capture directe.
 * (Conquête, extermination par nombre, limite de tours).
 * @param state Pointeur vers l'état du jeu.
 */
void logique_verifier_conditions_fin(GameState *state);

/**
 * @brief Calcule les scores et détermine le vainqueur si la partie atteint la limite de tours.
 * @param state Pointeur vers l'état du jeu à mettre à jour.
 */
void logique_calculer_scores(GameState *state);

#endif 