/**
 * @file plateau.h
 * @authors Groupe 8
 * @brief plateau.h définit la structure du plateau de jeu et les entités qu'il contient.
 *
 * Il contient les définitions pour les pièces, la structure d'une case,
 * et les prototypes des fonctions de gestion du plateau de jeu.
 */

#ifndef PLATEAU_H
#define PLATEAU_H

#include <gtk/gtk.h>

#define SIZE 9 /**< Dimension du plateau de jeu (9x9). */

// Définitions des types de pièces
#define EMPTY 0          /**< Représente une case vide. */
#define SOLDAT_ROUGE 1   /**< Représente un soldat de l'équipe rouge. */
#define SOLDAT_BLEU 2    /**< Représente un soldat de l'équipe bleue. */
#define ROI_ROUGE 3      /**< Représente le roi de l'équipe rouge. */
#define ROI_BLEU 4       /**< Représente le roi de l'équipe bleue. */

/**
 * @struct Case
 * @brief Structure représentant une case unique sur le plateau de jeu.
 */
typedef struct
{
    gchar *id;         /**< Identifiant textuel de la case (ex: "A1", "I9"). */
    int pion;          /**< Identifiant du pion présent sur la case (voir les defines de pièces). */
    int couleur;       /**< Couleur de contrôle de la case (0: neutre, 1: rouge, 2: bleu). */
    GtkWidget *button; /**< Pointeur vers le widget GtkButton associé à la case. */
} Case;

// Variables globales pour le plateau
extern Case *plateau[SIZE][SIZE]; /**< Matrice 2D représentant l'ensemble du plateau de jeu. */
extern Case *selected;            /**< Pointeur vers la case actuellement sélectionnée par le joueur. */

// Prototypes de fonctions

/**
 * @brief Charge le fichier CSS pour styliser l'interface GTK.
 */
void load_css(void);

/**
 * @brief Applique une couleur de contrôle à une case.
 * @param cell Pointeur vers la case à modifier.
 * @param couleur Code de la couleur (0: neutre, 1: rouge, 2: bleu).
 */
void set_cell_color(Case *cell, int couleur);

/**
 * @brief Initialise le plateau de jeu GTK, crée les cases et place les pièces.
 * @param grid Le widget GtkGrid dans lequel le plateau sera dessiné.
 */
void init_plateau(GtkWidget *grid);

/**
 * @brief Récupère une case par son identifiant.
 * @param id L'identifiant de la case (ex: "A1").
 * @return Un pointeur vers la structure Case correspondante, ou NULL si non trouvée.
 */
Case *get_case_by_id(const gchar *id);

#endif