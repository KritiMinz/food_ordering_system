#include "crow.h"
#include "crow/middlewares/cors.h"
#include <nlohmann/json.hpp>

#include <openssl/sha.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using json = nlohmann::json;

const std::string FOOD_FILE = "data/foods.json";
const std::string USER_FILE = "data/users.json";

std::mutex userFileMutex;
std::mutex sessionMutex;

std::unordered_map<std::string, int> sessions;

// ============================================================
// Linux IPC - Send order event to logger
// ============================================================

const char* ORDER_EVENT_PIPE =
    "/tmp/food_order_events";

void sendOrderEvent(const std::string& event)
{
    int fd = open(
        ORDER_EVENT_PIPE,
        O_WRONLY | O_NONBLOCK
    );

    if (fd == -1)
    {
        // Logger may not be running.
        // Do not stop the customer's order because of logging.
        return;
    }

    write(
        fd,
        event.c_str(),
        event.size()
    );

    close(fd);
}


// ============================================================
// SHA-256 password hashing
// ============================================================

std::string hashPassword(const std::string& password)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(
        reinterpret_cast<const unsigned char*>(password.c_str()),
        password.size(),
        hash
    );

    std::stringstream stream;

    for (unsigned char value : hash)
    {
        stream << std::hex
               << std::setw(2)
               << std::setfill('0')
               << static_cast<int>(value);
    }

    return stream.str();
}


// ============================================================
// Load JSON file
// ============================================================

json loadJsonFile(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        return json::array();
    }

    try
    {
        json data;
        file >> data;
        return data;
    }
    catch (...)
    {
        return json::array();
    }
}


// ============================================================
// Save JSON file
// ============================================================

bool saveJsonFile(
    const std::string& filename,
    const json& data
)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    file << data.dump(4);

    return true;
}


// ============================================================
// Generate simple session token
// ============================================================

std::string generateToken()
{
    static std::random_device randomDevice;
    static std::mt19937 generator(randomDevice());

    std::uniform_int_distribution<unsigned long long> distribution;

    unsigned long long value = distribution(generator);

    std::stringstream stream;
    stream << std::hex << value;

    return stream.str();
}


// ============================================================
// Load food data
// ============================================================

json loadFoods()
{
    std::ifstream file(FOOD_FILE);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open "
                  << FOOD_FILE << std::endl;

        return json::array();
    }

    try
    {
        json foods;
        file >> foods;

        return foods;
    }
    catch (const json::parse_error& error)
    {
        std::cerr << "Error parsing food JSON: "
                  << error.what() << std::endl;

        return json::array();
    }
}


int main()
{
    crow::App<crow::CORSHandler> app;

    // ========================================================
    // CORS
    // ========================================================

    auto& cors = app.get_middleware<crow::CORSHandler>();

    cors.global()
        .origin("http://localhost:5173")
        .methods(
            "GET"_method,
            "POST"_method,
            "PUT"_method,
            "DELETE"_method
        )
        .headers("Content-Type");


    // ========================================================
    // Health check
    // ========================================================

    CROW_ROUTE(app, "/api/health")
    ([]()
    {
        crow::json::wvalue response;

        response["status"] = "success";
        response["message"] =
            "Food Ordering System backend is running";

        return response;
    });


    // ========================================================
    // Application information
    // ========================================================

    CROW_ROUTE(app, "/api")
    ([]()
    {
        crow::json::wvalue response;

        response["application"] =
            "Food Ordering System";

        response["backend"] = "C++";

        response["version"] = "1.0";

        return response;
    });


    // ========================================================
    // FOOD API
    // ========================================================

    CROW_ROUTE(app, "/api/foods")
    ([]()
    {
        json foods = loadFoods();

        crow::json::wvalue response =
            crow::json::load(foods.dump());

        return response;
    });

    // ========================================================
// ADMIN - ADD FOOD
// ========================================================

CROW_ROUTE(app, "/api/admin/foods")
    .methods("POST"_method)
([](const crow::request& request)
{
    try
    {
        json requestData = json::parse(request.body);

        std::string name =
            requestData.value("name", "");

        std::string description =
            requestData.value("description", "");

        std::string category =
            requestData.value("category", "");

        std::string image =
            requestData.value("image", "🍽️");

        double price =
            requestData.value("price", 0.0);

        if (name.empty() ||
            description.empty() ||
            category.empty() ||
            price <= 0)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Invalid food data"})"
            );
        }

        std::lock_guard<std::mutex> lock(userFileMutex);

        json foods =
            loadJsonFile("data/foods.json");

        int newId = 1;

        for (const auto& food : foods)
        {
            if (food.contains("id"))
            {
                newId =
                    std::max(
                        newId,
                        food["id"].get<int>() + 1
                    );
            }
        }

        json newFood = {
            {"id", newId},
            {"name", name},
            {"description", description},
            {"price", price},
            {"category", category},
            {"image", image}
        };

        foods.push_back(newFood);

        saveJsonFile(
            "data/foods.json",
            foods
        );

        crow::response response(201);

        response.set_header(
            "Content-Type",
            "application/json"
        );

        response.write(
            json({
                {"status", "success"},
                {"message", "Food added successfully"},
                {"food", newFood}
            }).dump()
        );

        return response;
    }
    catch (...)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid request"})"
        );
    }
});


