/**
 * @file jeu.h
 * @authors Groupe 8
 * @brief jeu.h déclare les fonctions et variables globales relatives à la logique du jeu et aux interactions.
 *
 * Il contient les prototypes des fonctions qui gèrent le déroulement du jeu,
 * les actions des joueurs, et l'application des règles de Krojanty.
 */

#ifndef JEU_H
#define JEU_H

#include <gtk/gtk.h>
#include "plateau.h"

// Variables globales de l'état du jeu 
extern int dead_red_count;  /**< Compteur des soldats rouges capturés. */
extern int dead_blue_count; /**< Compteur des soldats bleus capturés. */
extern int tour;            /**< Numéro du tour actuel (impair: bleu, pair: rouge). */
extern int game_over;       /**< Indicateur de fin de partie (1 si terminée, 0 sinon). */

// Callbacks GTK pour les clics sur les cases 

/**
 * @brief Callback appelé lors d'un clic sur une case en mode de jeu local.
 * @param button Le widget GtkButton qui a été cliqué.
 * @param user_data Pointeur vers la structure Case associée au bouton.
 */
void on_cell_clicked_local(GtkWidget *button, gpointer user_data);

/**
 * @brief Callback appelé lors d'un clic sur une case en mode réseau.
 * @param button Le widget GtkButton qui a été cliqué.
 * @param user_data Pointeur vers la structure Case associée au bouton.
 */
void on_cell_clicked_tcp(GtkWidget *button, gpointer user_data);

// Fonctions des règles du jeu

/**
 * @brief Applique la règle de capture par sandwich ("Linca") autour de la case spécifiée.
 * @param cell La case où le pion vient de se déplacer.
 */
void prise(Case *cell);

/**
 * @brief Gère la fin de la partie.
 * @param fatal Indique si la victoire est immédiate (1) ou par décompte de points (0).
 * @param color Le camp du vainqueur (1: rouge, 2: bleu) si la victoire est fatale.
 */
void endgame(int fatal, int color);

/**
 * @brief Met à jour les compteurs de pions morts et vérifie une condition de victoire par extermination.
 */
void check_dead_count();

/**
 * @brief Vide une case de son pion et réinitialise sa couleur de contrôle.
 * @param cell La case à vider.
 */
void clear_case(Case *cell);

/**
 * @brief Gère la sélection d'une case par le joueur.
 * @param cell La case sélectionnée.
 */
void select_case(Case *cell);

/**
 * @brief Annule la sélection de la case courante.
 */
void unselect_case();

/**
 * @brief Applique la règle de capture par poussée ("Seultou").
 * @param cell La case où le pion vient de se déplacer.
 * @param mouvement La direction du mouvement ("haut", "bas", "gauche", "droite").
 */
void capture(Case *cell, char *mouvement);

#endif