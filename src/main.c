/**
 * @file main.c
 * @authors Groupe 8
 * @brief Point d'entrée principal de l'application Krojanty.
 *
 * Ce fichier gère l'initialisation de l'application, l'analyse des arguments
 * de la ligne de commande pour déterminer le mode de jeu (local, serveur, ou client),
 * et lance l'interface graphique GTK ainsi que le processus réseau si nécessaire.
 */


#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "ia.h"

#include "config.h"
#include "plateau.h"
#include "reseau.h"
#include "reseau_integration.h"


/**
 * @var config
 * @brief Instance globale de la configuration du jeu.
 */
GameConfig config;

/**
 * @brief activate construit et affiche la fenêtre principale du jeu GTK.
 *
 * Elle est le callback principal de l'application GTK. Elle crée la fenêtre,
 * les labels d'information, la grille de jeu, et organise les widgets.
 * @param app Pointeur vers l'application GtkApplication.
 * @param user_data Données utilisateur (non utilisé ici).
 */
static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;
    printf("%s\n", "Activation de l'interface graphique.\n");
    load_css();

    config.window = gtk_application_window_new(app);
    if (config.mode == LOCAL)
    {
        gtk_window_set_title(GTK_WINDOW(config.window), "Krojanty");
    }
    else if (config.mode == SERVER)
    {
        gtk_window_set_title(GTK_WINDOW(config.window), "Krojanty (server)");
    }
    else
    {
        gtk_window_set_title(GTK_WINDOW(config.window), "Krojanty (client)");
    }

    gtk_window_set_default_size(GTK_WINDOW(config.window), 800, 500);
    gtk_window_set_resizable(GTK_WINDOW(config.window), FALSE);

    config.tour_label = gtk_label_new("Tour : 1");
    if (config.mode != LOCAL)
    {
        config.couleur_label = gtk_label_new("Tour des bleus (client)");
    }
    else
    {
        config.couleur_label = gtk_label_new("Tour des bleus");
    }
    config.red_death_label = gtk_label_new("mort.s bleu : 0");
    config.blue_death_label = gtk_label_new("mort.s rouge : 0");

    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(info_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(info_box, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(info_box, 150, -1);
    gtk_box_append(GTK_BOX(info_box), config.tour_label);
    gtk_box_append(GTK_BOX(info_box), config.couleur_label);
    gtk_box_append(GTK_BOX(info_box), config.blue_death_label);
    gtk_box_append(GTK_BOX(info_box), config.red_death_label);

    GtkWidget *grid = gtk_grid_new();
    init_plateau(grid);
    GtkWidget *grid_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(grid_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(grid_box, 30);
    gtk_widget_set_margin_bottom(grid_box, 30);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 1);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 1);
    gtk_box_append(GTK_BOX(grid_box), grid);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 50);
    gtk_widget_set_halign(main_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(main_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), grid_box);
    gtk_box_append(GTK_BOX(main_box), info_box);
    gtk_window_set_child(GTK_WINDOW(config.window), main_box);

    gtk_window_present(GTK_WINDOW(config.window));
}


/**
 * @brief network_thread_func est exécutée par le processus d'écoute réseau.
 *
 * Elle lance une boucle infinie pour recevoir les mouvements
 * de l'adversaire sur le réseau.
 * @param arg Arguments pour le processus.
 * @return NULL.
 */
static void *network_thread_func(void *arg)
{
    (void)arg;
    network_listen_loop(); // boucle de réception réseau
    return NULL;
}

/**
 * Affiche comment utiliser le programme en cas d'arguments incorrects.
 */
static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  ./game -l\n");
    fprintf(stderr, "      Lancer une partie en mode local graphique (2 joueurs).\n\n");
    fprintf(stderr, "  ./game -s [-ia] <port>\n");
    fprintf(stderr, "      Lancer en mode serveur sur le <port> spécifié.\n");
    fprintf(stderr, "      -ia : L'IA jouera pour le serveur.\n\n");
    fprintf(stderr, "  ./game -c [-ia] <adresse:port>\n");
    fprintf(stderr, "      Se connecter à un serveur à <adresse:port>.\n");
    fprintf(stderr, "      -ia : L'IA jouera pour le client.\n");
}

/**
 * @brief start_server_game lance le jeu en mode serveur.
 *
 * Elle initialise le réseau en attente d'un client, crée l'application GTK,
 * et lance le processus d'écoute réseau.
 */
