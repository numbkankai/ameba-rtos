#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BROADCAST_PORT 8879
#define BROADCAST_IP "10.10.18.255"
#define BUFFER_SIZE 512
#define INTERVAL_MS 100000 // 100ms in microseconds

int main()
{
    printf("start\n");
    int sockfd;
    struct sockaddr_in broadcast_addr;
    char buffer[BUFFER_SIZE];

    // 创建UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("创建socket失败\n");
        return -1;
    }

    // 设置socket选项以允许广播
    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0)
    {
        printf("设置socket选项失败\n");
        close(sockfd);
        return -1;
    }

    // 配置广播地址
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    broadcast_addr.sin_port = htons(BROADCAST_PORT);

    // 循环发送UDP广播
    while (1)
    {
#if 0
        memset(buffer, 'A', BUFFER_SIZE); // 填充数据
        ssize_t sent_bytes = sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
        if (sent_bytes < 0)
        {
            printf("发送广播失败\n");
            break;
        }
        usleep(INTERVAL_MS); // 等待100ms
#else
        struct sockaddr_in recv_addr;
        socklen_t addr_len = sizeof(recv_addr);
        char recv_buffer[BUFFER_SIZE];

        // 创建接收UDP socket
        int recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (recv_sockfd < 0)
        {
            printf("创建接收socket失败\n");
            break;
        }

        // 配置接收地址
        memset(&recv_addr, 0, sizeof(recv_addr));
        recv_addr.sin_family = AF_INET;
        recv_addr.sin_addr.s_addr = INADDR_ANY; // 接收任意IP
        recv_addr.sin_port = htons(1549);       // 端口1549

        // 绑定socket
        if (bind(recv_sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0)
        {
            printf("绑定socket失败\n");
            close(recv_sockfd);
            break;
        }

        // 循环接收UDP消息
        while (1)
        {
            ssize_t recv_bytes = recvfrom(recv_sockfd, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&recv_addr, &addr_len);
            if (recv_bytes < 0)
            {
                printf("接收消息失败\n");
                break;
            }
#if 0
            printf("接收到消息: ");
            uint16_t i = 0;
            for (i = 0; i < recv_bytes; i++)
            {
                printf("%02X ", (unsigned char)recv_buffer[i]);
            }
            printf("\n");
#endif
            // printf("发送广播消息\n");
            ssize_t sent_bytes = sendto(sockfd, recv_buffer, recv_bytes, 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
            if (sent_bytes < 0)
            {
                printf("发送广播失败\n");
            }
        }

        close(recv_sockfd);
#endif
    }

    close(sockfd);
    return 0;
}
