/**
 * @file jeu.c
 * @brief Implémentation de la logique de l'interface utilisateur et des règles du jeu Krojanty.
 * @authors Groupe 8
 *
 * jeu.c gère les interactions des joueurs avec l'interface GTK. Il contient
 * les callbacks pour les clics sur les cases, la gestion des tours, l'application
 * des règles de capture et de prise, ainsi que la vérification des conditions de
 * fin de partie. Il fait le lien entre les actions de l'utilisateur et la
 * mise à jour de l'état du jeu, à la fois visuellement et logiquement.
 */

#include "jeu.h"
#include "config.h"
#include "plateau.h"
#include "reseau_integration.h"

#include <gtk/gtk.h>
int game_over = 0;

int dead_red_count = 0;
int dead_blue_count = 0;
int tour = 1;

/**
 * @brief Vérifie si les coordonnées sont dans les limites du plateau.
 * @param r Ligne.
 * @param c Colonne.
 * @return 1 si les coordonnées sont valides, 0 sinon.
 */
static inline int in_bounds(int r, int c)
{
    return (r >= 0 && r < SIZE && c >= 0 && c < SIZE);
}

/**
 * @see jeu.h
 */
void endgame(int fatal, int color)
{
    game_over = 1;
    gtk_widget_set_sensitive(config.window, FALSE);
    if (fatal)
    {
        if (color == 1)
        {
            gtk_label_set_text(GTK_LABEL(config.couleur_label), "Victoire des rouges !");
        }
        else
        {
            gtk_label_set_text(GTK_LABEL(config.couleur_label), "Victoire des bleus !");
        }
    }
    else
    {
        int point_bleu = -1;
        int point_rouge = -1;

        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                gchar id[10];
                g_snprintf(id, sizeof(id), "%c%d", 'A' + j, SIZE - i);
                Case *temp = get_case_by_id(id);

                if (temp->couleur == 1)
                {
                    point_rouge += 1;
                }
                else if (temp->couleur == 2)
                {
                    point_bleu += 1;
                }

                if (temp->pion == SOLDAT_ROUGE)
                {
                    point_rouge += 1;
                }
                else if (temp->pion == SOLDAT_BLEU)
                {
                    point_bleu += 1;
                }
            }
        }
        gchar *resultat;
        if (point_rouge > point_bleu)
        {
            resultat = g_strdup_printf(
                "Victoire des rouges avec %d points \ncontre %d pour les bleus",
                point_rouge, point_bleu);
        }
        else if (point_rouge < point_bleu)
        {
            resultat = g_strdup_printf(
                "Victoire des bleus avec %d points \ncontre %d pour les rouges",
                point_bleu, point_rouge);
        }
        else
        {
            resultat = g_strdup_printf(
                "égalité !\n%d points bleu contre %d points rouges",
                point_bleu, point_rouge);
        }
        gtk_label_set_text(GTK_LABEL(config.couleur_label), resultat);
        return;
    }
}

/**
 * @see jeu.h
 */
void check_dead_count()
{
    if (dead_red_count >= 8)
    {
        endgame(1, 2);
    }
    else if (dead_blue_count >= 8)
    {
        endgame(1, 1);
    }

    // if no endgame, change the counter label
    char *temp1 = g_strdup_printf(
        "mort.s rouge : %d",
        dead_red_count);
    char *temp2 = g_strdup_printf(
        "mort.s bleu : %d",
        dead_blue_count);
    gtk_label_set_text(GTK_LABEL(config.blue_death_label), temp2);
    gtk_label_set_text(GTK_LABEL(config.red_death_label), temp1);
}

/**
 * @see jeu.h
 */
void clear_case(Case *cell)
{
    gtk_button_set_label(GTK_BUTTON(cell->button), "");
    cell->pion = EMPTY;
    set_cell_color(cell, 0);
    if (strcmp(cell->id, "A9") == 0)
    {
        gtk_button_set_label(GTK_BUTTON(cell->button), "市");
        set_cell_color(cell, 2);
    }
    else if (strcmp(cell->id, "I1") == 0)
    {
        gtk_button_set_label(GTK_BUTTON(cell->button), "市");
        set_cell_color(cell, 1);
    }
}

/**
 * @see jeu.h
 */
