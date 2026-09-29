#include <iostream>
#include <csignal>
#include <unistd.h>

volatile sig_atomic_t running = 1;

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        running = 0;
    }
}

int main()
{
    std::signal(SIGINT, handle_signal);

    std::cout << "Signal handling demo started." << std::endl;
    std::cout << "PID: " << getpid() << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    while (running)
    {
        std::cout << "Program is running..." << std::endl;
        sleep(2);
    }

    std::cout << "SIGINT received." << std::endl;
    std::cout << "Graceful shutdown completed." << std::endl;

    return 0;
}
