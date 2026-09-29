#include <iostream>
#include <thread>
#include <chrono>

void worker(int id)
{
    std::cout << "Worker thread " << id
              << " started. Thread ID: "
              << std::this_thread::get_id() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "Worker thread " << id
              << " finished." << std::endl;
}

int main()
{
    std::cout << "Main thread started." << std::endl;

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);

    t1.join();
    t2.join();

    std::cout << "All worker threads completed."
              << std::endl;

    return 0;
}
