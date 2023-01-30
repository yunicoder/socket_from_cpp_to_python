#include <iostream>

#include <netinet/in.h>
#include <netdb.h>

#include <sys/socket.h> //アドレスドメイン
#include <sys/types.h>  //ソケットタイプ
#include <arpa/inet.h>  //バイトオーダの変換に利用
#include <unistd.h>     //close()に利用
#include <cstring>
#include <string>       //string型


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

void getFileNames(
    std::string path,
    std::vector<std::string> &file_names,
    std::vector<size_t> &labels,
    size_t cur_label = 0)
{
  for(const std::filesystem::directory_entry &i:std::filesystem::directory_iterator(path)){
    if(i.is_directory()){
        std::cout << i.path().filename().string() << std::endl;

        // ラベルを計算
        cur_label = std::stoi(i.path().filename().string());

        getFileNames(i.path().filename().string(),file_names,labels,cur_label);
    }else{
      std::cout << i.path().filename().string() << std::endl;
      file_names.push_back(i.path().filename().string());
      labels.push_back(cur_label);
    }
  }
}


int main() {
    std::vector<std::string> dir_names;
    std::vector<size_t> labels;
    getFileNames("./img/", dir_names, labels);
    // winならこれ？https://qiita.com/tes2840/items/8d295b1caaf10eaf33ad



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

    /* 接続の受付け */
    printf("Waiting for connection ...\n");
    dstSocket = accept(srcSocket, (struct sockaddr *) &dstAddr, (socklen_t *) &dstAddrSize);
    printf("Connected from %s\n", inet_ntoa(dstAddr.sin_addr));

    // 接続確認
    numrcv = recv(dstSocket, buffer, BUFFER_SIZE, 0); 
    if(numrcv == 0 || numrcv == -1) {
        close(dstSocket);
        return -1;
    }
    printf("received: %s\n", buffer);

    // この後送る画像の数
    int n_images = 3;
    send(dstSocket, (char*)&n_images, sizeof(int), 0);

    /* パケット受信 */
    for(int i=0; i<n_images; i++) {
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

    // ラベルを送る
    size_t label = 0;
    send(dstSocket, (char*)&label, sizeof(size_t), 0);

    // この後送るfileの大きさを送る
    send(dstSocket, (char*)&file_size, sizeof(int), 0);

    // fileを送信
    send(dstSocket, file_data, file_size, 0);
    }
}