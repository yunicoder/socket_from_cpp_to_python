import socket

from utils.server_base import BaseServer


class InetServer(BaseServer):
    def __init__(self, host: str = "0.0.0.0", port: int = 8080) -> None:
        self.address = (host, port)
        super().__init__(timeout=600000, buffer=1024)  # 親クラスのinit
        self.accept(self.address, socket.AF_INET, socket.SOCK_STREAM, 0)

    def start_server(self):
        print("Server started :", (self.address))
        while True:
            self.accept(self.address, socket.AF_INET, socket.SOCK_STREAM, 0)

    def respond(self, message: str) -> str:
        """データ受け取ってからのサーバー側の処理"""
        print("received -> ", message)
        return "Server accepted !!"


if __name__ == "__main__":
    server = InetServer(host="0.0.0.0", port=8080)
    server.start_server()
