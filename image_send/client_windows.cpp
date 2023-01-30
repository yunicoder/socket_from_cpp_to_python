#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h> // isspace
#include <iostream> // cout
#include <vector> // vector

#define BUFFER_SIZE 256
#define MAX_VALUE 256

/* 画像データ */
typedef struct {
    unsigned int width; /* 画像の横サイズ */
    unsigned int height; /* 画像の縦サイズ */
    unsigned int num_bit; /* 1ピクセルあたりのビット数 */
    unsigned int max_value; /* 最大輝度値 */
    unsigned char *data; /* 画像データの先頭アドレス */
} IMAGE;

/* PNMのタイプ */
typedef enum {
    PNM_TYPE_PBM_ASCII,
    PNM_TYPE_PGM_ASCII,
    PNM_TYPE_PPM_ASCII,
    PNM_TYPE_PBM_BINARY, 
    PNM_TYPE_PGM_BINARY,
    PNM_TYPE_PPM_BINARY,
    PNM_TYPE_ERROR
} PNM_TYPE;

/* 関数のプロトタイプ宣言 */
int allocImage(IMAGE *);
unsigned int getImageInfo(IMAGE *, char *, unsigned int, PNM_TYPE);
unsigned int getNextValue(unsigned int *, char *, unsigned int, unsigned int);
int readP5(IMAGE *, char *, unsigned int);

/**
 * 画像データ格納用のバッファを確保する
 * 
 * 引数
 * image: 画像データ格納用の構造体
 * 
 * 返却値
 * 成功: 0
 * 失敗: 0以外
 */
int allocImage(IMAGE *image) {
    unsigned int size;
    unsigned char *data;
    unsigned int line_byte;

    if (image == NULL) {
        return -1;
    }

    /* 1行あたりのバイト数を計算（切り上げ） */
    line_byte = (image->width * image->num_bit + 7) / 8;

    /* サイズを決定してメモリ取得 */
    size = line_byte * image->height;
    data = (unsigned char *)malloc(sizeof(unsigned char) * size);
    if (data == NULL) {
        printf("mallocに失敗しました\n");
        return -1;
    }

    /* 取得したメモリのアドレスをimage構造体にセット */
    image->data = data;

    return 0;

}

/**
 * ヘッダーを読み込み結果をIMAGE構造体に格納する
 * 
 * 引数
 * image: 画像データ格納用の構造体
 * file_data: ファイルデータの先頭アドレス
 * file_size: ファイルデータのサイズ
 * 
 * 返却値
 * 成功: 画像データの先頭位置
 * 失敗: 0
 */
unsigned int getImageInfo(IMAGE *image, char *file_data, unsigned int file_size, PNM_TYPE type) {

    unsigned int read_pos;
    unsigned int value;
    unsigned int read_size;

    /* データ読み込み位置を先頭にセット */
    read_pos = 0;

    /* マジックナンバー分を読み飛ばす */
    read_pos += 2;

    /* 画像の横サイズを取得する */
    read_size = getNextValue(&value, file_data, read_pos, file_size);
    image->width = value;

    read_pos += read_size;

    /* 画像の縦サイズを取得する */
    read_size = getNextValue(&value, file_data, read_pos, file_size);
    image->height = value;

    read_pos += read_size;

    /* 画像の最大輝度値を取得する */
    switch (type) {
        case PNM_TYPE_PGM_ASCII:
        case PNM_TYPE_PPM_ASCII:
        case PNM_TYPE_PGM_BINARY:
        case PNM_TYPE_PPM_BINARY:
            /* 取得するのはPGMとPBMのみ */
            read_size = getNextValue(&value, file_data, read_pos, file_size);

            /* 最大輝度値の値チェック */
            if (value > MAX_VALUE) {
                printf("最大輝度値が不正です\n");
                return 0;
            }

            image->max_value = value;
            read_pos += read_size;
            break;
        default:
            break;
    }

    /* PNMタイプに応じてピクセルあたりのバイト数を設定 */
    switch (type) {
        case PNM_TYPE_PBM_ASCII:
        case PNM_TYPE_PBM_BINARY:
            image->num_bit = 1;
            break;
        case PNM_TYPE_PGM_ASCII:
        case PNM_TYPE_PGM_BINARY:
            image->num_bit = 8;
            break;
        case PNM_TYPE_PPM_ASCII:
        case PNM_TYPE_PPM_BINARY:
            image->num_bit = 24;
            break;
        default:
            break;
    }

    return read_pos;
}


/**
 * ファイルデータの次の文字列を数値化して取得する
 * 
 * 引数
 * value: 数値化した結果
 * file: ファイルデータの先頭アドレス
 * read_pos: 読み込み位置
 * file_size: ファイルデータのサイズ
 * 
 * 返却値
 * 成功: ファイルデータから読み込んだサイズ
 * 失敗: 0
 */
