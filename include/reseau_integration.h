/**
 * @file reseau_integration.h
 * @authors Groupe 8
 * @brief reseau_integration.h déclare les fonctions qui intègrent la couche réseau avec la logique de jeu.
 *
 * Il sert de pont entre les fonctions de `reseau.h` et
 * l'interface utilisateur de `jeu.c`.
 */

#ifndef RESEAU_INTEGRATION_H
#define RESEAU_INTEGRATION_H

/**
 * @struct MoveData
 * @brief Structure pour passer les informations d'un coup à l'interface GTK de manière asynchrone.
 */
typedef struct
{
    char src_id[3]; /**< Identifiant de la case de départ (ex: "A1"). */
    char dst_id[3]; /**< Identifiant de la case d'arrivée (ex: "B1"). */
} MoveData;

/**
 * @brief Initialise l'état réseau du jeu.
 * @param sock Le socket de communication.
 * @param server_mode 1 si le programme est un serveur, 0 si c'est un client.
 */
void network_init(int sock, int server_mode);

/**
 * @brief Boucle d'écoute réseau à exécuter dans un processus dédié.
 */
void network_listen_loop(void);

/**
 * @brief Formate et envoie un coup sur le réseau.
 * @param src_id Identifiant de la case de départ.
 * @param dst_id Identifiant de la case d'arrivée.
 */
void send_move_to_network(const char *src_id, const char *dst_id);

/**
 * @brief Traite un coup reçu du réseau et l'applique à l'interface.
 * @param move La chaîne de 4 caractères représentant le coup.
 */
void apply_network_move(const char *move);

#endif