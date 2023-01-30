#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

int
main() {
  /* IP アドレス、ポート番号、ソケット */
  char *destination = "127.0.0.1";
  unsigned short port = 8080;
  int dstSocket;

  /* sockaddr_in 構造体 */
  struct sockaddr_in dstAddr;

  /* 各種パラメータ */
  int status;
  int numsnt;
  char *toSendText = "This is a test";

  /************************************************************/

  /* Windows 独自の設定 */
  WSADATA data;
  WSAStartup(MAKEWORD(2,0), &data);


  /* sockaddr_in 構造体のセット */
  memset(&dstAddr, 0, sizeof(dstAddr));
  dstAddr.sin_port = htons(port);
  dstAddr.sin_family = AF_INET;
  dstAddr.sin_addr.s_addr = inet_addr(destination);
 
  /* ソケット生成 */
  dstSocket = socket(AF_INET, SOCK_STREAM, 0);

  /* 接続 */
  printf("Trying to connect to %s: \n", destination);
  connect(dstSocket, (struct sockaddr *) &dstAddr, sizeof(dstAddr));

  /* パケット送出 */
  for(int i=0; i<10; i++) {
    printf("sending...\n");
    send(dstSocket, toSendText, strlen(toSendText)+1, 0);
    Sleep(1000);
  }

  /* Windows 独自の設定 */
  closesocket(dstSocket);
  WSACleanup();
}