void select_case(Case *cell)
{
    if (!cell)
    {
        printf("%s\n", "case founie incorrect, impossible de sélectionner");
        return;
    }
    selected = cell;
    gtk_widget_add_css_class(selected->button, "selected"); // passe la case cliquée en sélectionnée

    // parsing des coordonnées
    char lettre = selected->id[0];
    char chiffre = selected->id[1];

    // changement du label des cases jouables à partir de la case sélectionnée
    int start = chiffre - '0';              // conversion string en int
    for (int i = start + 1; i <= SIZE; i++) // HAUT
    {
        char id[10];
        sprintf(id, "%c%d", lettre, i);
        Case *temp = get_case_by_id(id);
        if (!temp)
        {
            printf("%s\n", "case founie incorrect, erreur boucle haut fonction de sélection");
        }

        if (temp->pion) // arrêt de recherche à la première case contenant un pion
        {
            break;
        }
        gtk_button_set_label(GTK_BUTTON(temp->button), "•"); // case jouable affichée
        gtk_widget_add_css_class(temp->button, "haut");      // direction du pion si case jouée
    }

    for (int i = start - 1; i > 0; i--) // BAS
    {
        char id[10];
        sprintf(id, "%c%d", lettre, i);
        Case *temp = get_case_by_id(id);
        if (!temp)
        {
            printf("%s\n", "case founie incorrect, erreur boucle bas fonction de sélection");
        }

        if (temp->pion)
        {
            break;
        }
        gtk_button_set_label(GTK_BUTTON(temp->button), "•");
        gtk_widget_add_css_class(temp->button, "bas");
    }

    for (char i = lettre + 1; i <= 'I'; i++) // DROITE
    {
        char id[10];
        sprintf(id, "%c%c", i, chiffre);
        Case *temp = get_case_by_id(id);
        if (!temp)
        {
            printf("%s\n", "case founie incorrect, erreur boucle droite fonction de sélection");
        }

        if (temp->pion)
        {
            break;
        }
        gtk_button_set_label(GTK_BUTTON(temp->button), "•");
        gtk_widget_add_css_class(temp->button, "droite");
    }

    for (char i = lettre - 1; i >= 'A'; i--) // GAUCHE
    {
        char id[10];
        sprintf(id, "%c%c", i, chiffre);
        Case *temp = get_case_by_id(id);
        if (!temp)
        {
            printf("%s\n", "case founie incorrect, erreur boucle gauche fonction de sélection");
        }

        if (temp->pion)
        {
            break;
        }
        gtk_button_set_label(GTK_BUTTON(temp->button), "•");
        gtk_widget_add_css_class(temp->button, "gauche");
    }
}

/**
 * @see jeu.h
 */
