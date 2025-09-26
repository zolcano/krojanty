/**
 * @file jeu_logique.c
 * @brief Implémentation du moteur de jeu et de toutes les règles de Krojanty.
 * @authors Groupe 8
 *
 * jeu_logique.c contient la logique pure du jeu, sans aucune dépendance à une
 * interface graphique. Il est conçu pour être testable de manière unitaire.
 */
#include "jeu_logique.h"
#include <string.h>
#include <stdio.h>

/**
 * @var board_init
 * @brief L'état initial du plateau, utilisé pour initialiser une nouvelle partie.
 */
static int board_init[SIZE][SIZE] = {
    {EMPTY, EMPTY, SOLDAT_BLEU, SOLDAT_BLEU, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, ROI_BLEU, SOLDAT_BLEU, SOLDAT_BLEU, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {SOLDAT_BLEU, SOLDAT_BLEU, SOLDAT_BLEU, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {SOLDAT_BLEU, SOLDAT_BLEU, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, SOLDAT_ROUGE, SOLDAT_ROUGE},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, SOLDAT_ROUGE, SOLDAT_ROUGE, SOLDAT_ROUGE},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, SOLDAT_ROUGE, SOLDAT_ROUGE, ROI_ROUGE, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, SOLDAT_ROUGE, SOLDAT_ROUGE, EMPTY, EMPTY}};

// Fonctions utilitaires internes 
/**
 * @brief Vérifie si les coordonnées sont dans les limites du plateau.
 * @param r Ligne.
 * @param c Colonne.
 * @return true si les coordonnées sont valides, false sinon.
 */
static inline bool in_bounds(int r, int c) { return r >= 0 && r < SIZE && c >= 0 && c < SIZE; }

/**
 * @brief Vérifie si un pion est bleu.
 * @param p Type de pion.
 * @return true si le pion est bleu, false sinon.
 */
static inline bool is_blue(int p) { return p == SOLDAT_BLEU || p == ROI_BLEU; }

/**
 * @brief Vérifie si un pion est rouge.
 * @param p Type de pion.
 * @return true si le pion est rouge, false sinon.
 */
static inline bool is_red(int p) { return p == SOLDAT_ROUGE || p == ROI_ROUGE; }

/**
 * @see jeu_logique.h
 */
void logique_init_game(GameState *state) {
    memcpy(state->pion, board_init, sizeof(board_init));
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (is_blue(state->pion[i][j])) state->couleur[i][j] = 2; // Bleu
            else if (is_red(state->pion[i][j])) state->couleur[i][j] = 1; // Rouge
            else state->couleur[i][j] = 0; // Neutre
        }
    }
    state->dead_blue_count = 0;
    state->dead_red_count = 0;
    state->tour = 1;
    state->game_over_status = 0; // 0 = En cours
}

/**
 * @see jeu_logique.h
 */
void logique_capture(GameState *state, int r, int c, const char* mouvement) {
    if (state->game_over_status != 0) return;

    int dr = 0, dc = 0;
    if (strcmp(mouvement, "haut") == 0) dr = -1;
    else if (strcmp(mouvement, "bas") == 0) dr = 1;
    else if (strcmp(mouvement, "gauche") == 0) dc = -1;
    else if (strcmp(mouvement, "droite") == 0) dc = 1;

    int r_victim = r + dr;
    int c_victim = c + dc;
    if (!in_bounds(r_victim, c_victim)) return;

    int attaquant = state->pion[r][c];
    int victime = state->pion[r_victim][c_victim];

    // La capture n'est possible que sur un pion adverse
    if (victime == EMPTY || (is_blue(attaquant) == is_blue(victime))) return;

    // Vérification de la protection par un garde
    int r_guard = r + 2 * dr;
    int c_guard = c + 2 * dc;
    bool est_garde = false;
    if (in_bounds(r_guard, c_guard)) {
        int garde = state->pion[r_guard][c_guard];
        // S'il y a une pièce derrière la victime et qu'elle est ennemie de l'attaquant
        if (garde != EMPTY && (is_blue(attaquant) != is_blue(garde))) {
            est_garde = true;
        }
    }

    // Si la victime n'est pas gardée, on la capture
    if (!est_garde) {
        state->pion[r_victim][c_victim] = EMPTY;
        if (is_red(victime)) {
            if (victime == ROI_ROUGE) state->game_over_status = 2; // Bleu gagne
            else state->dead_red_count++;
        } else {
            if (victime == ROI_BLEU) state->game_over_status = 1; // Rouge gagne
            else state->dead_blue_count++;
        }
    }
}

