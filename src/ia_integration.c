/**
 * @file ia_integration.c
 * @brief Fonctions qui lient le cerveau de l'IA à l'interface GTK.
 * @authors Groupe 8
 *
 * ia_integration.c sert de pont entre la logique de l'IA (ia.c) et l'état
 * du jeu géré par l'interface GTK (plateau.c, jeu.c). Il contient les
 * fonctions pour "photographier" l'état actuel du plateau et pour
 * simuler un clic sur l'interface afin de jouer le coup choisi par l'IA.
 */
#include "ia.h"
#include "config.h"
#include "jeu.h" 

/**
 * @brief Prend une "photo" de l'état du plateau GTK pour le donner à l'IA.
 *
 * Cette fonction parcourt le plateau de jeu géré par GTK (`plateau[r][c]`) et
 * copie l'état de chaque case (le type de pion) dans une structure `Board`
 * purement logique, qui peut être utilisée par les algorithmes de l'IA.
 *
 * @param b Pointeur vers la structure `Board` à remplir avec l'état actuel du jeu.
 */
static void snapshot_board(Board* b) {
    for (int r=0; r<SIZE; r++)
        for (int c=0; c<SIZE; c++)
            b->pion[r][c] = plateau[r][c]->pion;
}

/**
 * @see ia.h
 */
void ia_play_blue(void) {
    ia_init_once();
    if (config.mode == LOCAL) return;

    Board b; snapshot_board(&b);
    if (check_winner(&b) != 0) return;

    Move m = search_best_move(&b, true);
    if (m.r1<0) return;

    Case* from = plateau[m.r1][m.c1];
    Case* to   = plateau[m.r2][m.c2];
    select_case(from);
    on_cell_clicked_tcp(to->button, to);
}

/**
 * @see ia.h
 */
void ia_play_red(void) {
    ia_init_once();
    if (config.mode == LOCAL) return;

    Board b; snapshot_board(&b);
    if (check_winner(&b) != 0) return;

    Move m = search_best_move(&b, false);
    if (m.r1<0) return;

    Case* from = plateau[m.r1][m.c1];
    Case* to   = plateau[m.r2][m.c2];
    select_case(from);
    on_cell_clicked_tcp(to->button, to);
}
