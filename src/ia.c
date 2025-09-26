/**
 * @file ia.c
 * @brief Implémentation du cœur de l'intelligence artificielle pour Krojanty.
 * @authors Groupe 8
 *
 * ia.c contient la logique de l'IA, incluant l'algorithme Minimax avec
 * élagage Alpha-Bêta, la gestion des tables de transposition avec hachage de
 * Zobrist, l'évaluation de position, et l'ordonnancement des coups. Il est
 * conçu pour être indépendant de l'interface graphique.
 */

#include "ia.h"
#include "jeu.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// Variables globales (définition)

TTEntry *TT = NULL;               /**< La table de transposition. */
uint64_t Zobrist[5][SIZE][SIZE];  /**< Tableaux pour le hachage de Zobrist. */
uint64_t Z_SIDE;                  /**< Clé Zobrist pour le côté qui doit jouer. */

Move g_last_best_move_blue = {0}; /**< Mémorise le dernier meilleur coup pour les bleus. */
Move g_last_best_move_red  = {0}; /**< Mémorise le dernier meilleur coup pour les rouges. */

// Helpers internes
/**
 * @brief Vérifie si les coordonnées sont dans les limites du plateau.
 * @param r Ligne.
 * @param c Colonne.
 * @return true si les coordonnées sont valides, false sinon.
 */
static inline bool in_bounds(int r,int c){return (r>=0&&r<SIZE&&c>=0&&c<SIZE);}

/**
 * @brief Vérifie si une case est vide.
 * @param p Type de pion.
 * @return true si la case est vide, false sinon.
 */
static inline bool is_empty(int p){return p==EMPTY;}

/**
 * @brief Vérifie si un pion est bleu.
 * @param p Type de pion.
 * @return true si le pion est bleu, false sinon.
 */
static inline bool is_blue (int p){return p==SOLDAT_BLEU || p==ROI_BLEU;}

/**
 * @brief Vérifie si un pion est rouge.
 * @param p Type de pion.
 * @return true si le pion est rouge, false sinon.
 */
static inline bool is_red  (int p){return p==SOLDAT_ROUGE|| p==ROI_ROUGE;}

/**
 * @brief Vérifie si un pion est un allié.
 * @param p Type de pion.
 * @param blueSide true si le joueur actuel est bleu.
 * @return true si le pion est allié, false sinon.
 */
static inline bool ally (int p,bool blueSide){return blueSide ? is_blue(p) : is_red(p);}

/**
 * @brief Vérifie si un pion est un ennemi.
 * @param p Type de pion.
 * @param blueSide true si le joueur actuel est bleu.
 * @return true si le pion est ennemi, false sinon.
 */
static inline bool enemy(int p,bool blueSide){return blueSide ? is_red(p)  : is_blue(p);}

/**
 * @brief Retourne un index unique pour chaque type de pièce.
 * @param p Le type de pion (ex: SOLDAT_BLEU).
 * @return Un entier représentant l'index de la pièce.
 */
int piece_index(int p) {
    switch (p) {
        case EMPTY:         return 0;
        case SOLDAT_ROUGE:  return 1;
        case SOLDAT_BLEU:   return 2;
        case ROI_ROUGE:     return 3;
        case ROI_BLEU:      return 4;
        default:            return 0;
    }
}



// Zobrist 

/**
 * @brief Générateur de nombres pseudo-aléatoires 64-bit pour l'initialisation de Zobrist.
 * @param x Pointeur vers la graine (seed).
 * @return Un nombre pseudo-aléatoire de 64 bits.
 */
