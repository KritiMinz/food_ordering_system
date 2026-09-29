#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <csignal>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

const char* ORDER_EVENT_PIPE =
    "/tmp/food_order_events";

std::atomic<bool> running(true);
std::mutex log_mutex;

int pipe_fd = -1;

void handle_signal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        running = false;

        if (pipe_fd != -1)
        {
            close(pipe_fd);
            pipe_fd = -1;
        }
    }
}

void write_log(const std::string& event)
{
    std::lock_guard<std::mutex> lock(log_mutex);

    int fd = open(
        "data/order_log.txt",
        O_WRONLY | O_CREAT | O_APPEND,
        0644
    );

    if (fd == -1)
    {
        std::cerr << "Failed to open order log."
                  << std::endl;
        return;
    }

    std::string message = event + "\n";

    ssize_t result = write(
        fd,
        message.c_str(),
        message.size()
    );

    if (result == -1)
    {
        std::cerr << "Failed to write order event."
                  << std::endl;
    }

    close(fd);

    std::cout << "Logged: "
              << event << std::endl;
}

void logger_worker()
{
    char buffer[1024];

    while (running)
    {
        ssize_t bytes_read = read(
            pipe_fd,
            buffer,
            sizeof(buffer) - 1
        );

        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';

            write_log(
                std::string(buffer)
            );
        }
        else if (bytes_read == 0)
        {
            // No backend writer is currently connected.
            if (!running)
            {
                break;
            }

            close(pipe_fd);

            pipe_fd = open(
                ORDER_EVENT_PIPE,
                O_RDONLY
            );

            if (pipe_fd == -1)
            {
                sleep(1);
            }
        }
        else
        {
            if (!running)
            {
                break;
            }
        }
    }

    std::cout << "Logger worker stopped."
              << std::endl;
}

int main()
{
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    std::cout << "===================================="
              << std::endl;

    std::cout << " Food Order Event Logger"
              << std::endl;

    std::cout << " Persistent Linux IPC Logger"
              << std::endl;

    std::cout << "===================================="
              << std::endl;

    std::cout << "Logger PID: "
              << getpid()
              << std::endl;

    // Create the named pipe if it doesn't exist.
    if (mkfifo(
            ORDER_EVENT_PIPE,
            0666
        ) == -1)
    {
        // It may already exist.
        if (access(
                ORDER_EVENT_PIPE,
                F_OK
            ) != 0)
        {
            std::cerr << "Failed to create named pipe."
                      << std::endl;

            return 1;
        }
    }

    pipe_fd = open(
        ORDER_EVENT_PIPE,
        O_RDONLY
    );

    if (pipe_fd == -1)
    {
        std::cerr << "Failed to open named pipe."
                  << std::endl;

        return 1;
    }

    std::cout << "IPC pipe: "
              << ORDER_EVENT_PIPE
              << std::endl;

    std::cout << "Logger is waiting for order events."
              << std::endl;

    std::thread worker(logger_worker);

    worker.join();

    if (pipe_fd != -1)
    {
        close(pipe_fd);
    }

    unlink(ORDER_EVENT_PIPE);

    std::cout << "Logger shutting down gracefully."
              << std::endl;

    return 0;
}
