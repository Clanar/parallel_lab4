#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <chrono>

const int PORT = 1234;
const int ROWS = 4;
const int COLS = 3;
const int K = 2;

void fillMatrix(int *matrix, int size) {
    for (int i = 0; i < size; ++i) {
        matrix[i] = rand() % 11;
    }
}

int main() {
    while (true) {
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            std::cerr << "Error: Couldn't create socket\n";
            return 1;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_port = htons(PORT);
        
        if (connect(clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            std::cerr << "Error: Connection failed\n";
            close(clientSocket);
            return 1;
        }

        std::cout << "Connected to server\n";

        send(clientSocket, &ROWS, sizeof(ROWS), 0);
        send(clientSocket, &COLS, sizeof(COLS), 0);
        send(clientSocket, &K, sizeof(K), 0);

        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        srand(seed);
        
        int **A = new int*[ROWS];
        int **B = new int*[ROWS];
        for (int i = 0; i < ROWS; ++i) {
            A[i] = new int[COLS];
            B[i] = new int[COLS];
            fillMatrix(A[i], COLS);
            fillMatrix(B[i], COLS);
        }

        std::cout << "Generated random matrix A and B\n";

        for (int i = 0; i < ROWS; ++i) {
            send(clientSocket, A[i], sizeof(int) * COLS, 0);
            send(clientSocket, B[i], sizeof(int) * COLS, 0);
        }

        std::cout << "Sent matrix A and B to server\n";
        
        int **C = new int*[ROWS];
        for (int i = 0; i < ROWS; ++i) {
            C[i] = new int[COLS];
            recv(clientSocket, C[i], sizeof(int) * COLS, 0);
        }

        std::cout << "Received result matrix C from server:\n";
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                std::cout << C[i][j] << "\t";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

        close(clientSocket);

        for (int i = 0; i < ROWS; ++i) {
            delete[] A[i];
            delete[] B[i];
            delete[] C[i];
        }
        delete[] A;
        delete[] B;
        delete[] C;

        sleep(5);
    }

    return 0;
}