// ========================================================
// ADMIN - UPDATE FOOD
// ========================================================

CROW_ROUTE(app, "/api/admin/foods/<int>")
    .methods("PUT"_method)
([](const crow::request& request, int foodId)
{
    try
    {
        json requestData =
            json::parse(request.body);

        std::lock_guard<std::mutex> lock(userFileMutex);

        json foods =
            loadJsonFile("data/foods.json");

        bool found = false;
        json updatedFood;

        for (auto& food : foods)
        {
            if (
                food.contains("id") &&
                food["id"].get<int>() == foodId
            )
            {
                if (requestData.contains("name"))
                    food["name"] = requestData["name"];

                if (requestData.contains("description"))
                    food["description"] =
                        requestData["description"];

                if (requestData.contains("price"))
                    food["price"] =
                        requestData["price"];

                if (requestData.contains("category"))
                    food["category"] =
                        requestData["category"];

                if (requestData.contains("image"))
                    food["image"] =
                        requestData["image"];

                updatedFood = food;
                found = true;

                break;
            }
        }

        if (!found)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Food not found"})"
            );
        }

        saveJsonFile(
            "data/foods.json",
            foods
        );

        crow::response response(200);

        response.set_header(
            "Content-Type",
            "application/json"
        );

        response.write(
            json({
                {"status", "success"},
                {"message", "Food updated successfully"},
                {"food", updatedFood}
            }).dump()
        );

        return response;
    }
    catch (...)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid request"})"
        );
    }
});


// ========================================================
// ADMIN - DELETE FOOD
// ========================================================

CROW_ROUTE(app, "/api/admin/foods/<int>")
    .methods("DELETE"_method)
