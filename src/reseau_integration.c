/**
 * @file reseau_integration.c
 * @brief Intégration de la couche réseau avec la logique de jeu et l'interface GTK.
 * @authors Groupe 8
 *
 * Il fait le lien entre les données brutes reçues du réseau et leur
 * application concrète sur le plateau de jeu. Il gère la boucle d'écoute
 * réseau dans un processus séparé et assure que les mouvements reçus sont
 * appliqués de manière sûre à l'interface GTK via `g_idle_add`.
 */

#include "reseau.h"
#include "plateau.h"
#include "reseau_integration.h"
#include "jeu.h"
#include "ia.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Variables globales pour gérer l'état réseau
int net_socket = -1; /**< Socket utilisé pour la communication. */
int is_network = 0;  /**< 1 si la partie est en réseau, 0 sinon. */
int is_server = 0;   /**< 1 si le joueur est le serveur (rouge), 0 si client (bleu). */
extern int tour;

/**
 * @see reseau_integration.h
 */
void network_init(int sock, int server_mode)
{
    net_socket = sock;
    is_network = 1;
    is_server = server_mode;
}


/**
 * @brief Vérifie si une partie en réseau est active.
 * @return 1 si en réseau, 0 sinon.
 */
int network_is_active()
{
    return is_network;
}

/**
 * @see reseau_integration.h
 */
void send_move_to_network(const char *src_id, const char *dst_id)
{
    if (!is_network || net_socket < 0)
        return;

    char move[5];
    snprintf(move, sizeof(move), "%s%s", src_id, dst_id); // ex: A1A3
    net_send_move(net_socket, move);

    printf("%s%s\n", "envoie : ", move);
}

/**
 * @brief apply_network_move_ui applique un coup reçu du réseau à l'interface utilisateur.
 *
 * Elle est appelée de manière asynchrone par GTK (`g_idle_add`)
 * pour garantir qu'elle s'exécute dans le processus principal de l'interface
 * graphique, évitant ainsi les problèmes de concurrence.
 *
 * @param user_data Pointeur vers une structure `MoveData` contenant le coup.
 * @return FALSE pour que la fonction ne soit exécutée qu'une seule fois.
 */
gboolean apply_network_move_ui(gpointer user_data)
{
    MoveData *data = (MoveData *)user_data;

    Case *cellsrc = get_case_by_id(data->src_id);
    if (!cellsrc)
    {
        fprintf(stderr, "Source invalide : %s\n", data->src_id);
        free(data);
        return FALSE;
    }

    select_case(cellsrc);

    Case *celldst = get_case_by_id(data->dst_id);
    if (!celldst)
    {
        fprintf(stderr, "Destination invalide : %s\n", data->dst_id);
        unselect_case();
        free(data);
        return FALSE;
    }

    const char *label = gtk_button_get_label(GTK_BUTTON(celldst->button));
    if (!label || strcmp(label, "•") != 0)
    {
        printf("déplacement du pion interdit !\n");
        unselect_case();
        free(data);
        return FALSE;
    }

    int selected_color = selected->couleur;
    int selected_pion = selected->pion;
    const char *str = gtk_button_get_label(GTK_BUTTON(selected->button));
    char *selected_button_label = strdup(str);

    char *mouvement = NULL;
    if (gtk_widget_has_css_class(celldst->button, "haut"))
    {
        mouvement = "haut";
    }
    else if (gtk_widget_has_css_class(celldst->button, "bas"))
    {
        mouvement = "bas";
    }
    else if (gtk_widget_has_css_class(celldst->button, "droite"))
    {
        mouvement = "droite";
    }
    else if (gtk_widget_has_css_class(celldst->button, "gauche"))
    {
        mouvement = "gauche";
    }

    selected->pion = EMPTY;
    gtk_button_set_label(GTK_BUTTON(selected->button), "");

    if (strcmp(selected->id, "A9") == 0)
    {
        Case *ville1 = get_case_by_id("A9");
        gtk_button_set_label(GTK_BUTTON(ville1->button), "市");
        gtk_widget_remove_css_class(ville1->button, "haut");
        set_cell_color(ville1, 2);
    }
    else if (strcmp(selected->id, "I1") == 0)
    {
        Case *ville2 = get_case_by_id("I1");
        gtk_button_set_label(GTK_BUTTON(ville2->button), "市");
        gtk_widget_remove_css_class(ville2->button, "haut");
        set_cell_color(ville2, 1);
    }

    unselect_case();

    set_cell_color(celldst, selected_color);
    celldst->pion = selected_pion;
    gtk_button_set_label(GTK_BUTTON(celldst->button), selected_button_label);
    g_free(selected_button_label);

    // Règles de jeu
    capture(celldst, mouvement);
    prise(celldst);

    // Vérification des conditions de fin
    Case *verif_ville_bleu = get_case_by_id("A9");
    if (verif_ville_bleu && verif_ville_bleu->pion == ROI_ROUGE)
    {
        endgame(1, 2);
        free(data);
        return FALSE;
    }

    Case *verif_ville_rouge = get_case_by_id("I1");
    if (verif_ville_rouge && verif_ville_rouge->pion == ROI_BLEU)
    {
        endgame(1, 1);
        free(data);
        return FALSE;
    }

    // Incrément du tour
    tour += 1;
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%d", tour);
    char result[120] = "Tour : ";
    strcat(result, buffer);
    gtk_label_set_text(GTK_LABEL(config.tour_label), result);

    if (tour == 65)
    {
        endgame(0, 0);
        free(data);
        return FALSE;
    }

    const char *texte = gtk_label_get_text(GTK_LABEL(config.couleur_label));
    if (tour % 2 != 0 &&
        (strcmp(texte, "Tour des rouges (serveur)") == 0 || strcmp(texte, "Tour des bleus (client)") == 0))
    {
        gtk_label_set_text(GTK_LABEL(config.couleur_label), "Tour des bleus (client)");
    }
    else if (strcmp(texte, "Tour des rouges (serveur)") == 0 || strcmp(texte, "Tour des bleus (client)") == 0)
    {
        gtk_label_set_text(GTK_LABEL(config.couleur_label), "Tour des rouges (serveur)");
    }

    // Lancer l’IA si activée
    if (config.ai)
    {
        if (config.mode == SERVER && tour % 2 == 0)
        {
            ia_play_red();
        }
        else if (config.mode == CLIENT && tour % 2 != 0)
        {
            ia_play_blue();
        }
    }

    free(data);
    return FALSE;
}

/**
 * @see reseau_integration.h
 */
void apply_network_move(const char *move)
{
    printf("reçu : %s\n", move);

    if (strlen(move) != 4 ||
        !isalpha(move[0]) || !isdigit(move[1]) ||
        !isalpha(move[2]) || !isdigit(move[3]))
    {
        fprintf(stderr, "Erreur : format de move invalide : %s\n", move);
        return;
    }

    MoveData *data = malloc(sizeof(MoveData));
    strncpy(data->src_id, move, 2);
    data->src_id[2] = '\0';
    strncpy(data->dst_id, move + 2, 2);
    data->dst_id[2] = '\0';

    g_idle_add(apply_network_move_ui, data);
}

/**
 * @see reseau_integration.h
 */
void network_listen_loop()
{
    if (!is_network || net_socket < 0)
    {
        printf("pas en réseau, fonction network listen loop\n");
        return;
    }

    while (1)
    {
        char buffer[5];
        if (net_recv_move(net_socket, buffer) == 0)
        {
            apply_network_move(buffer);
        }
        else
        {
            continue;
        }
    }
}
