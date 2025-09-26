/**
 * @file reseau.h
 * @authors Groupe 8
 * @brief reseau.h déclare les fonctions de bas niveau pour la communication réseau TCP.
 *
 * Il fournit une interface simple pour créer un serveur, se connecter
 * à un serveur, et échanger des données de jeu via des sockets.
 */

#ifndef RESEAU_H
#define RESEAU_H

/**
 * @brief Met le programme en mode serveur et attend la connexion d'un client.
 * @return Le descripteur de fichier du socket connecté au client, ou -1 en cas d'erreur.
 */
int net_wait_for_client();

/**
 * @brief Établit une connexion avec un serveur en mode client.
 * @return Le descripteur de fichier du socket connecté au serveur, ou -1 en cas d'erreur.
 */
int net_connect_to_server();

/**
 * @brief Envoie un coup au travers d'un socket.
 * @param sock Le socket connecté.
 * @param move La chaîne de 4 caractères représentant le coup (ex: "A1A3").
 * @return 0 si succès, -1 en cas d'erreur.
 */
int net_send_move(int sock, const char *move);

/**
 * @brief Reçoit un coup depuis un socket.
 * @param sock Le socket connecté.
 * @param buffer Buffer d’au moins 5 octets pour stocker le coup reçu (4 + '\0').
 * @return 0 si succès, -1 en cas d'erreur ou de déconnexion.
 */
int net_recv_move(int sock, char *buffer);

#endif