static uint64_t splitmix64(uint64_t *x) {
    uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

/**
 * @brief Initialise les tableaux de hachage de Zobrist.
 */
static void zobrist_init(void) {
    uint64_t seed = 0xC0FFEEULL ^ 0x123456789ULL;
    for (int p=0;p<5;p++)
        for (int r=0;r<SIZE;r++)
            for (int c=0;c<SIZE;c++) {
                Zobrist[p][r][c] = splitmix64(&seed);
            }
    Z_SIDE = splitmix64(&seed);
}

/**
 * @brief Calcule la clé de hachage de Zobrist pour une position donnée.
 * @param b Le plateau de jeu.
 * @param blueToPlay true si c'est au tour des bleus.
 * @return La clé de hachage de 64 bits.
 */
static uint64_t zobrist_hash(const Board *b, bool blueToPlay) {
    uint64_t h = 0;
    for (int r=0;r<SIZE;r++)
        for (int c=0;c<SIZE;c++) {
            int pi = piece_index(b->pion[r][c]);
            if (pi) h ^= Zobrist[pi][r][c];
        }
    if (blueToPlay) h ^= Z_SIDE;
    return h;
}

// Génération / Simulation  

static const int DR4[4] = {-1, 1, 0, 0}; /**< Déplacements de ligne pour N, S, O, E. */
static const int DC4[4] = { 0, 0,-1, 1}; /**< Déplacements de colonne pour N, S, O, E. */

/**
 * @brief Génère tous les coups possibles pour un camp donné.
 * @param b Le plateau de jeu.
 * @param blueSide true si on génère les coups pour les bleus.
 * @param out Tableau pour stocker les coups générés.
 * @param maxOut Taille maximale du tableau de sortie.
 * @return Le nombre de coups générés.
 */
static int generate_moves(const Board* b, bool blueSide, Move out[], int maxOut) {
    int n = 0;
    for (int r=0; r<SIZE; r++) {
        for (int c=0; c<SIZE; c++) {
            int p = b->pion[r][c];
            if (!ally(p, blueSide)) continue;

            for (int d=0; d<4; d++) {
                int nr = r + DR4[d];
                int nc = c + DC4[d];
                while (in_bounds(nr,nc) && is_empty(b->pion[nr][nc])) {
                    if (n < maxOut) {
                        out[n].r1 = r; out[n].c1 = c;
                        out[n].r2 = nr; out[n].c2 = nc;
                        n++;
                    }
                    nr += DR4[d];
                    nc += DC4[d];
                }
            }
        }
    }
    return n;
}

/**
 * @brief Détermine la direction unitaire d'un mouvement.
 * @param r1 Ligne de départ.
 * @param c1 Colonne de départ.
 * @param r2 Ligne d'arrivée.
 * @param c2 Colonne d'arrivée.
 * @param[out] dr Pointeur pour stocker la direction en ligne (-1, 0, ou 1).
 * @param[out] dc Pointeur pour stocker la direction en colonne (-1, 0, ou 1).
 */
static void unit_dir(int r1,int c1,int r2,int c2,int*dr,int*dc){
    int rr=r2-r1, cc=c2-c1;
    *dr = (rr>0) ? 1 : (rr<0 ? -1 : 0);
    *dc = (cc>0) ? 1 : (cc<0 ? -1 : 0);
}

/**
 * @brief Simule une capture par poussée (Seultou).
 * @param b Pointeur vers le plateau à modifier.
 * @param r Ligne de la pièce attaquante.
 * @param c Colonne de la pièce attaquante.
 * @param dr Direction de la ligne d'attaque.
 * @param dc Direction de la colonne d'attaque.
 */
static void simulate_push_capture(Board* b, int r, int c, int dr, int dc) {
    if (dr==0 && dc==0) return;
    int me = b->pion[r][c];
    bool blueSide = is_blue(me);

    int vr = r + dr, vc = c + dc;        // victime
    if (!in_bounds(vr,vc)) return;

    int victim = b->pion[vr][vc];
    if (!enemy(victim, blueSide)) return;

    int gr = r + 2*dr, gc = c + 2*dc;    // garde
    if (in_bounds(gr,gc)) {
        int guard = b->pion[gr][gc];
        if (enemy(guard, blueSide)) return;
        b->pion[vr][vc] = EMPTY;
    } else {
        b->pion[vr][vc] = EMPTY;
    }
}

/**
 * @brief Simule une capture par sandwich (Linca).
 * @param b Pointeur vers le plateau à modifier.
 * @param r Ligne de la pièce qui vient de bouger.
 * @param c Colonne de la pièce qui vient de bouger.
 */
static void simulate_sandwich(Board * b, int r, int c) {
    int me = b->pion[r][c];
    bool blueSide = is_blue(me);

    for (int d=0; d<4; d++) {
        int nr = r + DR4[d], nc = c + DC4[d];
        int fr = r + 2*DR4[d], fc = c + 2*DC4[d];
        if (!in_bounds(nr,nc) || !in_bounds(fr,fc)) continue;

        int nearP = b->pion[nr][nc];
        int farP  = b->pion[fr][fc];

        if (enemy(nearP, blueSide) && ally(farP, blueSide)) {
            b->pion[nr][nc] = EMPTY;
        }
    }
}

/**
 * @brief Applique un coup sur le plateau, incluant les captures qui en découlent.
 * @param b Pointeur vers le plateau à modifier.
 * @param m Le coup à appliquer.
 */
static void apply_move(Board* b, const Move* m) {
    int p = b->pion[m->r1][m->c1];
    b->pion[m->r2][m->c2] = p;
    b->pion[m->r1][m->c1] = EMPTY;

    int dr,dc; unit_dir(m->r1,m->c1,m->r2,m->c2,&dr,&dc);
    simulate_push_capture(b, m->r2, m->c2, dr, dc);
    simulate_sandwich(b, m->r2, m->c2);
}

// Conditions de victoire 

/**
 * @see ia.h
 */
int check_winner(const Board* b) {
    bool blueKingAlive=false, redKingAlive=false;
    int rb=-1, cb=-1, rr=-1, cr=-1;
    int blueSoldiers=0, redSoldiers=0;

    for (int r=0; r<SIZE; r++) {
        for (int c=0; c<SIZE; c++) {
            int p = b->pion[r][c];
            if (p == ROI_BLEU)  { blueKingAlive = true; rb=r; cb=c; }
            if (p == ROI_ROUGE) { redKingAlive  = true; rr=r; cr=c; }
            if (p == SOLDAT_BLEU)  blueSoldiers++;
            if (p == SOLDAT_ROUGE) redSoldiers++;
        }
    }

    if (!redKingAlive)  return +1;
    if (!blueKingAlive) return -1;

    if (rb==SIZE-1 && cb==SIZE-1) return +1;
    if (rr==0 && cr==0)           return -1;

    if (redSoldiers == 0)  return +1;
    if (blueSoldiers == 0) return -1;

    return 0;
}

// Évaluation 

/**
 * @brief Calcule la mobilité d'un camp (nombre de coups légaux).
 * @param b Le plateau de jeu.
 * @param blueSide true pour évaluer la mobilité des bleus.
 * @return Le nombre de coups possibles.
 */
static int mobility(const Board* b, bool blueSide) {
    Move tmp[MAX_MOVES];
    return generate_moves(b, blueSide, tmp, MAX_MOVES);
}

/**
 * @brief Fonction d'évaluation heuristique d'une position.
 * @param b Le plateau de jeu à évaluer.
 * @return Un score entier (positif pour avantage bleu, négatif pour rouge).
 */
static int evaluate(const Board* b) {
    int score = 0;

    for (int r=0; r<SIZE; r++) {
        for (int c=0; c<SIZE; c++) {
            switch (b->pion[r][c]) {
                case SOLDAT_BLEU:  score += 12;  break;
                case ROI_BLEU:     score += 300; break;
                case SOLDAT_ROUGE: score -= 12;  break;
                case ROI_ROUGE:    score -= 300; break;
                default: break;
            }
        }
    }

    int bestBlueKingDist = 100, bestRedKingDist = 100;
    for (int r=0; r<SIZE; r++) for (int c=0; c<SIZE; c++) {
        if (b->pion[r][c] == ROI_BLEU) {
            int d = (SIZE-1 - r) + (SIZE-1 - c);
            if (d < bestBlueKingDist) bestBlueKingDist = d;
        } else if (b->pion[r][c] == ROI_ROUGE) {
            int d = r + c;
            if (d < bestRedKingDist) bestRedKingDist = d;
        }
    }
    if (bestBlueKingDist < 100) score += (30 - bestBlueKingDist);
    if (bestRedKingDist < 100)  score -= (30 - bestRedKingDist);

    score += (mobility(b,true) - mobility(b,false));

    for (int r=0; r<SIZE; r++) for (int c=0; c<SIZE; c++) {
        if (b->pion[r][c] == ROI_BLEU) {
            score += (4 - abs(r-4)) + (4 - abs(c-4));
        } else if (b->pion[r][c] == ROI_ROUGE) {
            score -= (4 - abs(r-4)) + (4 - abs(c-4));
        }
    }

    return score;
}

// Table de transpositions 

/**
 * @brief Accède à une entrée de la table de transposition.
 * @param key La clé de Zobrist.
 * @return Pointeur vers l'entrée correspondante.
 */
static inline TTEntry* tt_probe(uint64_t key) {
    return &TT[key & TT_MASK];
}

/**
 * @brief Stocke une nouvelle entrée dans la table de transposition.
 * @param key La clé de Zobrist.
 * @param depth La profondeur de recherche.
 * @param value La valeur de l'évaluation.
 * @param flag Le type de nœud (EXACT, LOWER, UPPER).
 * @param best Le meilleur coup trouvé depuis cette position.
 */
static inline void tt_store(uint64_t key, int depth, int value, TTFlag flag, Move best) {
    TTEntry *e = &TT[key & TT_MASK];
    if (e->key == 0 || depth >= e->depth) {
        e->key = key;
        e->depth = (int8_t)depth;
        e->value = (int16_t)EVAL_CLAMP(value);
        e->flag = (uint8_t)flag;
        e->best = best;
    }
}

// Move ordering  

/**
 * @brief Attribue un score à un coup pour l'ordonnancement.
 * @param b Le plateau.
 * @param blueToPlay true si c'est au tour des bleus.
 * @param m Le coup à évaluer.
 * @param ttMove Le meilleur coup suggéré par la table de transposition (peut être NULL).
 * @return Un score entier pour le coup.
 */
static int move_score(const Board* b, bool blueToPlay, const Move* m, const Move* ttMove) {
    int score = 0;

    if (ttMove && m->r1==ttMove->r1 && m->c1==ttMove->c1 && m->r2==ttMove->r2 && m->c2==ttMove->c2) {
        score += 100000;
    }

    int p = b->pion[m->r1][m->c1];

    int dr,dc; unit_dir(m->r1,m->c1,m->r2,m->c2,&dr,&dc);
    int vr = m->r2 + dr, vc = m->c2 + dc;
    if (in_bounds(vr,vc) && enemy(b->pion[vr][vc], is_blue(p))) {
        score += 5000;
    }

    if (p == ROI_BLEU) {
        int before = (SIZE-1 - m->r1) + (SIZE-1 - m->c1);
        int after  = (SIZE-1 - m->r2) + (SIZE-1 - m->c2);
        score += (before - after) * 50;
    } else if (p == ROI_ROUGE) {
        int before = (m->r1) + (m->c1);
        int after  = (m->r2) + (m->c2);
        score += (before - after) * 50;
    }

    score -= (abs(m->r2 - m->r1) + abs(m->c2 - m->c1));

    return score;
}

/**
 * @brief Trie une liste de coups du meilleur au moins bon.
 * @param b Le plateau.
 * @param blueToPlay true si c'est au tour des bleus.
 * @param moves Le tableau de coups à trier.
 * @param n Le nombre de coups dans le tableau.
 * @param ttMove Le meilleur coup suggéré par la table de transposition.
 */
static void sort_moves(Board* b, bool blueToPlay, Move* moves, int n, const Move* ttMove) {
    for (int i=1;i<n;i++) {
        Move key = moves[i];
        int keyScore = move_score(b, blueToPlay, &key, ttMove);
        int j = i-1;
        while (j>=0) {
            int s = move_score(b, blueToPlay, &moves[j], ttMove);
            if (s >= keyScore) break;
            moves[j+1] = moves[j];
            j--;
        }
        moves[j+1] = key;
    }
}

// Minimax Alpha-Beta

/**
 * @brief Fonction récursive de recherche Minimax avec élagage Alpha-Bêta.
 * @param b Pointeur vers le plateau (sera modifié et restauré).
 * @param depth Profondeur de recherche restante.
 * @param alpha La meilleure valeur garantie pour le joueur maximisant (bleu).
 * @param beta La meilleure valeur garantie pour le joueur minimisant (rouge).
 * @param blueToPlay true si le joueur actuel est bleu.
 * @param key La clé de Zobrist de la position actuelle.
 * @param[out] outBest Pointeur pour stocker le meilleur coup trouvé.
 * @return L'évaluation de la position.
 */
static int minimax(Board* b, int depth, int alpha, int beta, bool blueToPlay, uint64_t key, Move* outBest) {
    int winner = check_winner(b);
    if (winner == +1) return  100000;
    if (winner == -1) return -100000;
    if (depth == 0)   return evaluate(b);

    TTEntry *e = tt_probe(key);
    Move ttMove = { -1,-1,-1,-1 };
    if (e->key == key) {
        if (e->depth >= depth) {
            if (e->flag == TT_EXACT) return e->value;
            if (e->flag == TT_LOWER && e->value > alpha) alpha = e->value;
            else if (e->flag == TT_UPPER && e->value < beta) beta = e->value;
            if (alpha >= beta) return e->value;
        }
        ttMove = e->best;
    }

    Move moves[MAX_MOVES];
    int n = generate_moves(b, blueToPlay, moves, MAX_MOVES);
    if (n == 0) return evaluate(b);

    sort_moves(b, blueToPlay, moves, n, &ttMove);

    int bestVal = blueToPlay ? -INF_SCORE : INF_SCORE;
    Move bestMove = moves[0];

    for (int i=0;i<n;i++) {
        Board save = *b;
        apply_move(b, &moves[i]);

        uint64_t childKey = zobrist_hash(b, !blueToPlay);
        int val = minimax(b, depth-1, alpha, beta, !blueToPlay, childKey, outBest);

        *b = save;

        if (blueToPlay) {
            if (val > bestVal) { bestVal = val; bestMove = moves[i]; }
            if (bestVal > alpha) alpha = bestVal;
        } else {
            if (val < bestVal) { bestVal = val; bestMove = moves[i]; }
            if (bestVal < beta) beta = bestVal;
        }
        if (alpha >= beta) break;
    }

    TTFlag flag;
    if (bestVal <= alpha) flag = TT_UPPER;
    else if (bestVal >= beta) flag = TT_LOWER;
    else flag = TT_EXACT;

    tt_store(key, depth, bestVal, flag, bestMove);
    if (outBest) *outBest = bestMove;
    return bestVal;
}

// Iterative Deepening 

/**
 * @see ia.h
 */
Move search_best_move(const Board* start, bool blueToPlay) {
    Board b = *start;

    uint64_t key = zobrist_hash(&b, blueToPlay);
    Move best = { -1,-1,-1,-1 };

    Move hint = blueToPlay ? g_last_best_move_blue : g_last_best_move_red;
    if (hint.r1>=0) {
        TTEntry *e = tt_probe(key);
        tt_store(key, 0, 0, TT_EXACT, hint);
    }

    for (int d=1; d<=MAX_DEPTH; d++) {
        Move iterBest = best;
        int alpha = -INF_SCORE, beta = INF_SCORE;
        int val = minimax(&b, d, alpha, beta, blueToPlay, key, &iterBest);
        (void)val;
        if (iterBest.r1>=0) best = iterBest;
    }

    if (blueToPlay) g_last_best_move_blue = best;
    else            g_last_best_move_red  = best;

    return best;
}

// API publique 

/**
 * @see ia.h
 */
void ia_init_once(void) {
    if (!TT) {
        TT = (TTEntry*)calloc(TT_SIZE, sizeof(TTEntry));
        zobrist_init();
    }
}

