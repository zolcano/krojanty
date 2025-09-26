/**
 * @file test_jeu.c
 * @brief Suite de tests unitaires complète pour le moteur de jeu Krojanty.
 * @authors Groupe 8
 *
 * Ce fichier contient une série de tests pour valider chaque règle implémentée
 * dans jeu_logique.c. Il est compilé séparément pour créer un exécutable de test
 * et s'assure que la logique du jeu est correcte, indépendamment de l'interface
 * graphique ou de l'IA.
 */

#include <stdio.h>
#include <assert.h>
#include "jeu_logique.h" // dépendance logique pure.

// Structure et utilitaire pour l'exécution des tests

/**
 * @struct TestStats
 * @brief Structure simple pour suivre les statistiques d'une suite de tests.
 */
typedef struct {
    int failures;   /**< Nombre de tests échoués. */
    int test_count; /**< Nombre total de tests exécutés. */
} TestStats;


/**
 * @brief Exécute une fonction de test, affiche le résultat et met à jour les stats.
 * @param test_func Pointeur vers la fonction de test à exécuter.
 * @param test_name Nom descriptif du test à afficher.
 * @param stats Pointeur vers la structure TestStats à mettre à jour.
 */
void run_test(int (*test_func)(), const char* test_name, TestStats* stats) {
    printf("Test: %-55s...", test_name);
    stats->test_count++;
    if (test_func()) {
        printf("PASS\n");
    } else {
        printf("FAIL\n"); 
        stats->failures++;
    }
}

// SUITE DE TESTS 

//  Tests d'Initialisation 
/** @brief Teste l'initialisation correcte du plateau de jeu. */
int test_init_plateau() {
    GameState state;
    logique_init_game(&state);
    assert(state.pion[1][1] == ROI_BLEU);
    assert(state.pion[7][7] == ROI_ROUGE);
    assert(state.tour == 1);
    assert(state.dead_blue_count == 0);
    assert(state.dead_red_count == 0);
    assert(state.game_over_status == 0);
    return 1;
}

/** @brief Teste une capture par poussée ("Seultou"). */
int test_capture_poussee_reussit() {
    GameState state;
    logique_init_game(&state);
    state.pion[5][2] = SOLDAT_BLEU;
    state.pion[5][3] = SOLDAT_ROUGE;
    logique_capture(&state, 5, 2, "droite");
    assert(state.pion[5][3] == EMPTY);
    assert(state.dead_red_count == 1);
    return 1;
}

/** @brief Teste l'échec d'une capture par poussée si la victime est protégée. */
int test_capture_poussee_echoue_si_garde() {
    GameState state;
    logique_init_game(&state);
    state.pion[5][2] = SOLDAT_BLEU;
    state.pion[5][3] = SOLDAT_ROUGE;
    state.pion[5][4] = SOLDAT_ROUGE;
    logique_capture(&state, 5, 2, "droite");
    assert(state.pion[5][3] == SOLDAT_ROUGE);
    assert(state.dead_red_count == 0);
    return 1;
}

/** @brief Teste une capture par poussée réussie contre le bord du plateau. */
int test_capture_poussee_reussit_contre_bord() {
    GameState state;
    logique_init_game(&state);
    state.pion[0][7] = SOLDAT_ROUGE;
    state.pion[0][8] = SOLDAT_BLEU;
    logique_capture(&state, 0, 7, "droite");
    assert(state.pion[0][8] == EMPTY);
    assert(state.dead_blue_count == 1);
    return 1;
}

/** @brief Teste l'échec d'une capture par poussée sur une pièce alliée. */
int test_capture_poussee_echoue_sur_allie() {
    GameState state;
    logique_init_game(&state);
    state.pion[5][2] = SOLDAT_BLEU;
    state.pion[5][3] = SOLDAT_BLEU;
    logique_capture(&state, 5, 2, "droite");
    assert(state.pion[5][3] == SOLDAT_BLEU);
    assert(state.dead_blue_count == 0);
    return 1;
}

/** @brief Teste qu'une capture par poussée sur une case vide n'a aucun effet. */
int test_capture_poussee_sur_case_vide_ne_fait_rien() {
    GameState state;
    logique_init_game(&state);
    state.pion[5][2] = SOLDAT_BLEU;
    state.pion[5][3] = EMPTY;
    logique_capture(&state, 5, 2, "droite");
    assert(state.pion[5][3] == EMPTY);
    assert(state.dead_red_count == 0);
    return 1;
}

/** @brief Teste si la capture d'un roi termine bien la partie. */
int test_capture_poussee_roi_termine_partie() {
    GameState state;
    logique_init_game(&state);
    state.pion[5][2] = SOLDAT_BLEU;
    state.pion[5][3] = ROI_ROUGE;
    logique_capture(&state, 5, 2, "droite");
    assert(state.pion[5][3] == EMPTY);
    assert(state.game_over_status == 2); // Victoire bleue
    return 1;
}

