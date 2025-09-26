/**
 * @file test_ia.c
 * @brief Suite de tests unitaires complète pour l'Intelligence Artificielle de Krojanty.
 * @authors Groupe 8
 *
 * Cette suite de tests vérifie les capacités de décision fondamentales de l'IA :
 * - Reconnaître et jouer un coup gagnant.
 * - Identifier et parer une menace de victoire adverse.
 * - Privilégier les captures avantageuses.
 * - Améliorer sa position en l'absence de menace immédiate.
 * - Gérer les cas invalides et complexes pour une couverture de code complète.
 *
 * @note Ces tests sont basés sur la logique interne de l'IA (ia.c),
 * où la condition de victoire par conquête est :
 * - Le Roi Bleu atteint la case de coordonnées (8, 8) -> I1.
 * - Le Roi Rouge atteint la case de coordonnées (0, 0) -> A9.
 */

#include <stdio.h>
#include <assert.h>
#include "ia.h"


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
    printf("Test IA: %-60s...", test_name);
    stats->test_count++;
    if (test_func()) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        stats->failures++;
    }
}

// Déclarations des fonctions de test 
int test_ia_choisit_coup_gagnant_direct();
int test_ia_bloque_menace_de_victoire_imminente();
int test_ia_choisit_capture_avantageuse();
int test_ia_avance_roi_vers_objectif();
int test_ia_piece_index_cas_invalide();
int test_ia_genere_tt_exact_flag();


// SUITE DE TESTS POUR L'IA 

/**
 * @brief Test 1: Vérifie si l'IA joue un coup gagnant immédiat.
 *
 * @b Arrange: Configure le plateau pour que le Roi Bleu (en H1 / 8,7) puisse
 * gagner en se déplaçant sur la case de victoire I1 (8,8) qui est libre.
 * @b Act: Demande à l'IA de trouver le meilleur coup pour le joueur Bleu.
 * @b Assert: L'IA doit impérativement choisir le coup qui déplace le roi
 * sur la case de victoire.
 * @return 1 en cas de succès, 0 en cas d'échec.
 */
int test_ia_choisit_coup_gagnant_direct() {
    // Arrange: Le plateau est configuré pour que le Roi Bleu puisse gagner en un seul coup.
    // Le Roi Bleu est en H1 (8,7) et la case de victoire I1 (8,8) est libre.
    Board b = {0};
    b.pion[8][7] = ROI_BLEU;
    b.pion[0][1] = ROI_ROUGE;

    // Ajout de soldats pour éviter une fausse victoire par "extermination".
    b.pion[0][0] = SOLDAT_BLEU;
    b.pion[1][0] = SOLDAT_ROUGE;

    // Act: Demande à l'IA de trouver le meilleur coup pour le joueur Bleu.
    ia_init_once();
    Move best_move = search_best_move(&b, true);

    // Assert: L'IA doit impérativement choisir le coup qui déplace le roi sur la case de victoire.
    int move_is_correct = (best_move.r1 == 8 && best_move.c1 == 7 && best_move.r2 == 8 && best_move.c2 == 8);
    assert(move_is_correct);
    
    return move_is_correct;
}

/**
 * @brief Test 2: Vérifie si l'IA bloque une menace de victoire imminente.
 *
 * @b Arrange: Le Roi Bleu (8,7) menace de gagner au prochain tour. Le seul
 * coup utile pour le joueur Rouge est de déplacer son soldat de (7,8) à (8,8)
 * pour bloquer la case de victoire.
 * @b Act: Demande à l'IA de trouver le meilleur coup pour le joueur Rouge.
 * @b Assert: L'IA doit choisir le coup de blocage, qui est le seul à
 * empêcher la défaite.
 * @return 1 en cas de succès, 0 en cas d'échec.
 */