void unselect_case()
{
    // même logique que pour la fonction sélectionner, sauf qu'on retire tous les labels donnés lors de la sélection
    char lettre = selected->id[0];
    char chiffre = selected->id[1];

    int start = chiffre - '0';
    for (int i = start + 1; i <= SIZE; i++) // HAUT
    {
        char id[10];
        sprintf(id, "%c%d", lettre, i);
        Case *temp = get_case_by_id(id);
        if (!temp)
        {
            printf("%s\n", "case founie incorrect, erreur boucle haut fonction de déselection");
        }

        if (temp->pion)
        {
            break;
        }
        else if (strcmp(id, "A9") == 0)
        {
            Case *ville1 = get_case_by_id("A9");
            gtk_button_set_label(GTK_BUTTON(ville1->button), "市");
            gtk_widget_remove_css_class(ville1->button, "haut");
        }
        else if (strcmp(id, "I1") == 0)
        {
            Case *ville2 = get_case_by_id("I1");
            gtk_button_set_label(GTK_BUTTON(ville2->button), "市");
            gtk_widget_remove_css_class(ville2->button, "haut");
        }
        else
        {
            gtk_button_set_label(GTK_BUTTON(temp->button), "");
            gtk_widget_remove_css_class(temp->button, "haut");
        }
    }

    for (int i = start - 1; i > 0; i--) // BAS
    {
        char id[10];
        sprintf(id, "%c%d", lettre, i);
        Case *temp = get_case_by_id(id);
        if (!temp)
        {
            printf("%s\n", "case founie incorrect, erreur boucle bas fonction de déselection");
        }

        if (temp->pion)
        {
            break;
        }
        else if (strcmp(id, "A9") == 0)
        {
            Case *ville1 = get_case_by_id("A9");
            gtk_button_set_label(GTK_BUTTON(ville1->button), "市");
            gtk_widget_remove_css_class(ville1->button, "haut");
        }
        else if (strcmp(id, "I1") == 0)
        {
            Case *ville2 = get_case_by_id("I1");
            gtk_button_set_label(GTK_BUTTON(ville2->button), "市");
            gtk_widget_remove_css_class(ville2->button, "haut");
        }
        else
        {
            gtk_button_set_label(GTK_BUTTON(temp->button), "");
            gtk_widget_remove_css_class(temp->button, "bas");
        }
    }

    for (char i = lettre + 1; i <= 'I'; i++) // DROITE
    {
        char id[10];
        sprintf(id, "%c%c", i, chiffre);
        Case *temp = get_case_by_id(id);
        if (!temp)
        {
            printf("%s\n", "case founie incorrect, erreur boucle droite fonction de déselection");
        }

        if (temp->pion)
        {
            break;
        }
        else if (strcmp(id, "A9") == 0)
        {
            Case *ville1 = get_case_by_id("A9");
            gtk_button_set_label(GTK_BUTTON(ville1->button), "市");
            gtk_widget_remove_css_class(ville1->button, "haut");
        }
        else if (strcmp(id, "I1") == 0)
        {
            Case *ville2 = get_case_by_id("I1");
            gtk_button_set_label(GTK_BUTTON(ville2->button), "市");
            gtk_widget_remove_css_class(ville2->button, "haut");
        }
        else
        {
            gtk_button_set_label(GTK_BUTTON(temp->button), "");
            gtk_widget_remove_css_class(temp->button, "droite");
        }
    }

    for (char i = lettre - 1; i >= 'A'; i--) // GAUCHE
    {
        char id[10];
        sprintf(id, "%c%c", i, chiffre);
        Case *temp = get_case_by_id(id);
        if (!temp)
        {
            printf("%s\n", "case founie incorrect, erreur boucle gauche fonction de déselection");
        }

        if (temp->pion)
        {
            break;
        }
        else if (strcmp(id, "A9") == 0)
        {
            Case *ville1 = get_case_by_id("A9");
            gtk_button_set_label(GTK_BUTTON(ville1->button), "市");
            gtk_widget_remove_css_class(ville1->button, "haut");
        }
        else if (strcmp(id, "I1") == 0)
        {
            Case *ville2 = get_case_by_id("I1");
            gtk_button_set_label(GTK_BUTTON(ville2->button), "市");
            gtk_widget_remove_css_class(ville2->button, "haut");
        }
        else
        {
            gtk_button_set_label(GTK_BUTTON(temp->button), "");
            gtk_widget_remove_css_class(temp->button, "gauche");
        }
    }

    gtk_widget_remove_css_class(selected->button, "selected");
    selected = NULL;
    return;
}

/**
 * @see jeu.h
 */