/**
 * @brief Sous-fonction pour la prise, vérifie une direction.
 *
 * Vérifie si une prise en sandwich est possible dans une direction donnée à partir
 * de la pièce attaquante. Si les conditions sont remplies (victime adverse entre
 * deux pièces alliées), la victime est retirée du plateau.
 *
 * @param state Pointeur vers l'état du jeu.
 * @param attaquant Le type de la pièce qui attaque.
 * @param r_near Ligne de la case adjacente (victime potentielle).
 * @param c_near Colonne de la case adjacente.
 * @param r_far Ligne de la case deux cases plus loin (allié potentiel).
 * @param c_far Colonne de la case deux cases plus loin.
 */
static void logique_internal_prise_check(GameState *state, int attaquant, int r_near, int c_near, int r_far, int c_far) {
    if (!in_bounds(r_near, c_near) || !in_bounds(r_far, c_far)) return;

    int victime = state->pion[r_near][c_near];
    int allie = state->pion[r_far][c_far];

    // Condition du sandwich : attaquant et allié sont de la même couleur, victime est de couleur opposée
    if (victime != EMPTY && allie != EMPTY && (is_blue(attaquant) == is_blue(allie)) && (is_blue(attaquant) != is_blue(victime))) {
        state->pion[r_near][c_near] = EMPTY;
        if (is_red(victime)) {
            if (victime == ROI_ROUGE) state->game_over_status = 2; // Bleu gagne
            else state->dead_red_count++;
        } else {
            if (victime == ROI_BLEU) state->game_over_status = 1; // Rouge gagne
            else state->dead_blue_count++;
        }
    }
}

/**
 * @see jeu_logique.h
 */
void logique_prise(GameState *state, int r, int c) {
    if (state->game_over_status != 0) return;
    int attaquant = state->pion[r][c];
    if (attaquant == EMPTY) return;

    // Vérifie les 4 directions pour une prise en sandwich
    logique_internal_prise_check(state, attaquant, r - 1, c, r - 2, c); // Haut
    logique_internal_prise_check(state, attaquant, r + 1, c, r + 2, c); // Bas
    logique_internal_prise_check(state, attaquant, r, c - 1, r, c - 2); // Gauche
    logique_internal_prise_check(state, attaquant, r, c + 1, r, c + 2); // Droite
}


/**
 * @see jeu_logique.h
 */
void logique_calculer_scores(GameState *state) {
    int score_rouge = 0, score_bleu = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            // Points pour les cases contrôlées
            if (state->couleur[i][j] == 1) score_rouge++;
            else if (state->couleur[i][j] == 2) score_bleu++;
            
            // Points pour les soldats restants (le roi ne compte pas)
            if (state->pion[i][j] == SOLDAT_ROUGE) score_rouge++;
            else if (state->pion[i][j] == SOLDAT_BLEU) score_bleu++;
        }
    }
    
    if (score_rouge > score_bleu) state->game_over_status = 1; // Rouge gagne
    else if (score_bleu > score_rouge) state->game_over_status = 2; // Bleu gagne
    else state->game_over_status = 3; // Égalité
}

/**
 * @see jeu_logique.h
 */
void logique_verifier_conditions_fin(GameState *state) {
    if (state->game_over_status != 0) return; // Une victoire par capture a déjà eu lieu

    // Conquête : roi rouge sur A9 (ville bleue) ou roi bleu sur I1 (ville rouge)
    // A9 = (row 0, col 0), I1 = (row 8, col 8) dans cette logique
    if (state->pion[0][0] == ROI_ROUGE) { state->game_over_status = 1; return; }
    if (state->pion[8][8] == ROI_BLEU) { state->game_over_status = 2; return; }

    // Extermination : 8 soldats adverses capturés
    if (state->dead_red_count >= 8) { state->game_over_status = 2; return; }
    if (state->dead_blue_count >= 8) { state->game_over_status = 1; return; }

    // Limite de tour atteinte
    if (state->tour > 64) {
        logique_calculer_scores(state);
    }
}