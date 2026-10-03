# Food Ordering System

A Linux-based Food Ordering System developed as a capstone project using C/C++, Linux system programming, a Linux character device driver, a C++ backend, React frontend, and JSON file-based storage.

The project combines application-level software architecture with Linux system programming concepts such as Inter-Process Communication (IPC), file I/O, processes, threads, signals, synchronization, and character device drivers.

---

## 1. Project Overview

The Food Ordering System is a software architecture project that provides an online food ordering platform with separate customer and administrator functionality.

Customers can:

* Register and log in
* Browse food items
* Search and filter food
* View food details
* Manage favorites
* Manage delivery addresses
* Add items to a shopping cart
* Apply discount coupons
* Checkout and place orders
* Select payment methods
* View order history
* Track order status
* Cancel eligible orders
* View their profile

Administrators can:

* Log in through the administrator account
* Access the admin dashboard
* Manage food items
* Manage food availability
* View and manage customers
* View and manage orders
* Update order status
* Create and manage discount coupons
* Activate or deactivate coupons
* View earnings
* View the administrator profile
* View system information

The project also contains Linux system programming components and a custom Linux character device driver for processing food order events.

---

## 2. Project Objectives

The main objectives of the project are:

* Develop a complete food ordering application.
* Apply C/C++ programming concepts.
* Implement a C++ backend service.
* Implement a Linux character device driver using C.
* Use Linux as the development and execution environment.
* Implement persistent storage using JSON files.
* Demonstrate Linux Inter-Process Communication using a named FIFO.
* Demonstrate file I/O, processes, threads, signals, and synchronization.
* Integrate application-level order events with Linux system components.
* Provide separate customer and administrator functionality.
* Maintain the complete project using Git and GitHub.

---

## 3. Technology Stack

| Component           | Technology     |
| ------------------- | -------------- |
| Operating System    | Linux / Ubuntu |
| Backend             | C++            |
| Linux Device Driver | C              |
| HTTP Server         | Crow           |
| JSON Processing     | nlohmann/json  |
| Frontend            | React          |
| Frontend Build Tool | Vite           |
| Styling             | Tailwind CSS   |
| Data Storage        | JSON files     |
| IPC                 | Named FIFO     |
| Build System        | GNU Make       |
| Version Control     | Git            |
| Repository          | GitHub         |

---

## 4. System Architecture

The project follows a layered client-server architecture.

```text
                    +-------------------------+
                    |      React Frontend     |
                    |    Customer / Admin UI  |
                    +------------+------------+
                                 |
                                 | HTTP / REST
                                 v
                    +-------------------------+
                    |       C++ Backend       |
                    |       Crow Server       |
                    +------------+------------+
                                 |
                   +-------------+-------------+
                   |                           |
                   v                           v
          +------------------+       +----------------------+
          |   JSON Storage   |       | Linux System         |
          |   data/*.json    |       | Programming          |
          +------------------+       +----------+-----------+
                                                |
                                                v
                                     +----------------------+
                                     |    Named FIFO / IPC  |
                                     +----------+-----------+
                                                |
                                                v
                                     +----------------------+
                                     |     Order Logger     |
                                     +----------+-----------+
                                                |
                                                v
                                     +----------------------+
                                     | Linux Character      |
                                     | Device Driver        |
                                     | /dev/food_order      |
                                     +----------+-----------+
                                                |
                                                v
                                         Linux Kernel
```

### Main Data Flow

```text
Customer
   |
   v
React Frontend
   |
   | HTTP Request
   v
C++ Backend
   |
   +----> JSON Files
   |
   +----> Order Event
             |
             v
       Named FIFO
             |
             v
       Order Logger
          /     \
         /       \
        v         v
 order_log.txt  /dev/food_order
                    |
                    v
              Linux Driver
```

---

## 5. Main Features

### Customer Features

* Customer registration
* Customer login/logout
* Authentication
* Food menu browsing
* Food search
* Category filtering
* Food details
* Favorites management
* Shopping cart
* Address management
* Discount coupon application
* Checkout
* Multiple payment methods
* Order placement
* Order history
* Order tracking
* Order cancellation
* Customer profile

### Administrator Features

* Administrator login
* Administrator dashboard
* Administrator profile
* Food management
* Add food items
* Edit food items
* Delete food items
* Food availability management
* Customer management
* Order management
* Order status management
* Discount coupon management
* Coupon activation/deactivation
* Earnings tracking
* System information

### Linux/System Programming Features

* Linux character device driver
* Character device `/dev/food_order`
* Named FIFO IPC
* Order event logger
* File-based event logging
* POSIX file I/O
* Processes
* Threads
* Mutex synchronization
* Signal handling
* Kernel logging

---

## 6. Project Structure

