#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct {
    int return_code;
    int crash;
} Return_exec;

const int PORT = 8080; //i think the address is enough to type in

Return_exec run_command(std::string command, std::vector <std::string> command_w_args) {

    std::vector <char*> command_w_args_c;

    Return_exec ret_exc;



    pid_t pid = fork();


    if (pid < 0) {
        perror("failed to fork");

    } else if (pid == 0) {
        command_w_args_c.reserve(command_w_args.size() + 1);
        for(auto& s: command_w_args) command_w_args_c.push_back(&s[0]);
        command_w_args_c.push_back(nullptr);

        execvp(command.data(), command_w_args_c.data());
        perror("failure while execvp");
        exit(EXIT_FAILURE);
    } else {
        int status;


        waitpid(pid, &status, 0);
        ret_exc.crash = 0;
        if (WIFEXITED(status)) {
            ret_exc.return_code = WEXITSTATUS(status);

        } else if (WIFSIGNALED(status)) {
            ret_exc.crash = 1;
        }
    }

return ret_exc;
}

int main(int argc, char **argv) {

    std::string ip4_address = "";
    std::string command;
    std::vector <std::string> command_w_args;
    if (argc > 2) {
    ip4_address = argv[1];

    command = argv[2];
    // why do we need this ?  command_w_args[0] = argv[2];
    } else {
        printf("May you yap more \n");
        return 1;
    }
    for (int i = 2; i < argc; i++) {
        command_w_args.push_back(argv[i]);
    }
    int socket_to_send_to = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_to_send_to < 0) {
        perror("Socket failed to create ");
        return 1;
    }

    struct sockaddr_in address_server;
    std::memset(&address_server, 0, sizeof(address_server));
    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip4_address.data(), &address_server.sin_addr) <= 0) {
        perror("Invalid IP ");
        close(socket_to_send_to);
        return 1;
    };
    if (connect(socket_to_send_to, (struct sockaddr*)&address_server, sizeof(address_server))) {
        perror("Could not connect to device ");
        close(socket_to_send_to);
        return 1;

    }

    Return_exec return_val;
    return_val.crash = -1; //like that we know if something went wrong
    return_val= run_command(command, command_w_args);;
    std::string msg;
    msg = std::to_string(return_val.return_code) + ";" + std::to_string(return_val.crash) + "\n";
    int transmitted = send(socket_to_send_to, msg.c_str(), msg.length(), 0);
    if (transmitted < 0) {
        printf("could not send \n");
    }

    if (return_val.crash < 0) {
        perror("wtf just happened ");
        return 1;
    }

    printf("Return code : %d \n", return_val.return_code);
    close(socket_to_send_to);
    return 0;
}