unsigned int getNextValue(unsigned int *value, char *data, unsigned int read_pos, unsigned int file_size) {
    char str[256];

    /* 空白系の文字やコメントを除いた次の文字列を取得する */
    unsigned int i, j, k;
    
    i = 0;
    while (read_pos + i < file_size) {
        /* 空白系の文字の場合は次の文字へスキップ */
        if (isspace(data[read_pos + i])) {
            i++;
            continue;
        }

        /* #ならそれ以降はコメントなので次の行へ */
        if (data[read_pos + i] == '#') {
            do {
                i++;
            } while (read_pos + i < file_size && data[read_pos + i] != '\n');

            /* \nの１文字文進める */
            i++;
        }

        break;
    }

    /* 文字列を取得 */
    j = 0;
    while (read_pos + i + j < file_size && !isspace(data[read_pos + i + j])) {
        /* 読み込んだバイト数をカウントアップ */
        j++;
    }

    /* 文字列を数字に変換 */
    for (k = 0; k < j; k++) {
        str[k] = data[read_pos + i + k];
    }
    str[k] = '\0';

    /* int化 */
    *value = (unsigned int)atoi(str);

    /* 読み込んだ文字数を返却 */
    return (i + j);
}


/**
 * P5ファイルのヘッダーと画像データをIMAGE構造体に格納する
 * 
 * 引数
 * image: 読み込んだ画像データ構造体
 * file_data: ファイルデータの先頭アドレス
 * file_size; ファイルデータのサイズ
 * 
 * 返却値
 * 成功: 0
 * 失敗: 0以外
 * 
 */
int readP5(IMAGE *image, char *file_data, unsigned int file_size) {
    unsigned int read_pos;
    unsigned int num_byte;
    unsigned int i, j, c;
    unsigned char byte_data;
    unsigned int color;
 
    /* ヘッダーを読み込んでImage構造体にデータをつめる */
    read_pos = getImageInfo(image, file_data, file_size, PNM_TYPE_PPM_BINARY);

    if (read_pos == 0) {
        /* ヘッダー読み込みに失敗していたら終了 */
        printf("ヘッダーがおかしいです\n");
        return -1;
    }

    /* ヘッダーの情報に基づいてメモリ確保 */
    if (allocImage(image) != 0) {
        printf("メモリ取得に失敗しました\n");
        return -1;
    }

    /* 最大輝度値の次にある空白系文字の分、読み込み位置を加算 */
    read_pos += 1;

    /* １ピクセルあたりの色の数を設定 */
    color = image->num_bit / 8;

    /* ファイル全体を読み終わるか必要な数分の輝度数をセットするまでループ */
    num_byte = 0;
    for (j = 0; j < image->height; j++) {
        for (i = 0; i < image->width; i++) {
            for (c = 0; c < color; c++) {

                /* 輝度値をIMAGE構造体に格納 */
                byte_data = (unsigned char)file_data[read_pos];
                image->data[num_byte] = byte_data;

                /* 格納したデータ数をインクリメント */
                num_byte += 1;

                /* データ読み込み位置と読み込んだデータ数を計算 */
                read_pos += 1;
                
                /* ファイルサイズ分読み込んでいたら終了 */
                if (read_pos >= file_size) {
                    return 0;
                }
            }
        }
    }

    return 0;
}

/**
 * バイトデータをencodeしてuint_image_dataに格納する
 * 
 * 今は引数がIMAGEになっているが、
 * 実際は unsigned char *image->data でもいい。
 * 
 */
void encodePnm(IMAGE *image, unsigned int data_size, std::vector<unsigned int> &uint_image_data) {
    // std::cout << image->data << std::endl;
    std::cout << "--- encode start --- " << std::endl;
    std::cout << "data_size: " << data_size << "\n";
    
    for (int i = 1; i < data_size + 1; i++)
    {
        unsigned int value = (unsigned int)image->data[i - 1];
        // std::cout << value << " ";
        uint_image_data[i-1] = value;
    }

    // std::cout << "\n";
    std::cout << "--- encode end --- " << "\n";
}

int main() {
  /* IP アドレス、ポート番号、ソケット */
  char destination[] = "127.0.0.1";
  unsigned short port = 8080;
  int dstSocket;

  /* sockaddr_in 構造体 */
  struct sockaddr_in dstAddr;

  /* 各種パラメータ */
  int status;
  int numsnt;
  char toSendText[] = "This is a test";

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

  /* 受信内容 */
  int file_size;
  char *file_data;
  IMAGE image;

  /* パケット送出 */
  for(int i=0; i<1; i++) {
    printf("sending...\n");
    send(dstSocket, toSendText, strlen(toSendText)+1, 0);
    Sleep(1000);

    // この後送られるファイルのサイズを受け取る
    recv(dstSocket, (char*)&file_size, sizeof(int), 0);
    printf("received: %d\n", file_size);

    // ファイルデータ読み込み用のメモリ確保してから受け取る
    file_data = (char*)malloc(sizeof(char) * file_size);    
    recv(dstSocket, file_data, file_size, 0);
    printf("%s\n", file_data);

    // 画像の読み込み
    readP5(&image, file_data, file_size);

    // 画像のエンコード
    std::cout << "w:" << image.width << ", h:" << image.height << std::endl;
    unsigned int data_size = image.width * image.height * 1;
    std::vector<unsigned int> uint_image_data(data_size);
    encodePnm(&image, data_size, uint_image_data);

    std::cout << "uint_image_data: " << uint_image_data.size() << "\n";
    for(int i = 0; i < uint_image_data.size(); i++){
        std::cout <<  uint_image_data[i] << " ";
    }
    std::cout << "\n";

    free(file_data);
  }

  /* Windows 独自の設定 */
  closesocket(dstSocket);
  WSACleanup();
}