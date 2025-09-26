/**
 * @file config.h
 * @authors Groupe 8
 * @brief config.h définit les structures et énumérations globales pour la configuration du jeu.
 *
 * Il centralise les types de données utilisés pour gérer l'état de l'application,
 * que ce soit le mode de jeu, les paramètres réseau, ou les pointeurs vers les widgets
 * de l'interface graphique GTK.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <glib-2.0/glib.h>
#include <gtk/gtk.h>

/**
 * @enum GameMode
 * @brief Énumération des modes de jeu possibles.
 */
typedef enum
{
    ERROR,  /**< Indique une erreur lors de l'analyse des arguments de la ligne de commande. */
    LOCAL,  /**< Mode de jeu local, deux joueurs sur la même machine. */
    SERVER, /**< Le programme agit en tant que serveur réseau. */
    CLIENT  /**< Le programme agit en tant que client réseau. */
} GameMode;

/**
 * @struct GameConfig
 * @brief Structure contenant la configuration globale de l'application.
 *
 * Cette structure regroupe les paramètres de jeu (mode, IA, réseau) ainsi que
 * les pointeurs vers les widgets GTK nécessaires à la mise à jour de l'interface.
 */
typedef struct
{
    //  La configuration du jeu 
    GameMode mode;    /**< Le mode de jeu actuel (LOCAL, SERVER, ou CLIENT). */
    gboolean ai;      /**< Booléen indiquant si l'IA est activée pour le joueur courant. */
    int port;         /**< Le port utilisé pour la communication réseau. */
    char address[16]; /**< L'adresse IP du serveur à laquelle se connecter (pour le client). */

    // Les Widgets de l'interface GTK
    GtkWidget *tour_label;        /**< Pointeur vers le label affichant le numéro du tour. */
    GtkWidget *couleur_label;     /**< Pointeur vers le label affichant le joueur dont c'est le tour. */
    GtkWidget *blue_death_label;  /**< Pointeur vers le label affichant le nombre de pions rouges capturés. */
    GtkWidget *red_death_label;   /**< Pointeur vers le label affichant le nombre de pions bleus capturés. */
    GtkWidget *window;            /**< Pointeur vers la fenêtre principale de l'application. */
} GameConfig;

/**
 * @var config
 * @brief Instance globale de la structure de configuration du jeu.
 */
extern GameConfig config;

#endif