/*
 * Snootlab
 *-----------------------------------------------------------------------------------------
 * Example of a communication between two Chistera-Pi or between a Chistera-Pi and a sensor
 * with a RFM95 like Snootlab's TeensyWiNo.
 *
 * This sender gets input from a local Unix Domain Socket and sends it over LoRa.
 *
 * usage: ./<software_name> <path_name>
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <RH_RF95.h>

RH_RF95 rf95;

/* Setup the hardware */

void setup()
{
    wiringPiSetupGpio();

    if (!rf95.init())
    {
        fprintf(stderr, "Init failed\n");
        exit(1);
    }

    /* Tx power is from +5 to +23 dBm */
    rf95.setTxPower(23);
    /* There are different configurations
     * you can find in lib/radiohead/RH_RF95.h
     * at line 437
     */
    rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);
    rf95.setFrequency(868.0); /* Mhz */
}

/* Transmit wirelessly what is received from the socket */

void loop(char *name)
{
    int sock, msgsock, rval;
    struct sockaddr_un server;
    char msg[1024];

    /* initialise socket file */

    unlink(name);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, name);
    if (bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)))
    {
        perror("binding stream socket");
        exit(1);
    }
    printf("using %s", name)

    /* process every client request */

    printf("hit Ctl-C to break\n");
    listen(sock, 5);
    while( run )
    {

        printf("waiting for a connection\n");
        msgsock = accept(sock, 0, 0);
        if (msgsock == -1)
            perror("accept");
        else do {

            /* get a message */

            bzero(msg, sizeof(msg));
            if ((rval = read(msgsock, msg, 1024)) < 0)
                perror("reading message");
            else if (rval == 0)
                printf("ending connection\n");
            else {

                /* transmit it wirelessly */

                printf("--> %s\n", msg);

                rf95.send(msg, sizeof(msg));
                rf95.waitPacketSent();
                printf("transmitted\n");

            }

        } while (rval > 0);
        close(msgsock);
    }

    /* kill socket file */

    close(sock);
    unlink(name);
}

/* Should we run or stop */

int run = 1;

void sigint_handler(int signal)
{
    run = 0;
}

/* Set everything and start processing */

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage:%s <path_name>", argv[0]);
        exit(1);
    }

    signal(SIGINT, sigint_handler);

    setup();

    loop(argv[1])

    return EXIT_SUCCESS;
}