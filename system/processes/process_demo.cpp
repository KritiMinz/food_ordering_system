#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    std::cout << "Parent process started." << std::endl;

    pid_t pid = fork();

    if (pid < 0)
    {
        std::cerr << "fork() failed." << std::endl;
        return 1;
    }

    if (pid == 0)
    {
        std::cout << "Child process running. PID: "
                  << getpid() << std::endl;

        std::cout << "Child process finished."
                  << std::endl;

        return 0;
    }

    std::cout << "Parent created child process. Child PID: "
              << pid << std::endl;

    wait(nullptr);

    std::cout << "Parent detected child completion."
              << std::endl;

    return 0;
}
