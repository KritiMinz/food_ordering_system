#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

int main()
{
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1)
    {
        std::cerr << "pipe() failed." << std::endl;
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        std::cerr << "fork() failed." << std::endl;
        return 1;
    }

    if (pid == 0)
    {
        close(pipe_fd[1]);

        char buffer[256] = {};

        ssize_t bytes_read = read(
            pipe_fd[0],
            buffer,
            sizeof(buffer) - 1
        );

        if (bytes_read > 0)
        {
            std::cout << "Child received message: "
                      << buffer << std::endl;
        }

        close(pipe_fd[0]);

        return 0;
    }

    close(pipe_fd[0]);

    const char* message =
        "ORDER_PLACED: Order #101";

    write(
        pipe_fd[1],
        message,
        strlen(message) + 1
    );

    std::cout << "Parent sent message: "
              << message << std::endl;

    close(pipe_fd[1]);

    wait(nullptr);

    std::cout << "IPC communication completed."
              << std::endl;

    return 0;
}