([](const crow::request&, int foodId)
{
    try
    {
        std::lock_guard<std::mutex> lock(userFileMutex);

        json foods =
            loadJsonFile("data/foods.json");

        json updatedFoods =
            json::array();

        bool found = false;

        for (const auto& food : foods)
        {
            if (
                food.contains("id") &&
                food["id"].get<int>() == foodId
            )
            {
                found = true;
                continue;
            }

            updatedFoods.push_back(food);
        }

        if (!found)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Food not found"})"
            );
        }

        saveJsonFile(
            "data/foods.json",
            updatedFoods
        );

        return crow::response(
            200,
            R"({"status":"success","message":"Food deleted successfully"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Failed to delete food"})"
        );
    }
});


    // ========================================================
    // REGISTER
    // ========================================================

    CROW_ROUTE(app, "/api/register")
        .methods("POST"_method)
    ([](const crow::request& request)
    {
        try
        {
            json requestData =
                json::parse(request.body);

            std::string name =
                requestData.value("name", "");

            std::string phone =
                requestData.value("phone", "");

            std::string email =
                requestData.value("email", "");

            std::string password =
                requestData.value("password", "");


            // -----------------------------
            // Validation
            // -----------------------------

            if (
                name.empty() ||
                phone.empty() ||
                email.empty() ||
                password.empty()
            )
            {
                return crow::response(
                    400,
                    R"({"status":"error","message":"All fields are required"})"
                );
            }


            if (password.length() < 6)
            {
                return crow::response(
                    400,
                    R"({"status":"error","message":"Password must contain at least 6 characters"})"
                );
            }


            std::lock_guard<std::mutex> lock(
                userFileMutex
            );


            json users =
                loadJsonFile(USER_FILE);


            // -----------------------------
            // Check duplicate email
            // -----------------------------

            for (const auto& user : users)
            {
                if (
                    user.contains("email") &&
                    user["email"] == email
                )
                {
                    return crow::response(
                        409,
                        R"({"status":"error","message":"Email already registered"})"
                    );
                }
            }


            // -----------------------------
            // Generate user ID
            // -----------------------------

            int newId = 1;

            if (!users.empty())
            {
                newId =
                    users.back()["id"].get<int>() + 1;
            }


            // -----------------------------
            // Create user
            // -----------------------------

            json newUser;

            newUser["id"] = newId;
            newUser["name"] = name;
            newUser["phone"] = phone;
            newUser["email"] = email;

            // Store hash, NOT password
            newUser["password"] =
                hashPassword(password);

            newUser["role"] = "customer";


            users.push_back(newUser);


            if (!saveJsonFile(USER_FILE, users))
            {
                return crow::response(
                    500,
                    R"({"status":"error","message":"Unable to save user"})"
                );
            }


            // Do not return password
            json responseData;

            responseData["status"] =
                "success";

            responseData["message"] =
                "Registration successful";

            responseData["user"]["id"] =
                newUser["id"];

            responseData["user"]["name"] =
                newUser["name"];

            responseData["user"]["email"] =
                newUser["email"];

            responseData["user"]["role"] =
                newUser["role"];


            crow::response response(
                responseData.dump()
            );

            response.set_header(
                "Content-Type",
                "application/json"
            );

            return response;
        }
        catch (...)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Invalid request data"})"
            );
        }
    });


    // ========================================================
    // LOGIN
    // ========================================================

    CROW_ROUTE(app, "/api/login")
        .methods("POST"_method)
    ([](const crow::request& request)
    {
        try
        {
            json requestData =
                json::parse(request.body);

            std::string email =
                requestData.value("email", "");

            std::string password =
                requestData.value("password", "");


            if (
                email.empty() ||
                password.empty()
            )
            {
                return crow::response(
                    400,
                    R"({"status":"error","message":"Email and password are required"})"
                );
            }


            std::lock_guard<std::mutex> lock(
                userFileMutex
            );


            json users =
                loadJsonFile(USER_FILE);


            std::string passwordHash =
                hashPassword(password);


            for (const auto& user : users)
            {
                if (
                    user["email"] == email &&
                    user["password"] == passwordHash
                )
                {
                    std::string token =
                        generateToken();


                    {
                        std::lock_guard<std::mutex>
                            sessionLock(sessionMutex);

                        sessions[token] =
                            user["id"].get<int>();
                    }


                    json responseData;

                    responseData["status"] =
                        "success";

                    responseData["message"] =
                        "Login successful";

                    responseData["token"] =
                        token;

                    responseData["user"]["id"] =
                        user["id"];

                    responseData["user"]["name"] =
                        user["name"];

                    responseData["user"]["email"] =
                        user["email"];

                    responseData["user"]["role"] =
                        user["role"];


                    crow::response response(
                        responseData.dump()
                    );

                    response.set_header(
                        "Content-Type",
                        "application/json"
                    );

                    return response;
                }
            }


            return crow::response(
                401,
                R"({"status":"error","message":"Invalid email or password"})"
            );
        }
        catch (...)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Invalid request data"})"
            );
        }
    });

    // ========================================================
// CREATE ORDER
// ========================================================

CROW_ROUTE(app, "/api/orders")
    .methods("POST"_method)
([](const crow::request& request)
{
    try
    {
        json requestData = json::parse(request.body);

        int userId =
            requestData.value("userId", 0);

        std::string address =
            requestData.value("address", "");

        std::string paymentMethod =
            requestData.value("paymentMethod", "");

        json items =
            requestData.value("items", json::array());

        // -----------------------------
        // Validation
        // -----------------------------

        if (userId <= 0)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Invalid user ID"})"
            );
        }

        if (address.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Delivery address is required"})"
            );
        }

        if (paymentMethod.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Payment method is required"})"
            );
        }

        if (items.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Order must contain at least one item"})"
            );
        }

        // -----------------------------
        // Calculate total
        // -----------------------------

        double total = 0;

        for (const auto& item : items)
        {
            double price =
                item.value("price", 0.0);

            int quantity =
                item.value("quantity", 0);

            if (price < 0 || quantity <= 0)
            {
                return crow::response(
                    400,
                    R"({"status":"error","message":"Invalid order item"})"
                );
            }

            total += price * quantity;
        }

        // -----------------------------
        // Load existing orders
        // -----------------------------

        json orders;

        {
            std::lock_guard<std::mutex> lock(
                userFileMutex
            );

            orders = loadJsonFile(
                "data/orders.json"
            );

            int newOrderId = 1;

            if (!orders.empty())
            {
                newOrderId =
                    orders.back()["id"].get<int>() + 1;
            }

            // -----------------------------
            // Create order
            // -----------------------------

            json newOrder;

            newOrder["id"] = newOrderId;
            newOrder["userId"] = userId;
            newOrder["address"] = address;
            newOrder["paymentMethod"] = paymentMethod;
            newOrder["items"] = items;
            newOrder["total"] = total;

            // Initial order state
            newOrder["status"] = "PLACED";

            orders.push_back(newOrder);

            if (!saveJsonFile(
        "data/orders.json",
        orders))
{
    return crow::response(
        500,
        R"({"status":"error","message":"Unable to save order"})"
    );
}

// ------------------------------------------------------------
// Linux IPC: notify the event logger
// ------------------------------------------------------------

std::stringstream eventStream;

eventStream
    << "ORDER_PLACED"
    << " | Order ID: " << newOrderId
    << " | User ID: " << userId
    << " | Total: " << total;

sendOrderEvent(eventStream.str());

json responseData;

            responseData["status"] =
                "success";

            responseData["message"] =
                "Order placed successfully";

            responseData["order"] =
                newOrder;

            crow::response response(
                responseData.dump()
            );

            response.set_header(
                "Content-Type",
                "application/json"
            );

            return response;
        }
    }
    catch (...)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid order data"})"
        );
    }
});