int test_ia_bloque_menace_de_victoire_imminente() {
    // Arrange: Le Roi Bleu en H1 (8,7) menace de gagner au prochain tour en allant en I1 (8,8).
    // Le seul coup utile pour le joueur Rouge est de déplacer son soldat de I2 (7,8) à I1 pour bloquer.
    Board b = {0};
    b.pion[8][7] = ROI_BLEU;      // Roi Bleu menaçant
    b.pion[7][8] = SOLDAT_ROUGE;  // Soldat Rouge qui doit bloquer
    
    // Le Roi Rouge est déplacé de sa case de victoire (0,0) vers une case neutre (7,7).
    b.pion[7][7] = ROI_ROUGE;     // Roi Rouge sur une case non gagnante

    b.pion[2][2] = SOLDAT_BLEU;   // Présence d'un soldat bleu requise

    // Piège pour l'autre roi afin de s'assurer que seul le soldat est pertinent pour la défense.
    b.pion[6][7] = SOLDAT_BLEU;
    b.pion[7][6] = SOLDAT_BLEU;

    // Act: Demande à l'IA de trouver le meilleur coup pour le joueur Rouge.
    ia_init_once();
    Move best_move = search_best_move(&b, false);

    // Assert: L'IA doit choisir le coup de blocage, qui est le seul à empêcher la défaite.
    int move_is_correct = (best_move.r1 == 7 && best_move.c1 == 8 && best_move.r2 == 8 && best_move.c2 == 8);
    assert(move_is_correct);
    
    return move_is_correct;
}

/**
 * @brief Test 3: Vérifie si l'IA privilégie une capture.
 *
 * @b Arrange: Un soldat rouge peut capturer un soldat bleu. Tous les autres
 * mouvements possibles sont bloqués pour forcer l'IA à choisir la capture.
 * @b Act: Demande à l'IA de trouver le meilleur coup pour le joueur Rouge.
 * @b Assert: Le seul coup possible doit être le déplacement qui
 * déclenche la capture.
 * @return 1 en cas de succès, 0 en cas d'échec.
 */
int test_ia_choisit_capture_avantageuse() {
    // Arrange: Un soldat rouge peut capturer un soldat bleu.
    // Le plateau est configuré pour que ce soit le SEUL coup possible pour le rouge,
    // forçant ainsi l'IA à choisir la capture.
    Board b = {0};

    // 1. La situation de capture que nous voulons tester
    b.pion[5][1] = SOLDAT_ROUGE;  // Soldat Rouge attaquant
    b.pion[5][3] = SOLDAT_BLEU;   // Victime bleue
    b.pion[5][4] = EMPTY;         // La case derrière la victime doit être vide pour la capture "Seultou"

    // 2. Les Rois, placés de manière à ne pas interférer et à ne pas avoir de coups.
    b.pion[7][7] = ROI_ROUGE;     
    b.pion[1][1] = ROI_BLEU;
    
    // Piéger le Roi Rouge
    b.pion[6][7] = SOLDAT_BLEU;
    b.pion[8][7] = SOLDAT_BLEU;
    b.pion[7][6] = SOLDAT_BLEU;
    b.pion[7][8] = SOLDAT_BLEU;
    
    // Forcer la capture en bloquant tous les autres mouvements possibles du soldat rouge.
    b.pion[4][1] = SOLDAT_BLEU;   // Bloque le mouvement vers le haut
    b.pion[6][1] = SOLDAT_BLEU;   // Bloque le mouvement vers le bas
    b.pion[5][0] = SOLDAT_BLEU;   // Bloque le mouvement vers la gauche

    // La seule case libre adjacente est (5,2), qui mène à la capture.

    // Act: Demande à l'IA de trouver le meilleur coup pour le joueur Rouge.
    ia_init_once();
    Move best_move = search_best_move(&b, false);

    // Assert: Le meilleur (et seul) coup doit être le déplacement en (5,2) qui déclenche la capture.
    int move_is_correct = (best_move.r1 == 5 && best_move.c1 == 1 && best_move.r2 == 5 && best_move.c2 == 2);
    assert(move_is_correct);

    return move_is_correct;
}

/**
 * @brief Test 4: Vérifie si l'IA améliore la position de son roi en situation calme.
 *
 * @b Arrange: La partie est dans une situation neutre, sans menace ou
 * opportunité de capture immédiate.
 * @b Act: L'IA cherche le meilleur coup pour le joueur Bleu.
 * @b Assert: Le coup choisi doit être un mouvement du roi qui réduit la
 * distance à la ville adverse (8,8).
 * @return 1 en cas de succès, 0 en cas d'échec.
 */