void capture(Case *cell, char *mouvement)
{
    bool ack = true;
    Case *victim_case;
    Case *guard_case;

    // récupération de coordonnées
    char lettre = cell->id[0];
    char chiffre = cell->id[1];
    if (strcmp(mouvement, "haut") == 0)
    {
        int tmp1 = (chiffre - '0') + 1;
        int tmp2 = (chiffre - '0') + 2;
        if (tmp1 > 9) // si coordonnée de la victime en dehors du plateau, alors fin de fonction
        {
            return;
        }
        else if (tmp2 > 9)
        {
            ack = false; // pas de garde
        }

        char victim[10];
        sprintf(victim, "%c%d", lettre, tmp1);

        victim_case = get_case_by_id(victim);
        if (!victim_case)
        {
            printf("%s\n", "erreur de fonction capture, victim haut");
            return;
        }

        if (ack)
        {
            char guard[10];
            sprintf(guard, "%c%d", lettre, tmp2);

            guard_case = get_case_by_id(guard);
            if (!guard_case)
            {
                printf("%s\n", "erreur de fonction capture, guard haut");
                return;
            }
        }
    }
    else if (strcmp(mouvement, "bas") == 0)
    {
        int tmp1 = (chiffre - '0') - 1;
        int tmp2 = (chiffre - '0') - 2;
        if (tmp1 < 1)
        {
            return;
        }
        else if (tmp2 < 1)
        {
            ack = false;
        }

        char victim[10];
        sprintf(victim, "%c%d", lettre, tmp1);
        victim_case = get_case_by_id(victim);
        if (!victim_case)
        {
            printf("%s\n", "erreur de fonction capture, victim bas");
            return;
        }

        if (ack)
        {
            char guard[10];
            sprintf(guard, "%c%d", lettre, tmp2);
            guard_case = get_case_by_id(guard);
            if (!guard_case)
            {
                printf("%s\n", "erreur de fonction capture, guard bas");
                return;
            }
        }
    }
    else if (strcmp(mouvement, "droite") == 0)
    {
        int tmp1 = lettre + 1;
        int tmp2 = lettre + 2;
        if (tmp1 > 'I')
        {
            return;
        }
        else if (tmp2 > 'I')
        {
            ack = false;
        }

        char victim[10];
        sprintf(victim, "%c%c", tmp1, chiffre);
        victim_case = get_case_by_id(victim);
        if (!victim_case)
        {
            printf("%s\n", "erreur de fonction capture, victim droite");
            return;
        }

        if (ack)
        {
            char guard[10];
            sprintf(guard, "%c%c", tmp2, chiffre);
            guard_case = get_case_by_id(guard);
            if (!guard_case)
            {
                printf("%s\n", "erreur de fonction capture, guard droite");
                return;
            }
        }
    }
    else if (strcmp(mouvement, "gauche") == 0)
    {
        int tmp1 = lettre - 1;
        int tmp2 = lettre - 2;
        if (tmp1 < 'A')
        {
            return;
        }
        else if (tmp2 < 'A')
        {
            ack = false;
        }

        char victim[10];
        sprintf(victim, "%c%c", tmp1, chiffre);
        victim_case = get_case_by_id(victim);
        if (!victim_case)
        {
            printf("%s\n", "erreur de fonction capture, victim gauche");
            return;
        }
        if (ack)
        {
            char guard[10];
            sprintf(guard, "%c%c", tmp2, chiffre);
            guard_case = get_case_by_id(guard);
            if (!guard_case)
            {
                printf("%s\n", "erreur de fonction capture, guard gauche");
                return;
            }
        }
    }
    // vérifications
    if ((cell->pion == SOLDAT_BLEU) || (cell->pion == ROI_BLEU))
    {
        if (ack)
        {
            if (guard_case->pion == SOLDAT_ROUGE || guard_case->pion == ROI_ROUGE)
            {
                return;
            }
            else
            {
                if (victim_case->pion == SOLDAT_ROUGE)
                {
                    clear_case(victim_case);
                    dead_red_count += 1;
                    check_dead_count();
                    return;
                }
                else if (victim_case->pion == ROI_ROUGE)
                {
                    clear_case(victim_case);
                    endgame(1, 2);
                    return;
                }
            }
        }
        else
        {
            if (victim_case->pion == SOLDAT_ROUGE)
            {
                clear_case(victim_case);
                dead_red_count += 1;
                check_dead_count();
                return;
            }
            else if (victim_case->pion == ROI_ROUGE)
            {
                clear_case(victim_case);
                endgame(1, 2);
                return;
            }
        }
    }
    else if ((cell->pion == SOLDAT_ROUGE) || (cell->pion == ROI_ROUGE))
    {
        if (ack)
        {
            if (guard_case->pion == SOLDAT_BLEU || guard_case->pion == ROI_BLEU)
            {
                return;
            }
            else
            {
                if (victim_case->pion == SOLDAT_BLEU)
                {
                    clear_case(victim_case);
                    dead_blue_count += 1;
                    check_dead_count();
                    return;
                }
                else if (victim_case->pion == ROI_BLEU)
                {
                    clear_case(victim_case);
                    endgame(1, 1);
                    return;
                }
            }
        }
        else
        {
            if (victim_case->pion == SOLDAT_BLEU)
            {
                clear_case(victim_case);
                dead_blue_count += 1;
                check_dead_count();
                return;
            }
            else if (victim_case->pion == ROI_BLEU)
            {
                clear_case(victim_case);
                endgame(1, 1);
                return;
            }
        }
    }
}

/**
 * @brief Vérifie et exécute une prise en sandwich dans une direction donnée.
 * @param cell La case de la pièce qui vient de bouger.
 * @param near_case La case adjacente (potentielle victime).
 * @param far_case La case de l'autre côté (potentiel allié).
 */
