#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 256

/**
 * PNMファイルを読み込む
 * 
 * 引数
 * file_name: 読み込むファイルのパス
 * file_size: 読み込んだファイルのサイズ
 * 
 * 返却値
 * 成功: 読み込んだファイルデータの先頭アドレス
 * 失敗: NULL
 * 
 */
char * readFileData(char *file_name, unsigned int *file_size) {
    char *file_data = NULL;
    size_t read_size;
    unsigned int size;
    char tmp[256];
    FILE *fp;

    /* ファイルサイズ取得用にオープン */
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        printf("%sが読み込めません\n", file_name);
        return NULL;
    }

    /* ファイルのサイズを取得 */
    size = 0;
    do {
        read_size = fread(tmp, 1, 256, fp);
        size += read_size;
    } while (read_size == 256);

    /* ファイルを一旦クローズ */
    fclose(fp);

    /* ファイルデータ読み込み用にオープン */
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        printf("%sが読み込めません\n", file_name);
        return NULL;
    }

    /* ファイルデータ読み込み用のメモリ確保 */
    file_data = (char*)malloc(sizeof(char) * size);
    if (file_data == NULL) {
        printf("メモリが取得できません\n");
        fclose(fp);
        return NULL;
    }

    /* データの読み込み */
    read_size = fread(file_data, 1, size, fp);

    fclose(fp);

    if (read_size != size) {
        printf("読み込みサイズがおかしいです\n");
        free(file_data);
        return NULL;
    }

    /* サイズを設定 */
    *file_size = (unsigned int)size;

    /* 読み込んだデータの先頭アドレス返却 */
    return file_data;
}

int main() {
  /* ポート番号、ソケット */
  unsigned short port = 8080;
  int srcSocket;  // 自分
  int dstSocket;  // 相手

  /* sockaddr_in 構造体 */
  struct sockaddr_in srcAddr;
  struct sockaddr_in dstAddr;
  int dstAddrSize = sizeof(dstAddr);

  /* 各種パラメータ */
  int numrcv;
  char buffer[BUFFER_SIZE];

  /************************************************************/
  /* Windows 独自の設定 */
  WSADATA data;
  WSAStartup(MAKEWORD(2,0), &data); 

  /* sockaddr_in 構造体のセット */
  memset(&srcAddr, 0, sizeof(srcAddr));
  srcAddr.sin_port = htons(port);
  srcAddr.sin_family = AF_INET;
  srcAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

  /* ソケットの生成 */
  srcSocket = socket(AF_INET, SOCK_STREAM, 0);

  /* ソケットのバインド */
  bind(srcSocket, (struct sockaddr *) &srcAddr, sizeof(srcAddr));

  /* 接続の許可 */
  listen(srcSocket, 1);

  /* ---------- 画像読み込み ---------- */
  char file_name[256];
  unsigned int file_size;
  char *file_data;

  strcpy(file_name, "./img/temp.pgm");

  /* ファイル全体のデータを読み込む */
  file_data = readFileData(file_name, &file_size);
  if (file_data == NULL) {
      printf("ファイルデータの読み込みに失敗しました\n");
      return -1;
  }

  printf("file_size: %d\n", file_size);
  printf("file_size: %s\n", file_data);

  /* ---------- 画像読み込み ---------- */


  /* 接続の受付け */
  printf("Waiting for connection ...\n");
  dstSocket = accept(srcSocket, (struct sockaddr *) &dstAddr, &dstAddrSize);
  printf("Connected from %s\n", inet_ntoa(dstAddr.sin_addr));

  /* パケット受信 */
  while(1) {
    numrcv = recv(dstSocket, buffer, BUFFER_SIZE, 0); 
    if(numrcv == 0 || numrcv == -1) {
      closesocket(dstSocket);
      break;
    }
    printf("received: %s\n", buffer);

    // この後送るfileの大きさを送る
    send(dstSocket, (char*)&file_size, sizeof(int), 0);

    // fileを送信
    send(dstSocket, file_data, file_size, 0);
  }

  /* Windows 独自の設定 */
  WSACleanup();
}