int test_ia_avance_roi_vers_objectif() {
    // Arrange: La partie est dans une situation calme.
    // L'un des meilleurs coups pour Bleu est de rapprocher son roi de la ville rouge (I1 / 8,8).
    Board b = {0};
    b.pion[1][1] = ROI_BLEU;
    b.pion[7][7] = ROI_ROUGE;
    // On garde des soldats sur le plateau pour s'assurer que la partie est valide,
    // mais on ne bloque pas les mouvements progressifs du roi bleu.
    b.pion[0][5] = SOLDAT_BLEU;
    b.pion[6][7] = SOLDAT_ROUGE;
    b.pion[7][6] = SOLDAT_ROUGE;

    // Act: L'IA cherche le meilleur coup pour Bleu.
    ia_init_once();
    Move best_move = search_best_move(&b, true);

    // Assert: Le coup choisi doit être un mouvement du roi qui réduit la distance à la cible (8,8).
    // Un "bon" coup (rapprochement) est celui où r2 > r1 ou c2 > c1.
    int is_king_move = (best_move.r1 == 1 && best_move.c1 == 1);
    int is_progressive_move = (best_move.r2 > best_move.r1 || best_move.c2 > best_move.c1);
    
    assert(is_king_move && is_progressive_move);

    return is_king_move && is_progressive_move;
}




/**
 * @brief Test 6: Met en place un scénario pour déclencher le flag TT_EXACT.
 *
 * Ce test vise à améliorer la couverture de code en déclenchant des scénarios
 * de recherche où une coupure alpha-bêta n'a pas lieu, couvrant ainsi
 * la branche `else flag = TT_EXACT;` dans la fonction minimax.
 * @return 1 si un coup valide est trouvé, 0 sinon.
 */
int test_ia_genere_tt_exact_flag() {
    // Arrange: vise principalement à améliorer la couverture de code
    // en déclenchant des scénarios de recherche où une coupure alpha-bêta n'a pas lieu,
    // ce qui couvre la branche `else flag = TT_EXACT;`.
    Board b = {0};
    b.pion[4][4] = ROI_BLEU;
    b.pion[0][0] = SOLDAT_BLEU;
    b.pion[8][8] = ROI_ROUGE;
    b.pion[7][7] = SOLDAT_ROUGE;
    
    // Act: On lance une recherche. Le scénario est conçu pour que la recherche
    // explore plusieurs branches sans trouver de coupure immédiate.
    ia_init_once();
    Move m = search_best_move(&b, true);
    
    // Assert: L'assertion principale est que l'IA trouve un coup valide.
    // La vérification de la couverture de la ligne se fera par l'outil `lcov`.
    assert(m.r1 != -1);

    return m.r1 != -1;
}


/**
 * @brief Point d'entrée principal pour l'exécutable de test de l'IA.
 *
 * Lance l'ensemble des tests définis dans ce fichier et affiche un résumé
 * des résultats (succès ou échec).
 * @return Le nombre de tests échoués (0 pour un succès complet).
 */
int main() {
    TestStats stats = {0, 0};
    
    printf("--- Lancement des tests pour l'Intelligence Artificielle ---\n");
    
    run_test(test_ia_choisit_coup_gagnant_direct, "Doit choisir un coup gagnant immédiat", &stats);
    run_test(test_ia_bloque_menace_de_victoire_imminente, "Doit bloquer une victoire imminente de l'adversaire", &stats);
    run_test(test_ia_choisit_capture_avantageuse, "Doit préférer une capture à un coup neutre", &stats);
    run_test(test_ia_avance_roi_vers_objectif, "Doit avancer son roi en situation neutre", &stats);
    run_test(test_ia_genere_tt_exact_flag, "Doit déclencher la recherche complète (couverture)", &stats);
    
    printf("--- Résumé des tests IA ---\n");
    if (stats.failures == 0) {
        printf("SUCCÈS : %d/%d tests passés.\n", stats.test_count, stats.test_count);
    } else {
        printf("ÉCHEC : %d tests échoués sur %d.\n", stats.failures, stats.test_count);
    }

    return stats.failures; 
}