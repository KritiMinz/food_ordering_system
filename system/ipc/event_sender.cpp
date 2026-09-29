#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

const char* ORDER_EVENT_PIPE =
    "/tmp/food_order_events";

int main()
{
    std::cout
        << "===================================="
        << std::endl;

    std::cout
        << " Food Order Event Sender"
        << std::endl;

    std::cout
        << " Linux Named Pipe IPC"
        << std::endl;

    std::cout
        << "===================================="
        << std::endl;


    /*
     * Check whether the named pipe exists.
     */
    if (access(
            ORDER_EVENT_PIPE,
            F_OK
        ) != 0)
    {
        std::cerr
            << "Logger is not running."
            << std::endl;

        std::cerr
            << "Start order_logger first."
            << std::endl;

        return 1;
    }


    /*
     * Open the named pipe for writing.
     */
    int pipe_fd = open(
        ORDER_EVENT_PIPE,
        O_WRONLY
    );

    if (pipe_fd == -1)
    {
        std::cerr
            << "Failed to open event pipe."
            << std::endl;

        return 1;
    }


    /*
     * Create a sample order event.
     */
    std::string event =
        "ORDER_PLACED | Order ID: 101 | User ID: 1 | Total: 320";


    /*
     * Send the event to the logger.
     */
    ssize_t bytes_written = write(
        pipe_fd,
        event.c_str(),
        event.size()
    );


    if (bytes_written == -1)
    {
        std::cerr
            << "Failed to send event."
            << std::endl;

        close(pipe_fd);

        return 1;
    }


    std::cout
        << "Event sent: "
        << event
        << std::endl;


    close(pipe_fd);


    std::cout
        << "IPC event transmission completed."
        << std::endl;


    return 0;
}