/** @brief Teste une prise en sandwich ("Linca") horizontale. */
int test_prise_sandwich_reussit_horizontal() {
    GameState state;
    logique_init_game(&state);
    state.pion[4][1] = SOLDAT_ROUGE;
    state.pion[4][2] = SOLDAT_BLEU;
    state.pion[4][3] = SOLDAT_ROUGE;
    logique_prise(&state, 4, 3);
    assert(state.pion[4][2] == EMPTY);
    assert(state.dead_blue_count == 1);
    return 1;
}

/** @brief Teste une prise en sandwich ("Linca") verticale. */
int test_prise_sandwich_reussit_vertical() {
    GameState state;
    logique_init_game(&state);
    state.pion[6][2] = SOLDAT_BLEU;
    state.pion[5][2] = SOLDAT_ROUGE;
    state.pion[4][2] = SOLDAT_BLEU;
    logique_prise(&state, 4, 2);
    assert(state.pion[5][2] == EMPTY);
    assert(state.dead_red_count == 1);
    return 1;
}

/** @brief Teste l'échec d'une prise en sandwich si la pièce "alliée" ne l'est pas. */
int test_prise_sandwich_echoue_si_pas_allie() {
    GameState state;
    logique_init_game(&state);
    state.pion[4][1] = SOLDAT_BLEU;
    state.pion[4][2] = SOLDAT_BLEU;
    state.pion[4][3] = SOLDAT_ROUGE;
    logique_prise(&state, 4, 3);
    assert(state.pion[4][2] == SOLDAT_BLEU);
    assert(state.dead_blue_count == 0);
    return 1;
}

/** @brief Teste si une prise en sandwich sur un roi termine la partie. */
int test_prise_sandwich_capture_roi_termine_partie() {
    GameState state;
    logique_init_game(&state);
    state.pion[4][1] = SOLDAT_ROUGE;
    state.pion[4][2] = ROI_BLEU;
    state.pion[4][3] = SOLDAT_ROUGE;
    logique_prise(&state, 4, 3);
    assert(state.pion[4][2] == EMPTY);
    assert(state.game_over_status == 1);
    return 1;
}

/** @brief Teste l'échec d'une prise en sandwich près du bord du plateau. */
int test_prise_echoue_pres_du_bord() {
    GameState state;
    logique_init_game(&state);
    state.pion[7][0] = SOLDAT_ROUGE;
    state.pion[8][0] = SOLDAT_BLEU;
    logique_prise(&state, 8, 0);
    assert(state.pion[7][0] == SOLDAT_ROUGE);
    assert(state.dead_red_count == 0);
    return 1;
}


/** @brief Teste la condition de victoire par conquête pour le joueur rouge. */
int test_victoire_conquete_rouge() {
    GameState state;
    logique_init_game(&state);
    state.pion[0][0] = ROI_ROUGE;
    logique_verifier_conditions_fin(&state);
    assert(state.game_over_status == 1);
    return 1;
}

/** @brief Teste la condition de victoire par conquête pour le joueur bleu. */
int test_victoire_conquete_bleu() {
    GameState state;
    logique_init_game(&state);
    // CORRIGÉ : La case de victoire bleue est I1 -> (8,8)
    state.pion[8][8] = ROI_BLEU;
    logique_verifier_conditions_fin(&state);
    assert(state.game_over_status == 2);
    return 1;
}

/** @brief Teste la condition de victoire par extermination pour le joueur bleu. */
int test_victoire_extermination_bleu() {
    GameState state;
    logique_init_game(&state);
    state.dead_red_count = 8;
    logique_verifier_conditions_fin(&state);
    assert(state.game_over_status == 2);
    return 1;
}

/** @brief Teste la condition de victoire par extermination pour le joueur rouge. */
int test_victoire_extermination_rouge() {
    GameState state;
    logique_init_game(&state);
    state.dead_blue_count = 8;
    logique_verifier_conditions_fin(&state);
    assert(state.game_over_status == 1);
    return 1;
}

/** @brief Teste la fin de partie par décompte de points avec victoire rouge. */
int test_fin_par_score_rouge_gagne() {
    GameState state;
    logique_init_game(&state);
    state.tour = 65;
    state.couleur[5][5] = 1; // Donne 1 point de plus au rouge
    logique_verifier_conditions_fin(&state);
    assert(state.game_over_status == 1);
    return 1;
}

/** @brief Teste la fin de partie par décompte de points avec victoire bleue. */
int test_fin_par_score_bleu_gagne() {
    GameState state;
    logique_init_game(&state);
    state.tour = 65;
    state.couleur[5][5] = 2; // Donne 1 point de plus au bleu
    logique_verifier_conditions_fin(&state);
    assert(state.game_over_status == 2);
    return 1;
}

