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

using json = nlohmann::json;

const std::string FOOD_FILE = "data/foods.json";
const std::string USER_FILE = "data/users.json";

std::mutex userFileMutex;
std::mutex sessionMutex;

std::unordered_map<std::string, int> sessions;


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