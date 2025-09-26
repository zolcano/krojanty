/**
 * @file plateau.c
 * @brief Implémentation de la gestion du plateau de jeu GTK pour Krojanty.
 * @authors Groupe 8
 *
 * plateau.c est responsable de la création, de l'initialisation et de la
 * mise à jour visuelle du plateau de jeu. Il contient la logique pour
 * dessiner la grille, placer les pièces, appliquer les styles CSS, et
 * fournir des fonctions d'accès aux cases du plateau.
 */

#include "plateau.h"
#include "jeu.h"
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

Case *plateau[SIZE][SIZE]; /**< @see plateau.h */
Case *selected = NULL;     /**< @see plateau.h */

/**
 * @var board_init
 * @brief Matrice définissant la position initiale des pièces sur le plateau.
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

/**
 * @see plateau.h
 */
void load_css(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
                                      "button.flat { border: 1px solid black; padding: 10px; font-size: 20px; font-weight: bold; }"
                                      "button.flat.red { background-color: rgba(255, 95, 95, 0.81); }"
                                      "button.flat.blue { background-color: rgba(89, 89, 255, 0.75); }"
                                      "button.flat.diagred { background-color: rgba(177, 65, 65, 0.81); }"
                                      "button.flat.diagblue { background-color: rgba(59, 59, 177, 0.75); }"
                                      "button.flat.selected { border: 1px solid rgba(251, 255, 0, 1); }"
                                      "button.flat.diagonal { background-color: rgba(255, 211, 89, 0.4); }");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

/**
 * @see plateau.h
 */
void set_cell_color(Case *cell, int color) // 0 = blanc | 1 = rouge | 2 = bleu
{
    // vérifier si le widget possède la class "diagonal"
    if (gtk_widget_has_css_class(cell->button, "diagonal") || gtk_widget_has_css_class(cell->button, "diagblue") || gtk_widget_has_css_class(cell->button, "diagred"))
    {
        gtk_widget_remove_css_class(cell->button, "diagonal");
        gtk_widget_remove_css_class(cell->button, "diagred");
        gtk_widget_remove_css_class(cell->button, "diagblue");

        if (color == 1)
            gtk_widget_add_css_class(cell->button, "diagred");
        else if (color == 2)
            gtk_widget_add_css_class(cell->button, "diagblue");
        else if (color == 0)
            gtk_widget_add_css_class(cell->button, "diagonal");

        if (strcmp(cell->id, "A9") != 0 || strcmp(cell->id, "I1") != 0)
        {
            cell->couleur = color;
        }
        return;
    }

    gtk_widget_remove_css_class(cell->button, "red");
    gtk_widget_remove_css_class(cell->button, "blue");

    if (color == 1)
        gtk_widget_add_css_class(cell->button, "red");
    else if (color == 2)
        gtk_widget_add_css_class(cell->button, "blue");

    if (strcmp(cell->id, "A9") != 0 || strcmp(cell->id, "I1") != 0)
    {
        cell->couleur = color;
    }

    gtk_widget_queue_draw(cell->button);
}

/**
 * @see plateau.h
 */
void init_plateau(GtkWidget *grid)
{
    for (int col = 0; col < SIZE; col++) // Ajouter les lettres A à I en haut (ligne 0)
    {
        gchar label_text[2];
        g_snprintf(label_text, sizeof(label_text), "%c", 'A' + col);
        GtkWidget *label = gtk_label_new(label_text);
        gtk_widget_set_size_request(label, 20, 20);
        gtk_grid_attach(GTK_GRID(grid), label, col + 1, 0, 1, 1); // Décalé à droite
    }

    for (int row = 0; row < SIZE; row++) // Ajouter les chiffres 9 à 1 sur la gauche (colonne 0)
    {
        gchar label_text[2];
        g_snprintf(label_text, sizeof(label_text), "%d", SIZE - row); // de 9 à 1
        GtkWidget *label = gtk_label_new(label_text);
        gtk_widget_set_size_request(label, 20, 20);
        gtk_grid_attach(GTK_GRID(grid), label, 0, row + 1, 1, 1); // Décalé vers le bas
    }

    // Générer les cellules de jeu
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            Case *c = g_malloc(sizeof(Case));

            gchar id[4];
            g_snprintf(id, sizeof(id), "%c%d", 'A' + j, SIZE - i); // ID correspond à affichage
            c->id = g_strdup(id);
            c->pion = board_init[i][j];
            c->couleur = 0; // 0 = blanc | 1 = rouge | 2 = bleu

            GtkWidget *cell = gtk_button_new();
            gtk_widget_set_size_request(cell, 47, 47);
            gtk_widget_add_css_class(cell, "flat");
            c->button = cell;

            if (i + j == SIZE - 1)
            {
                gtk_widget_add_css_class(cell, "diagonal");
                c->couleur = 0;
            }

            if (strcmp(c->id, "A9") == 0)
            {
                gtk_button_set_label(GTK_BUTTON(cell), "市");
                c->couleur = 2;
            }
            else if (strcmp(c->id, "I1") == 0)
            {
                gtk_button_set_label(GTK_BUTTON(cell), "市");
                c->couleur = 1;
            }

            switch (c->pion)
            {
            case SOLDAT_ROUGE:
                gtk_button_set_label(GTK_BUTTON(cell), "♖");
                c->couleur = 1;
                break;
            case ROI_ROUGE:
                gtk_button_set_label(GTK_BUTTON(cell), "♔");
                c->couleur = 1;
                break;
            case SOLDAT_BLEU:
                gtk_button_set_label(GTK_BUTTON(cell), "♜");
                c->couleur = 2;
                break;
            case ROI_BLEU:
                gtk_button_set_label(GTK_BUTTON(cell), "♚");
                c->couleur = 2;
                break;
            default:
                break;
            }

            if (c->couleur)
                set_cell_color(c, c->couleur);
            plateau[i][j] = c;

            // Attacher la cellule en décalant de +1 pour lignes et colonnes
            gtk_grid_attach(GTK_GRID(grid), cell, j + 1, i + 1, 1, 1);

            if (config.mode == LOCAL)
            {
                g_signal_connect(cell, "clicked", G_CALLBACK(on_cell_clicked_local), c);
            }
            else if (config.mode == SERVER || config.mode == CLIENT)
            {
                g_signal_connect(cell, "clicked", G_CALLBACK(on_cell_clicked_tcp), c);
            }
        }
    }
}

/**
 * @see plateau.h
 */
Case *get_case_by_id(const gchar *id)
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (g_strcmp0(plateau[i][j]->id, id) == 0)
                return plateau[i][j];
        }
    }
    return NULL;
}