static void start_server_game()
{
    printf("Lancement du SERVEUR sur le port %d. IA active: %s\n",
           config.port, config.ai ? "Oui" : "Non");

    printf("%s\n", "En attente de client...");
    int sock = net_wait_for_client();
    printf("%s\n", "client connecté");
    network_init(sock, 1); // 1 = serveur (rouge)

    GtkApplication *app = gtk_application_new("org.example.krojanty.serv", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    pthread_t net_thread;
    pthread_create(&net_thread, NULL, network_thread_func, NULL);
    pthread_detach(net_thread);

    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
    (void)status;
    return;
}

/**
 * @brief Pour faire jouer l'IA bleue après un délai.
 * @param user_data Données utilisateur.
 * @return FALSE pour que le timeout ne se répète pas.
 */
static gboolean ia_blue_timeout(gpointer user_data)
{
    (void)user_data;
    ia_play_blue(); // appelle ton IA
    return FALSE;   // exécuter une seule fois puis arrêter
}

/**
 * @brief start_client_game lance le jeu en mode client.
 *
 * Elle se connecte au serveur, initialise l'application GTK et le thread réseau.
 * Si l'IA est activée, elle joue automatiquement le premier coup.
 */
static void start_client_game()
{
    printf("Lancement du CLIENT vers %s:%d. IA active: %s\n",
           config.address, config.port, config.ai ? "Oui" : "Non");

    int sock = net_connect_to_server();
    printf("%s\n", "connexion reussie");
    network_init(sock, 0); // 0 = client (bleu)

    GtkApplication *app = gtk_application_new("org.example.krojanty.cli", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    pthread_t net_thread;
    pthread_create(&net_thread, NULL, network_thread_func, NULL);
    pthread_detach(net_thread);

    if (config.ai)
    {
        g_timeout_add_seconds(2, ia_blue_timeout, NULL);
    }

    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
    (void)status;

    return;
}

/**
 * fonction d'entrée du programme
 */
int main(int argc, char **argv)
{
    config.mode = ERROR;

    // vérification des arguments
    if (argc < 2)
    {
        print_usage();
        return 1;
    }
    if (strcmp(argv[1], "-l") == 0)
    {
        config.mode = LOCAL;
        if (argc == 3 && strcmp(argv[2], "-ia") == 0)
        {
            config.ai = true;
        }
        else if (argc > 2)
        {
            config.mode = ERROR;
        }
    }
    else if (strcmp(argv[1], "-c") == 0)
    {
        config.mode = CLIENT;
        char *portAddress = NULL;

        if (argc >= 4 && strcmp(argv[2], "-ia") == 0)
        {
            config.ai = true;
            portAddress = argv[3];
        }
        else if (argc == 3)
        {
            portAddress = argv[2];
        }
        else
        {
            config.mode = ERROR;
            print_usage();
            return 1;
        }

        if (portAddress)
        {
            char *colon = strchr(portAddress, ':');
            if (colon)
            {
                *colon = '\0';
                strncpy(config.address, portAddress, 15);
                config.address[15] = '\0';
                config.port = atoi(colon + 1);
            }
            else
            {
                fprintf(stderr, "Erreur: Format client invalide. Attendu: <adresse:port>\n");
                config.mode = ERROR;
            }
        }
    }
    else if (strcmp(argv[1], "-s") == 0)
    {
        config.mode = SERVER;
        char *portStr = NULL;

        if (argc >= 4 && strcmp(argv[2], "-ia") == 0)
        {
            config.ai = true;
            portStr = argv[3];
        }
        else if (argc == 3)
        {
            portStr = argv[2];
        }
        else
        {
            config.mode = ERROR;
            print_usage();
            return 1;
        }

        config.port = atoi(portStr);
        if (config.port <= 0)
        {
            fprintf(stderr, "Erreur: Port invalide.\n");
            config.mode = ERROR;
        }
    }
    else
    {
        config.mode = ERROR;
    }

    if (config.mode == ERROR)
    {
        print_usage();
        return 1;
    }

    // Lancement du mode de jeu
    switch (config.mode)
    {
    case LOCAL:
    {
        GtkApplication *app = gtk_application_new("org.example.krojanty", G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

        int status = g_application_run(G_APPLICATION(app), 0, NULL);
        g_object_unref(app);
        return status;
    }
    case SERVER:
        start_server_game();
        break;
    case CLIENT:
        start_client_game();
        break;
    case ERROR:
    default:
        if (argc > 1)
        { // N'affiche pas d'erreur si l'erreur vient d'un manque d'args
            fprintf(stderr, "Erreur dans les arguments fournis.\n");
        }
        print_usage();
        return 1;
    }

    return 0;
}