void prise_check(Case *cell, Case *near_case, Case *far_case)
{
    if (cell->pion == SOLDAT_ROUGE || cell->pion == ROI_ROUGE)
    {
        if (near_case->pion == SOLDAT_BLEU && (far_case->pion == SOLDAT_ROUGE || far_case->pion == ROI_ROUGE))
        {
            clear_case(near_case);
            dead_blue_count += 1;
            check_dead_count();
            return;
        }
        else if (near_case->pion == ROI_BLEU && (far_case->pion == SOLDAT_ROUGE || far_case->pion == ROI_ROUGE))
        {
            clear_case(near_case);
            endgame(1, 1);
        }
    }
    else if (cell->pion == SOLDAT_BLEU || cell->pion == ROI_BLEU)
    {
        if (near_case->pion == SOLDAT_ROUGE && (far_case->pion == SOLDAT_BLEU || far_case->pion == ROI_BLEU))
        {
            clear_case(near_case);
            dead_red_count += 1;
            check_dead_count();
            return;
        }
        else if (near_case->pion == ROI_ROUGE && (far_case->pion == SOLDAT_BLEU || far_case->pion == ROI_BLEU))
        {
            clear_case(near_case);
            endgame(1, 2);
        }
    }
}

/**
 * @see jeu.h
 */
void prise(Case *cell)
{
    bool ack = false; // si au moins 2 cases au dessus de la case jouée
    Case *near_case;
    Case *far_case;

    char lettre = cell->id[0]; // parsing des coordonnées de la case jouée
    char chiffre = cell->id[1];

    void check_up()
    {
        int tmp1 = (chiffre - '0') + 1;
        int tmp2 = (chiffre - '0') + 2;
        if (tmp2 <= 9)
        {
            ack = true;
        }

        if (ack)
        {
            char near[10];
            char far[10];
            sprintf(near, "%c%d", lettre, tmp1);
            sprintf(far, "%c%d", lettre, tmp2);

            near_case = get_case_by_id(near);
            far_case = get_case_by_id(far);
            if (!near_case || !far_case)
            {
                printf("%s\n", "erreur de fonction prise, check haut");
            }

            prise_check(cell, near_case, far_case);
        }
    }
    void check_down()
    {
        int tmp1 = (chiffre - '0') - 1;
        int tmp2 = (chiffre - '0') - 2;
        if (tmp2 >= 1)
        {
            ack = true;
        }

        if (ack)
        {
            char near[10];
            char far[10];
            sprintf(near, "%c%d", lettre, tmp1);
            sprintf(far, "%c%d", lettre, tmp2);

            near_case = get_case_by_id(near);
            far_case = get_case_by_id(far);
            if (!near_case || !far_case)
            {
                printf("%s\n", "erreur de fonction prise, check haut");
            }

            prise_check(cell, near_case, far_case);
        }
    }

    void check_right()
    {
        char tmp1 = lettre + 1;
        char tmp2 = lettre + 2;
        if (tmp2 <= 'I')
        {
            ack = true;
        }

        if (ack)
        {
            char near[10];
            char far[10];
            sprintf(near, "%c%c", tmp1, chiffre);
            sprintf(far, "%c%c", tmp2, chiffre);

            near_case = get_case_by_id(near);
            far_case = get_case_by_id(far);
            if (!near_case || !far_case)
            {
                printf("%s\n", "erreur de fonction prise, check haut");
            }

            prise_check(cell, near_case, far_case);
        }
    }

    void check_left()
    {
        char tmp1 = lettre - 1;
        char tmp2 = lettre - 2;
        if (tmp2 >= 'A')
        {
            ack = true;
        }

        if (ack)
        {
            char near[10];
            char far[10];
            sprintf(near, "%c%c", tmp1, chiffre);
            sprintf(far, "%c%c", tmp2, chiffre);

            near_case = get_case_by_id(near);
            far_case = get_case_by_id(far);
            if (!near_case || !far_case)
            {
                printf("%s\n", "erreur de fonction prise, check haut");
            }

            prise_check(cell, near_case, far_case);
        }
    }

    check_up();
    ack = false;
    check_down();
    ack = false;
    check_right();
    ack = false;
    check_left();
    return;
}

/**
 * @see jeu.h
 */
