#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <string>

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
        // Child becomes the logger process.
        close(pipe_fd[1]);

        std::string fd = std::to_string(pipe_fd[0]);

        execl(
            "./system/ipc/order_logger",
            "./system/ipc/order_logger",
            fd.c_str(),
            static_cast<char*>(nullptr)
        );

        std::cerr << "Failed to start logger." << std::endl;
        return 1;
    }

    // Parent acts as the food-order backend/event producer.
    close(pipe_fd[0]);

    std::cout << "Event sender started." << std::endl;
    std::cout << "Logger process PID: "
              << pid << std::endl;

    const char* event =
        "ORDER_PLACED | Order ID: 101 | User ID: 1 | Total: 320";

    ssize_t bytes_written = write(
        pipe_fd[1],
        event,
        strlen(event)
    );

    if (bytes_written == -1)
    {
        std::cerr << "Failed to send event." << std::endl;
        close(pipe_fd[1]);
        waitpid(pid, nullptr, 0);
        return 1;
    }

    std::cout << "Event sent: "
              << event << std::endl;

    // Closing the write end tells the logger there are no more events.
    close(pipe_fd[1]);

    waitpid(pid, nullptr, 0);

    std::cout << "Logger process completed." << std::endl;
    std::cout << "IPC event processing completed." << std::endl;

    return 0;
}