/** @brief Teste la fin de partie par décompte de points avec une égalité. */
int test_fin_par_score_egalite() {
    GameState state;
    logique_init_game(&state);
    state.tour = 65;
    logique_verifier_conditions_fin(&state);
    assert(state.game_over_status == 3);
    return 1;
}

/** @brief Teste qu'aucune action n'est possible si la partie est déjà terminée. */
int test_actions_impossibles_si_partie_finie() {
    GameState state;
    logique_init_game(&state);
    state.game_over_status = 1;
    state.pion[4][1] = SOLDAT_ROUGE;
    state.pion[4][2] = SOLDAT_BLEU;
    state.pion[4][3] = SOLDAT_ROUGE;
    logique_prise(&state, 4, 3);
    assert(state.pion[4][2] == SOLDAT_BLEU);
    assert(state.dead_blue_count == 0);
    return 1;
}

/** @brief Teste l'échec d'une prise depuis une case vide. */
int test_prise_echoue_si_attaquant_vide() {
    GameState state;
    logique_init_game(&state);
    state.pion[4][1] = SOLDAT_ROUGE;
    state.pion[4][2] = SOLDAT_BLEU;
    logique_prise(&state, 4, 3); // Case (4,3) est vide
    assert(state.pion[4][2] == SOLDAT_BLEU);
    assert(state.dead_blue_count == 0);
    return 1;
}


/**
 * @brief Point d'entrée principal pour l'exécutable de test de la logique du jeu.
 *
 * Lance l'ensemble des tests de règles définis dans ce fichier et affiche un
 * résumé des résultats.
 * @return Le nombre de tests échoués (0 pour un succès complet).
 */
int main() {
    TestStats stats = {0, 0};
    
    printf("--- Lancement des tests pour la logique de jeu de Krojanty ---\n");
    
    // Tests de base
    run_test(test_init_plateau, "Initialisation : Plateau correct", &stats);

    // Tests de capture (Seultou)
    run_test(test_capture_poussee_reussit, "Capture (Seultou) : Cas nominal", &stats);
    run_test(test_capture_poussee_echoue_si_garde, "Capture (Seultou) : Échoue si protégé", &stats);
    run_test(test_capture_poussee_reussit_contre_bord, "Capture (Seultou) : Réussit contre le bord", &stats);
    run_test(test_capture_poussee_echoue_sur_allie, "Capture (Seultou) : Échoue sur un allié", &stats);
    run_test(test_capture_poussee_sur_case_vide_ne_fait_rien, "Capture (Seultou) : Cible une case vide", &stats);
    run_test(test_capture_poussee_roi_termine_partie, "Capture (Seultou) : Capture d'un roi", &stats);

    // Tests de prise (Linca)
    run_test(test_prise_sandwich_reussit_horizontal, "Prise (Linca) : Cas nominal horizontal", &stats);
    run_test(test_prise_sandwich_reussit_vertical, "Prise (Linca) : Cas nominal vertical", &stats);
    run_test(test_prise_sandwich_echoue_si_pas_allie, "Prise (Linca) : Échoue si pas un allié", &stats);
    run_test(test_prise_echoue_pres_du_bord, "Prise (Linca) : Échoue près du bord", &stats);
    run_test(test_prise_sandwich_capture_roi_termine_partie, "Prise (Linca) : Capture d'un roi", &stats);

    // Tests des conditions de fin
    run_test(test_victoire_conquete_rouge, "Condition de fin : Conquête (Rouge)", &stats);
    run_test(test_victoire_conquete_bleu, "Condition de fin : Conquête (Bleu)", &stats);
    run_test(test_victoire_extermination_rouge, "Condition de fin : Extermination (Rouge)", &stats);
    run_test(test_victoire_extermination_bleu, "Condition de fin : Extermination (Bleu)", &stats);
    run_test(test_fin_par_score_rouge_gagne, "Condition de fin : Limite de tours (Rouge gagne)", &stats);
    run_test(test_fin_par_score_bleu_gagne, "Condition de fin : Limite de tours (Bleu gagne)", &stats);
    run_test(test_fin_par_score_egalite, "Condition de fin : Limite de tours (Égalité)", &stats);

    // Tests des cas limites
    run_test(test_actions_impossibles_si_partie_finie, "Cas Limites : Action impossible si partie finie", &stats);
    run_test(test_prise_echoue_si_attaquant_vide, "Cas Limites : Prise impossible depuis une case vide", &stats);

    printf("--- Résumé des tests ---\n");
    if (stats.failures == 0) {
        printf("SUCCÈS : %d/%d tests passés.\n", stats.test_count, stats.test_count);
    } else {
        printf("ÉCHEC : %d tests échoués sur %d.\n", stats.failures, stats.test_count);
    }

    return stats.failures; 
}