void on_cell_clicked_local(GtkWidget *button, gpointer user_data)
{
    Case *cell = (Case *)user_data; // retrouver la case sélectionnée

    if (!cell)
    {
        printf("%s\n", "erreur de selection de case");
        return;
    }

    if (!selected) // logique si une première case n'est pas encore selectionnée
    {
        if (tour % 2 != 0)
        {
            if (cell->pion == SOLDAT_BLEU || cell->pion == ROI_BLEU)
            {
                select_case(cell);
            }
        }
        else
        {
            if (cell->pion == SOLDAT_ROUGE || cell->pion == ROI_ROUGE)
            {
                select_case(cell);
            }
        }
        return;
    }
    else if (selected->id == cell->id) // déselection si case cliquée est celle sélectionnée
    {
        unselect_case();
        return;
    }
    else if (cell->pion) // si une autre case contenant un pion est cliquée, changer la sélection
    {
        if (tour % 2 != 0)
        {
            if (cell->pion == SOLDAT_BLEU || cell->pion == ROI_BLEU)
            {
                unselect_case();
                select_case(cell);
            }
        }
        else
        {
            if (cell->pion == SOLDAT_ROUGE || cell->pion == ROI_ROUGE)
            {
                unselect_case();
                select_case(cell);
            }
        }
        return;
    }

    // logique de jeu après avoir sélectionné une case
    const char *label = gtk_button_get_label(GTK_BUTTON(button));
    if (!label) // si la case sélectionnée n'a pas de label, alors skip
    {
        return;
    }

    if (strcmp(label, "•") != 0) // regarder si la case jouée est une case jouable, sinon skip
    {
        return;
    }

    // mouvement de pion sur une case jouable
    // enregistrement des informations de la case sélectionnée avans déselection
    int selected_color = selected->couleur;
    int selected_pion = selected->pion;

    const char *str = gtk_button_get_label(GTK_BUTTON(selected->button));
    char *selected_button_label = strdup(str); // copie de la valeur au lieu du pointeur

    char *mouvement;
    if (gtk_widget_has_css_class(button, "haut"))
    {
        mouvement = "haut";
    }
    else if (gtk_widget_has_css_class(button, "bas"))
    {
        mouvement = "bas";
    }
    else if (gtk_widget_has_css_class(button, "droite"))
    {
        mouvement = "droite";
    }
    else if (gtk_widget_has_css_class(button, "gauche"))
    {
        mouvement = "gauche";
    }

    // clear la case sélectionnée en laissant la couleur
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

    // mettre à jour la case jouée
    set_cell_color(cell, selected_color);
    cell->pion = selected_pion;
    gtk_button_set_label(GTK_BUTTON(cell->button), selected_button_label);
    g_free(selected_button_label);

    // application des règles

    capture(cell, mouvement);
    prise(cell);

    Case *verif_ville_bleu = get_case_by_id("A9");
    if (!verif_ville_bleu)
    {
        printf("%s\n", "case founie incorrect, erreur de vérification ville rouge");
    }
    if (verif_ville_bleu->pion == ROI_ROUGE)
    {
        endgame(1, 1);
        return;
    }

    Case *verif_ville_rouge = get_case_by_id("I1");
    if (!verif_ville_rouge)
    {
        printf("%s\n", "case founie incorrect, erreur de vérification ville bleu");
    }
    if (verif_ville_rouge->pion == ROI_BLEU)
    {
        endgame(1, 2);
        return;
    }

    // incrémenter le compteur de tour
    tour += 1;
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%d", tour);
    char result[120] = "Tour : ";
    strcat(result, buffer);
    gtk_label_set_text(GTK_LABEL(config.tour_label), result);

    if (tour == 65)
    {
        endgame(0, 0);
        return;
    }

    const char *texte = gtk_label_get_text(GTK_LABEL(config.couleur_label));
    if (tour % 2 != 0 && (strcmp(texte, "Tour des rouges") == 0 || strcmp(texte, "Tour des bleus") == 0))
    {
        gtk_label_set_text(GTK_LABEL(config.couleur_label), "Tour des bleus");
    }
    else if (strcmp(texte, "Tour des rouges") == 0 || strcmp(texte, "Tour des bleus") == 0)
    {
        gtk_label_set_text(GTK_LABEL(config.couleur_label), "Tour des rouges");
    }

    return;
}

/**
 * @see jeu.h
 */
