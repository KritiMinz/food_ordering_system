#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

int main()
{
    const char* filename = "data/order_log.txt";

    const char* message =
        "ORDER_PLACED: Order #101\n";

    int fd = open(
        filename,
        O_WRONLY | O_CREAT | O_APPEND,
        0644
    );

    if (fd == -1)
    {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    ssize_t bytes_written = write(
        fd,
        message,
        strlen(message)
    );

    if (bytes_written == -1)
    {
        std::cerr << "Failed to write to file." << std::endl;
        close(fd);
        return 1;
    }

    close(fd);

    std::cout << "Order event written to "
              << filename << std::endl;

    return 0;
}