```text
food_ordering_system/
│
├── backend/
│   ├── Makefile
│   ├── crow/
│   └── src/
│       └── main.cpp
│
├── driver/
│   ├── Makefile
│   └── food_order_driver.c
│
├── frontend/
│   ├── public/
│   ├── src/
│   │   ├── App.jsx
│   │   ├── App.css
│   │   ├── index.css
│   │   ├── main.jsx
│   │   └── components/
│   │       ├── AddressManager.jsx
│   │       ├── AdminDashboard.jsx
│   │       ├── AuthModal.jsx
│   │       ├── Cart.jsx
│   │       ├── Checkout.jsx
│   │       ├── Favorites.jsx
│   │       ├── FoodCard.jsx
│   │       ├── FoodDetails.jsx
│   │       ├── MyOrders.jsx
│   │       ├── Navbar.jsx
│   │       ├── Orders.jsx
│   │       └── Profile.jsx
│   ├── package.json
│   ├── package-lock.json
│   └── vite.config.js
│
├── data/
│   ├── addresses.json
│   ├── coupons.json
│   ├── earnings.json
│   ├── favorites.json
│   ├── foods.json
│   ├── orders.json
│   ├── reviews.json
│   └── users.json
│
├── system/
│   ├── file_io/
│   │   └── file_io_demo.cpp
│   │
│   ├── ipc/
│   │   ├── event_sender.cpp
│   │   ├── ipc_demo.cpp
│   │   └── order_logger.cpp
│   │
│   ├── processes/
│   │   └── process_demo.cpp
│   │
│   ├── signals/
│   │   └── signal_demo.cpp
│   │
│   └── threads/
│       └── thread_demo.cpp
│
├── .gitignore
├── README.md
└── ...
```

---

## 7. Data Storage

The project uses JSON files for persistent application storage.

The main data files are:

```text
data/
├── addresses.json
├── coupons.json
├── earnings.json
├── favorites.json
├── foods.json
├── orders.json
├── reviews.json
└── users.json
```

The C++ backend reads and updates these files during application operation.

The project does not require MySQL for its current file-based storage implementation.

---

## 8. Linux Character Device Driver

The project contains a custom Linux character device driver written in C.

### Driver Source

```text
driver/food_order_driver.c
```

The driver creates the device:

```text
/dev/food_order
```

The driver demonstrates:

* Character device registration
* Dynamic device number allocation
* Device class creation
* Device creation
* Device initialization
* Device cleanup
* `open()`
* `read()`
* `write()`
* `release()`
* Kernel logging
* User-space and kernel-space data transfer
* `copy_to_user()`
* `copy_from_user()`
* Mutex synchronization

### Build the Driver

From the project root:

```bash
cd driver
make
```

### Load the Driver

```bash
sudo insmod food_order_driver.ko
```

### Verify the Driver

```bash
lsmod | grep food_order
```

Check the device:

```bash
ls -l /dev/food_order
```

### View Kernel Messages

```bash
sudo dmesg | tail -20
```

### Remove the Driver

```bash
sudo rmmod food_order_driver
```

---

## 9. Linux IPC and Order Event Logger

The project contains an order event logger demonstrating Linux Inter-Process Communication.

### Logger Source

```text
system/ipc/order_logger.cpp
```

The system uses a named FIFO:

```text
/tmp/food_order_events
```

The event flow is:

```text
C++ Backend
     |
     | Order Event
     v
Named FIFO
     |
     v
Order Logger
     |
     +------> data/order_log.txt
     |
     +------> /dev/food_order
                    |
                    v
             Linux Character Driver
```

The IPC component demonstrates concepts including:

* Named FIFO
* `mkfifo()`
* `open()`
* `read()`
* `write()`
* `close()`
* `unlink()`
* File I/O
* Threads
* Mutex synchronization
* Signal handling

---

## 10. Event Logging

Order events can be recorded in:

```text
data/order_log.txt
```

Example events:

```text
ORDER_PLACED | Order ID: 16 | User ID: 3 | Total: 150
ORDER_CANCELLED | Order ID: 14 | User ID: 3
```

The event logger receives order events through the named FIFO and forwards relevant events to the Linux character device driver.

---

## 11. Backend

The backend is implemented in C++ using the Crow HTTP framework.

### Main Source

```text
backend/src/main.cpp
```

The backend provides APIs for:

* User registration
* User authentication
* Food management
* Food availability
* Orders
* Order status
* Customer management
* Coupons
* Earnings
* Favorites
* Addresses
* Reviews
* Health checking

### Backend URL

```text
http://localhost:18080
```

### Build Backend

From the project root:

```bash
make -C backend
```

### Start Backend

```bash
./backend/build/food_backend
```

### Health Check

```bash
curl http://localhost:18080/api/health
```

Expected response:

```json
{
  "status": "success",
  "message": "Food Ordering System backend is running"
}
```

### Food API

```bash
curl http://localhost:18080/api/foods
```

---

## 12. Frontend

The frontend is implemented using React and Vite.

### Frontend Directory

```text
frontend/
```

### Install Dependencies

```bash
cd ~/food_ordering_system/frontend
npm install
```

### Start Development Server

```bash
npm run dev
```

The Vite development server normally runs at:

