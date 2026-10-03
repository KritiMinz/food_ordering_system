#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <csignal>
#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define ORDER_EVENT_PIPE "/tmp/food_order_events"
#define FOOD_DEVICE "/dev/food_order"
#define LOG_FILE "../../data/order_log.txt"

std::atomic<bool> running(true);

std::mutex log_mutex;


/*
 * Handle Ctrl+C / termination.
 */
void handle_signal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        running = false;
    }
}


/*
 * Send event to the Linux character device.
 */
void send_to_driver(const std::string& event)
{
    int fd = open(
        FOOD_DEVICE,
        O_WRONLY
    );

    if (fd == -1)
    {
        std::cerr
            << "Driver unavailable: "
            << FOOD_DEVICE
            << std::endl;

        return;
    }


    ssize_t result = write(
        fd,
        event.c_str(),
        event.size()
    );


    if (result == -1)
    {
        std::cerr
            << "Failed to write event to driver."
            << std::endl;
    }
    else
    {
        std::cout
            << "Driver event sent: "
            << event
            << std::endl;
    }


    close(fd);
}


/*
 * Write event to application log file.
 */
void write_log(const std::string& event)
{
    std::lock_guard<std::mutex> lock(log_mutex);


    int fd = open(
        LOG_FILE,
        O_WRONLY | O_CREAT | O_APPEND,
        0644
    );


    if (fd == -1)
    {
        std::cerr
            << "Failed to open log file."
            << std::endl;

        return;
    }


    std::string message =
        event + "\n";


    ssize_t result = write(
        fd,
        message.c_str(),
        message.size()
    );


    if (result == -1)
    {
        std::cerr
            << "Failed to write log event."
            << std::endl;
    }
    else
    {
        std::cout
            << "Logged: "
            << event
            << std::endl;
    }


    close(fd);


    /*
     * Forward the same event to the
     * Linux character device.
     */
    send_to_driver(event);
}


/*
 * Worker thread that receives events
 * from the named FIFO.
 */
void logger_worker(int pipe_fd)
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


            std::string event(buffer);


            write_log(event);
        }
        else if (bytes_read == 0)
        {
            /*
             * With O_RDWR this normally occurs only
             * when the FIFO is closed/reopened.
             */
            continue;
        }
        else
        {
            if (!running)
            {
                break;
            }

            continue;
        }
    }


    std::cout
        << "Logger worker stopped."
        << std::endl;
}


/*
 * Main program.
 */
int main()
{
    std::signal(
        SIGINT,
        handle_signal
    );

    std::signal(
        SIGTERM,
        handle_signal
    );


    std::cout
        << "===================================="
        << std::endl;

    std::cout
        << " Food Order Event Logger"
        << std::endl;

    std::cout
        << " Linux IPC + File I/O + Device Driver"
        << std::endl;

    std::cout
        << "===================================="
        << std::endl;


    std::cout
        << "Logger PID: "
        << getpid()
        << std::endl;


    /*
     * Remove an old FIFO if it exists.
     */
    unlink(ORDER_EVENT_PIPE);


    /*
     * Create a fresh named FIFO.
     */
    if (mkfifo(
            ORDER_EVENT_PIPE,
            0666
        ) == -1)
    {
        std::cerr
            << "Failed to create named pipe."
            << std::endl;

        return 1;
    }


    /*
     * Ensure the FIFO has usable permissions.
     */
    chmod(
        ORDER_EVENT_PIPE,
        0666
    );


    /*
     * Open FIFO using O_RDWR.
     *
     * This prevents the logger from blocking
     * before the first event sender connects.
     */
    int pipe_fd = open(
        ORDER_EVENT_PIPE,
        O_RDWR
    );


    if (pipe_fd == -1)
    {
        std::cerr
            << "Failed to open named pipe."
            << std::endl;

        unlink(ORDER_EVENT_PIPE);

        return 1;
    }


    std::cout
        << "IPC pipe: "
        << ORDER_EVENT_PIPE
        << std::endl;


    std::cout
        << "Linux device: "
        << FOOD_DEVICE
        << std::endl;


    std::cout
        << "Log file: "
        << LOG_FILE
        << std::endl;


    std::cout
        << "Logger is waiting for order events."
        << std::endl;


    /*
     * Start worker thread.
     */
    std::thread worker(
        logger_worker,
        pipe_fd
    );


    /*
     * Wait for worker thread.
     */
    worker.join();


    /*
     * Clean up.
     */
    close(pipe_fd);

    unlink(ORDER_EVENT_PIPE);


    std::cout
        << "Logger shutting down gracefully."
        << std::endl;


    return 0;
}