// ========================================================
// GET ORDERS FOR CUSTOMER
// ========================================================

CROW_ROUTE(app, "/api/orders")
    .methods("GET"_method)
([](const crow::request& request)
{
    try
    {
        const char* userIdParam =
            request.url_params.get("userId");

        if (userIdParam == nullptr)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"userId is required"})"
            );
        }

        int userId =
            std::stoi(userIdParam);

        json orders =
            loadJsonFile("data/orders.json");

        json userOrders =
            json::array();

        for (const auto& order : orders)
        {
            if (
                order.contains("userId") &&
                order["userId"].get<int>() == userId
            )
            {
                userOrders.push_back(order);
            }
        }

        crow::response response(
            userOrders.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        return response;
    }
    catch (...)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid user ID"})"
        );
    }
});


// ========================================================
// GET ALL ORDERS FOR ADMIN
// ========================================================

CROW_ROUTE(app, "/api/admin/orders")
    .methods("GET"_method)
([]()
{
    try
    {
        json orders =
            loadJsonFile("data/orders.json");

        crow::response response(
            orders.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        return response;
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to load orders"})"
        );
    }
});

// ========================================================
// GET ALL CUSTOMERS FOR ADMIN
// ========================================================

CROW_ROUTE(app, "/api/admin/customers")
    .methods("GET"_method)
([]()
{
    try
    {
        json users =
            loadJsonFile(USER_FILE);

        json customers =
            json::array();

        for (const auto& user : users)
        {
            // Only return customer accounts
            if (
                user.contains("role") &&
                user["role"] == "customer"
            )
            {
                json customer;

                customer["id"] =
                    user.value("id", 0);

                customer["name"] =
                    user.value("name", "");

                customer["phone"] =
                    user.value("phone", "");

                customer["email"] =
                    user.value("email", "");

                customer["role"] =
                    user.value("role", "customer");

                // IMPORTANT:
                // Password hash is intentionally
                // NOT included in the response.

                customers.push_back(customer);
            }
        }

        crow::response response(
            customers.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        return response;
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to load customers"})"
        );
    }
});


// ========================================================
// UPDATE ORDER STATUS
// ========================================================

CROW_ROUTE(app, "/api/orders/<int>/status")
    .methods("PUT"_method)
([](const crow::request& request, int orderId)
{
    try
    {
        json requestData =
            json::parse(request.body);

        std::string newStatus =
            requestData.value("status", "");

        const std::vector<std::string> validStatuses = {
            "PLACED",
            "CONFIRMED",
            "PREPARING",
            "READY",
            "OUT_FOR_DELIVERY",
            "DELIVERED",
            "CANCELLED"
        };

        bool validStatus = false;

        for (const auto& status : validStatuses)
        {
            if (status == newStatus)
            {
                validStatus = true;
                break;
            }
        }

        if (!validStatus)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Invalid order status"})"
            );
        }

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json orders =
            loadJsonFile("data/orders.json");

        for (auto& order : orders)
        {
            if (
                order.contains("id") &&
                order["id"].get<int>() == orderId
            )
            {
                order["status"] = newStatus;

                if (!saveJsonFile(
                        "data/orders.json",
                        orders))
                {
                    return crow::response(
                        500,
                        R"({"status":"error","message":"Unable to save order"})"
                    );
                }

                json responseData;

                responseData["status"] =
                    "success";

                responseData["message"] =
                    "Order status updated";

                responseData["order"] =
                    order;

                crow::response response(
                    responseData.dump()
                );

                response.set_header(
                    "Content-Type",
                    "application/json"
                );

                return response;
            }
        }

        return crow::response(
            404,
            R"({"status":"error","message":"Order not found"})"
        );
    }
    catch (...)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid request"})"
        );
    }
});


    // ========================================================
    // SERVER
    // ========================================================

    std::cout
        << "====================================\n"
        << " Food Ordering System Backend\n"
        << " C++ HTTP Server\n"
        << " File-based Persistence\n"
        << " Authentication Enabled\n"
        << "====================================\n"
        << "Server running on http://localhost:18080\n";


    app.port(18080)
       .multithreaded()
       .run();


    return 0;
}