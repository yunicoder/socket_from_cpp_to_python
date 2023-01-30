import socket
from pathlib import Path
from typing import List

from utils.server_base import BaseServer


class InetServer(BaseServer):
    def __init__(self, host: str = "0.0.0.0", port: int = 8080) -> None:
        self.address = (host, port)
        super().__init__(timeout=600000, buffer=1024)  # 親クラスのinit
        self.accept(self.address, socket.AF_INET, socket.SOCK_STREAM, 0)
        
        # 保存先のパス
        self.save_file = Path("./data/train_data.csv")
        if self.save_file.exists():
            self.save_file = Path("./data/train_data_stash.csv")
        self._initial_save_file()
    
    def _initial_save_file(self):
        """ヘッダ情報を書き込む"""
        with open(self.save_file, mode='w') as f:
            f.write("loss,accuracy\n")

    def start_server(self):
        print("Server started :", (self.address))
        while True:
            self.accept(self.address, socket.AF_INET, socket.SOCK_STREAM, 0)

    def respond(self, message: str) -> str:
        """データ受け取ってからのサーバー側の処理"""
        print("received -> ", message)
        self.save_recieved_data(message)
        return "Server accepted !!"
    
    def save_recieved_data(self, message: str):
        with open(self.save_file, mode='a') as f:
            f.write(message)
            f.write("\n")

if __name__ == "__main__":
    server = InetServer(host="0.0.0.0", port=8080)
    server.start_server()
