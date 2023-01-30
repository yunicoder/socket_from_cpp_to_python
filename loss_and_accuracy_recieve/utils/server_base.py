"""
    参考: https://qiita.com/t_katsumura/items/a83431671a41d9b6358f
"""
import socket
from typing import Tuple


class BaseServer:
    """socket通信を使用したサーバー処理 (継承して使用する)"""

    def __init__(self, timeout: int = 60, buffer: int = 1024):
        self.__socket = None
        self.__timeout = timeout
        self.__buffer = buffer
        self.close()

    def __del__(self):
        self.close()  # 破棄タイミングでsocketをcloseする

    def close(self) -> None:
        """socketのclose"""
        try:
            self.__socket.shutdown(socket.SHUT_RDWR)
            self.__socket.close()
        except:
            pass

    def accept(self, address: Tuple[str, int], socket_family: int, socket_type: int, proto: int) -> None:
        """clientとのやり取り

        Flow:
            socket() => bind() => listen() => accept() => receive()/send() => close()

        Args:
            address (Tuple[str, int]): アドレス(ホストipアドレス,ポート番号) ex.) ("0.0.0.0", 8080)
            socket_family (str): ソケットにバインドするアドレスの種類 ex.) AF_INET
            socket_type (int): ソケットタイプ ex.) SOCK_STREAM
        """
        self.__socket = socket.socket(socket_family, socket_type, proto)
        self.__socket.settimeout(self.__timeout)
        self.__socket.bind(address)  # IPとPORTをバインド
        self.__socket.listen(30)  # キューの最大値 = エポック数

        print("Waiting for connections...")
        conn, _ = self.__socket.accept()

        while True:
            try:
                message_recv = conn.recv(self.__buffer).decode("utf-8")
                message_resp = self.respond(message_recv)
                conn.send(message_resp.encode("utf-8"))
            except ConnectionResetError:
                break
            except BrokenPipeError:
                break
        self.close()

    def respond(self, message: str) -> str:
        """サーバー側の処理 (継承後のクラスで自分で実装する)"""
        return message
