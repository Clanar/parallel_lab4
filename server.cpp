#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

const int PORT = 1234;

struct ClientData {
    int socket;
};

void calculateMatrix(int **A, int **B, int **C, int rows, int cols, int k) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            C[i][j] = A[i][j] + k * B[i][j];
        }
    }
}

void* handleClient(void* arg) {
    ClientData* data = (ClientData*)arg;
    int clientSocket = data->socket;

    int rows, cols, k;
    if (recv(clientSocket, &rows, sizeof(rows), 0) <= 0 ||
        recv(clientSocket, &cols, sizeof(cols), 0) <= 0 ||
        recv(clientSocket, &k, sizeof(k), 0) <= 0) {
        std::cerr << "Error: Failed to receive data from client\n";
        close(clientSocket);
        delete data;
        return nullptr;
    }
    std::cout << "Received matrix sizes and k from client\n";

    int **A = new int*[rows];
    int **B = new int*[rows];
    for (int i = 0; i < rows; ++i) {
        A[i] = new int[cols];
        B[i] = new int[cols];
        if (recv(clientSocket, A[i], sizeof(int) * cols, 0) <= 0 ||
            recv(clientSocket, B[i], sizeof(int) * cols, 0) <= 0) {
            std::cerr << "Error: Failed to receive matrix from client\n";
            close(clientSocket);
            delete data;
            return nullptr;
        }
    }
    std::cout << "Received matrix A and B from client\n";

    int **C = new int*[rows];
    for (int i = 0; i < rows; ++i) {
        C[i] = new int[cols];
    }
    calculateMatrix(A, B, C, rows, cols, k);
    
    for (int i = 0; i < rows; ++i) {
        if (send(clientSocket, C[i], sizeof(int) * cols, 0) <= 0) {
            std::cerr << "Error: Failed to send result to client\n";
            close(clientSocket);
            delete data;
            return nullptr;
        }
    }

    std::cout << "Sent result matrix C to client\n\n";

    close(clientSocket);

    for (int i = 0; i < rows; ++i) {
        delete[] A[i];
        delete[] B[i];
        delete[] C[i];
    }
    delete[] A;
    delete[] B;
    delete[] C;

    delete data;

    return nullptr;
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error: Couldn't create socket\n";
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error: Bind failed\n";
        return 1;
    }

    if (listen(serverSocket, 3) == -1) {
        std::cerr << "Error: Listen failed\n";
        return 1;
    }
    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientLen);
        if (clientSocket == -1) {
            std::cerr << "Error: Accept failed\n";
            return 1;
        }
        std::cout << "Accepted incoming connection\n";

        ClientData* data = new ClientData;
        data->socket = clientSocket;
    
        pthread_t thread;
        if (pthread_create(&thread, nullptr, handleClient, (void*)data) != 0) {
            std::cerr << "Error: Failed to create thread\n";
            return 1;
        }

        pthread_detach(thread);
    }

    close(serverSocket);

    return 0;
}