#define _WIN32_WINNT 0x0A00 
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

#pragma comment (lib, "Ws2_32.lib")

int main()
{

    WSADATA wsaData;
    int iResult;

    //windowsソケットの初期化
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) 
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    //SOCKETの作成準備
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    // サーバー名からIPアドレスを取得
    iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    //SOCKETオブジェクトの作成
    SOCKET ConnectSocket = INVALID_SOCKET;
    ptr = result; //getaddrinfoで取得した最初のIPアドレスに接続する

    ConnectSocket = socket(
        ptr->ai_family, 
        ptr->ai_socktype,
        ptr->ai_protocol
    );

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    //サーバーに接続
    iResult = connect(
        ConnectSocket, 
        ptr->ai_addr, 
        (int)ptr->ai_addrlen
    );

    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // サーバーに接続するためのソケットを生成
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // サーバーに接続
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }


    int recvbuflen = DEFAULT_BUFLEN;
    const char *sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];

    //バッファの先頭をサーバーへ送信
    iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    printf("Bytes Sent: %ld\n", iResult);

    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    //サーバーが接続を閉じるまでデータを受信する
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            printf("Bytes received: %d\n", iResult);
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed: %d\n", WSAGetLastError());
    } 
    while (iResult > 0);

    //ソケットの切断
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}