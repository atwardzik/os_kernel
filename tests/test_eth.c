//
// Created by Artur Twardzik on 07/06/2026.
//

#include "libc.h"
#include "socket.h"
#include "kernel/network.h"


void test_raw_ethernet_frames(void) {
        int sockfd = socket(AF_INET, SOCK_RAW, 0);
        if (sockfd < 0) {
                return;
        }

        for (int i = 0; i < 100; ++i) {
                char test_data[64] = "This will be a TCP stack with Modbus on top: ";
                char num[10];
                itoa(i, num, 10);
                strcat(test_data, num);

                char src[6];
                char dst[6];
                str2mac("de:da:be:ba:fe:fa", dst);
                str2mac("de:ad:01:10:be:ef", src);

                char frame[128];

                memcpy(frame, src, 6);
                memcpy(frame + 6, dst, 6);
                frame[12] = 0x88b5 >> 8;
                frame[13] = 0x88b5 & 0xff;
                memcpy(frame + 14, test_data, strlen(test_data));
                write(sockfd, frame, strlen(test_data) + 14);
        }

        close(sockfd);
}

void test_tcp_server(void) {
        while (1) {
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0) {
                        dprintf(2, "Error while trying to open socket\n");
                        return;
                }

                struct sockaddr_in source = {AF_INET, 8080};
                bind(sockfd, (struct sockaddr *) &source, sizeof(source));

                if (listen(sockfd, 1) < 0) {
                        dprintf(2, "Error while trying to listen on port: %i\n", source.sin_port);
                        return;
                }

                struct sockaddr destination = {};
                if (accept(sockfd, &destination, 0) == 0) {
                        printf("Connection accepted :)\n");

                        const char *static_website =
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html\r\n"
                                "Content-Length: 76\r\n"
                                "Connection: close\r\n\r\n"
                                "<html><body><h1>Server on GeT Computer v0.1 responded :)</h1></body></html>\n";

                        write(sockfd, static_website, strlen(static_website));
                }

                close(sockfd);
        }
}

void test_tcp_client(void) {
        while (1) {
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0) {
                        dprintf(2, "Error while trying to open socket\n");
                        continue;
                }
                struct sockaddr_in source = {AF_INET, 8080};
                bind(sockfd, (struct sockaddr *) &source, sizeof(source));


                struct sockaddr_in dest = {AF_INET, 53764, {192, 168, 1, 41}};

                if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) < 0) {
                        dprintf(2, "Error while trying to connect - connection timed out\n");
                        close(sockfd);
                        continue;
                }

                char buffer[64] = {0};
                read(sockfd, buffer, 64);

                printf("Recv: %s\n", buffer);

                close(sockfd);
        }
}