```text
http://localhost:5173
```

The frontend communicates with the C++ backend through HTTP APIs.

---

## 13. Running the Complete Project

The complete system can be run using multiple terminals.

### Terminal 1 — Linux Character Driver

```bash
cd ~/food_ordering_system/driver
make
sudo insmod food_order_driver.ko
```

Verify:

```bash
lsmod | grep food_order
ls -l /dev/food_order
```

### Terminal 2 — C++ Backend

```bash
cd ~/food_ordering_system
./backend/build/food_backend
```

### Terminal 3 — Order Event Logger

```bash
cd ~/food_ordering_system/system/ipc
./order_logger
```

### Terminal 4 — React Frontend

```bash
cd ~/food_ordering_system/frontend
npm run dev
```

Then open:

```text
http://localhost:5173
```

---

## 14. Requirements

### Operating System

The project is designed to run on Linux.

Recommended environment:

```text
Ubuntu Linux
```

The project can also be developed and executed inside an Ubuntu VirtualBox virtual machine.

### Required Software

* GCC
* G++
* GNU Make
* Node.js
* npm
* Git
* Linux kernel headers

### Hardware

Recommended:

* Dual-core processor or better
* 4 GB RAM or more
* At least 5 GB free disk space

---

## 15. Authentication

The application supports customer and administrator authentication.

Authentication uses bearer tokens for protected API requests.

Example:

```text
Authorization: Bearer <token>
```

The system supports separate customer and administrator roles.

---

## 16. Testing Performed

The following application features have been tested:

### Customer Testing

* Customer registration
* Customer login
* Food browsing
* Food searching
* Category filtering
* Food details
* Favorites
* Cart operations
* Address management
* Coupon application
* Checkout
* Payment selection
* Order placement
* Order history
* Order tracking
* Order cancellation
* Customer profile

### Administrator Testing

* Administrator login
* Admin dashboard
* Food management
* Food availability
* Customer management
* Order management
* Order status updates
* Coupon creation
* Coupon activation/deactivation
* Earnings tracking
* Administrator profile

### Linux Testing

* Backend health endpoint
* JSON persistence
* Named FIFO IPC
* Order event logger
* File I/O
* Linux character device driver
* Device creation
* Kernel logging
* Driver event handling

Driver verification:

```bash
lsmod | grep food_order
```

Device verification:

```bash
ls -l /dev/food_order
```

Kernel log verification:

```bash
sudo dmesg | tail -20
```

---

## 17. Example Linux Driver Event

An order event can be passed through the IPC system in a format such as:

```text
ORDER_PLACED | Order ID: 16 | User ID: 3 | Total: 150
```

The Linux driver can record the event through the kernel logging system.

Example:

```text
food_order: device opened
food_order: received event: ORDER_PLACED | Order ID: 16 | User ID: 3 | Total: 150
food_order: device closed
```

---

## 18. GitHub Repository

The complete project is maintained using Git and GitHub.

Repository:

```text
https://github.com/KritiMinz/food_ordering_system
```

The repository contains:

* C++ backend source
* C Linux character device driver
* Linux system programming components
* React frontend
* JSON storage
* Makefiles
* README documentation
* Project configuration files

Generated build files and unnecessary runtime artifacts are excluded through `.gitignore`.

### Git Workflow

```bash
git status
git add .
git commit -m "Finalize Food Ordering System capstone project"
git push origin main
```

---

## 19. Academic Concepts Demonstrated

### C/C++ Programming

* C++ classes and objects
* Functions
* Data structures
* STL
* File handling
* JSON processing
* Error handling
* Multithreading
* Mutex synchronization

### Linux System Programming

* Processes
* Threads
* Signals
* File descriptors
* File I/O
* Named pipes
* Inter-Process Communication
* Synchronization

### Linux Device Drivers

* Linux kernel modules
* Character devices
* Device registration
* Major/minor device numbers
* `struct file_operations`
* `open()`
* `read()`
* `write()`
* `release()`
* `copy_to_user()`
* `copy_from_user()`
* Kernel logging
* Device creation and cleanup
* Mutex protection

### Software Architecture

* Client-server architecture
* Layered architecture
* REST-style APIs
* Frontend/backend separation
* File-based persistence
* Event-driven communication
* Modular software components

---

## 20. Future Enhancements

Possible future improvements include:

* Production-grade password hashing
* Improved authentication security
* JWT-based authentication
* Real payment gateway integration
* Real-time order notifications
* Delivery partner functionality
* Advanced analytics and reporting
* Automated testing
* Production Linux deployment
* Additional Linux driver functionality

---

## 21. Conclusion

The Food Ordering System combines a practical food-ordering application with C/C++ programming and Linux system programming.

The project demonstrates communication between a React frontend and C++ backend, persistent JSON-based storage, Linux IPC using a named FIFO, file I/O, processes, threads, signals, synchronization, event logging, and a custom Linux character device driver.

The project provides practical implementation of software architecture concepts together with low-level Linux programming and device-driver concepts.