void on_cell_clicked_tcp(GtkWidget *button, gpointer user_data)
{
    Case *cell = (Case *)user_data; // retrouver la case sélectionnée

    if (!cell)
    {
        printf("%s\n", "erreur de selection de case");
        return;
    }

    if (!selected) // logique si une première case n'est pas encore selectionnée
    {
        if (tour % 2 != 0 && config.mode == CLIENT)
        {
            if (cell->pion == SOLDAT_BLEU || cell->pion == ROI_BLEU)
            {
                select_case(cell);
            }
        }
        else if (tour % 2 == 0 && config.mode == SERVER)
        {
            if (cell->pion == SOLDAT_ROUGE || cell->pion == ROI_ROUGE)
            {
                select_case(cell);
            }
        }
        return;
    }
    else if (selected->id == cell->id) // déselection si case cliquée est celle sélectionnée
    {
        unselect_case();
        return;
    }
    else if (cell->pion) // si une autre case contenant un pion est cliquée, changer la sélection
    {
        if (tour % 2 != 0)
        {
            if (cell->pion == SOLDAT_BLEU || cell->pion == ROI_BLEU)
            {
                unselect_case();
                select_case(cell);
            }
        }
        else
        {
            if (cell->pion == SOLDAT_ROUGE || cell->pion == ROI_ROUGE)
            {
                unselect_case();
                select_case(cell);
            }
        }
        return;
    }

    // logique de jeu après avoir sélectionné une case
    const char *label = gtk_button_get_label(GTK_BUTTON(button));
    if (!label) // si la case sélectionnée n'a pas de label, alors skip
    {
        return;
    }

    if (strcmp(label, "•") != 0) // regarder si la case jouée est une case jouable, sinon skip
    {
        return;
    }

    // mouvement de pion sur une case jouable
    // enregistrement des informations de la case sélectionnée avans déselection
    int selected_color = selected->couleur;
    int selected_pion = selected->pion;
    char *selected_id = selected->id;

    const char *str = gtk_button_get_label(GTK_BUTTON(selected->button));
    char *selected_button_label = strdup(str); // copie de la valeur au lieu du pointeur

    char *mouvement;
    if (gtk_widget_has_css_class(button, "haut"))
    {
        mouvement = "haut";
    }
    else if (gtk_widget_has_css_class(button, "bas"))
    {
        mouvement = "bas";
    }
    else if (gtk_widget_has_css_class(button, "droite"))
    {
        mouvement = "droite";
    }
    else if (gtk_widget_has_css_class(button, "gauche"))
    {
        mouvement = "gauche";
    }

    // clear la case sélectionnée en laissant la couleur
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

    // mettre à jour la case jouée
    set_cell_color(cell, selected_color);
    cell->pion = selected_pion;
    gtk_button_set_label(GTK_BUTTON(cell->button), selected_button_label);
    g_free(selected_button_label);
    send_move_to_network(selected_id, cell->id);

    // application des règles

    capture(cell, mouvement);
    prise(cell);

    Case *verif_ville_bleu = get_case_by_id("A9");
    if (!verif_ville_bleu)
    {
        printf("%s\n", "case founie incorrect, erreur de vérification ville rouge");
    }
    if (verif_ville_bleu->pion == ROI_ROUGE)
    {
        endgame(1, 1);
        return;
    }

    Case *verif_ville_rouge = get_case_by_id("I1");
    if (!verif_ville_rouge)
    {
        printf("%s\n", "case founie incorrect, erreur de vérification ville bleu");
    }
    if (verif_ville_rouge->pion == ROI_BLEU)
    {
        endgame(1, 2);
        return;
    }

    // incrémenter le compteur de tour
    tour += 1;
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%d", tour);
    char result[120] = "Tour : ";
    strcat(result, buffer);
    gtk_label_set_text(GTK_LABEL(config.tour_label), result);

    if (tour == 65)
    {
        endgame(0, 0);
        return;
    }

    const char *texte = gtk_label_get_text(GTK_LABEL(config.couleur_label));
    if (tour % 2 != 0 && (strcmp(texte, "Tour des rouges (serveur)") == 0 || strcmp(texte, "Tour des bleus (client)") == 0))
    {
        gtk_label_set_text(GTK_LABEL(config.couleur_label), "Tour des bleus (client)");
    }
    else if (strcmp(texte, "Tour des rouges (serveur)") == 0 || strcmp(texte, "Tour des bleus (client)") == 0)
    {
        gtk_label_set_text(GTK_LABEL(config.couleur_label), "Tour des rouges (serveur)");
    }

    return;
}
