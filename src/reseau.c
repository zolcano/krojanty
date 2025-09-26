/**
 * @file reseau.c
 * @brief Implémentation des fonctions pour la communication réseau TCP.
 * @authors Groupe 8
 *
 * reseau.c fournit les briques de base pour la communication réseau du jeu,
 * en s'appuyant sur l'API des sockets. Il gère la création de serveur, la
 * connexion client, et l'envoi/réception de données formatées pour le jeu.
 */


#include "reseau.h"
#include "config.h"

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * @see reseau.h
 */
int net_wait_for_client()
{
    int server_fd, client_fd; // socket d'écoute et de comm
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        return -1;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(config.port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(server_fd);
        return -1;
    }
    if (listen(server_fd, 1) < 0)
    {
        close(server_fd);
        return -1;
    }

    client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
    if (client_fd < 0)
    {
        close(server_fd);
        return -1;
    }

    close(server_fd);
    return client_fd;
}


/**
 * @see reseau.h
 */
int net_connect_to_server()
{
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return -1;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(config.port);

    if (inet_pton(AF_INET, config.address, &serv_addr.sin_addr) <= 0)
    {
        close(sock);
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        close(sock);
        return -1;
    }

    return sock;
}

/**
 * @see reseau.h
 */
int net_send_move(int sock, const char *move)
{
    int len = strlen(move);
    if (len != 4)
        return -1;
    return send(sock, move, 4, 0);
}

/**
 * @see reseau.h
 */
int net_recv_move(int sock, char *buffer)
{
    int n = recv(sock, buffer, 4, MSG_WAITALL);
    if (n <= 0)
        return -1;
    buffer[4] = '\0';
    return 0;
}
