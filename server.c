// MinGWでコンパイルする際に必要
// _WIN32_WINNT の定義について、
// Windowsのバージョンごとに定義が決まっている
// インクルードしたヘッダファイルよりも先に宣言
#define _WIN32_WINNT 0x0A00 

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_PORT "27015"

//このpragmaが使えるのはMinGW(gcc)だけ、他の実行環境では動作しない
#pragma comment (lib, "Ws2_32.lib")

int main()
{

    //Windows Socket API(WSA)の初期化
    WSADATA wsaData;
    int iResult;

    //winsock2の初期化
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    printf("WSAStartup Succeed to Initialized\n");


    struct addrinfo *result = NULL, 
                    *ptr = NULL;
    struct addrinfo hints;

    //配列, 構造体の格納領域を0埋めして初期化する
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {   
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    printf("getaddrinfo Succeed to Initialized\n");

    //サーバーがクライアント接続をリッスンするための
    //SOCKETオブジェクトを作成する
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(
        result->ai_family, 
        result->ai_socktype,
        result->ai_protocol
    );

    if (ListenSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("ListenSocket Succeed to Initialized\n");


    //TCPソケットのバインド
    iResult = bind(
        ListenSocket, 
        result->ai_addr,
        (int)result->ai_addrlen
    );

    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("tcp socket Succeed to build\n");

    //ソケットでリッスンする
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("tcp socket listen succeed\n");

    //接続を受け入れる
    SOCKET ClientSocket;
    ClientSocket = INVALID_SOCKET;

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("client socket accepted succeed\n");

    

    return 0;

}