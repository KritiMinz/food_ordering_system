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
#include <cctype>

using json = nlohmann::json;

const std::string FOOD_FILE = "data/foods.json";
const std::string USER_FILE = "data/users.json";
const std::string COUPON_FILE = "data/coupons.json";

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
        std::cerr
            << "[IPC] Logger is not running. "
            << "Order event was not logged."
            << std::endl;

        return;
    }

    ssize_t bytes_written = write(
        fd,
        event.c_str(),
        event.size()
    );

    if (bytes_written == -1)
    {
        std::cerr
            << "[IPC] Failed to send order event."
            << std::endl;
    }
    else
    {
        std::cout
            << "[IPC] Order event sent: "
            << event
            << std::endl;
    }

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
// Authentication and authorization helpers
// ============================================================

bool authenticateRequest(
    const crow::request& request,
    int& userId,
    std::string& role)
{
    std::string authorization =
        request.get_header_value("Authorization");

    const std::string prefix = "Bearer ";

    if (
        authorization.size() <= prefix.size() ||
        authorization.compare(
            0,
            prefix.size(),
            prefix
        ) != 0
    )
    {
        return false;
    }

    std::string token =
        authorization.substr(prefix.size());

    int authenticatedUserId = 0;

    {
        std::lock_guard<std::mutex> lock(
            sessionMutex
        );

        auto session =
            sessions.find(token);

        if (session == sessions.end())
        {
            return false;
        }

        authenticatedUserId =
            session->second;
    }

    json users =
        loadJsonFile(USER_FILE);

    for (const auto& user : users)
    {
        if (
            user.contains("id") &&
            user["id"].get<int>() == authenticatedUserId
        )
        {
            userId = authenticatedUserId;

            role =
                user.value(
                    "role",
                    "customer"
                );

            return true;
        }
    }

    return false;
}


bool requireAdmin(
    const crow::request& request,
    int& userId)
{
    std::string role;

    if (!authenticateRequest(
            request,
            userId,
            role))
    {
        return false;
    }

    return role == "admin";
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

auto& cors =
    app.get_middleware<crow::CORSHandler>();

cors.global()
    .origin("http://localhost:5173")
    .headers(
        "Content-Type",
        "Authorization"
    )
    .methods(
        "GET"_method,
        "POST"_method,
        "PUT"_method,
        "DELETE"_method,
        "OPTIONS"_method
    )
    .max_age(86400);

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

    // Make older food records available by default
    // if they do not yet contain the availability field.

    for (auto& food : foods)
    {
        if (!food.contains("available"))
        {
            food["available"] = true;
        }
    }

    crow::json::wvalue response =
        crow::json::load(foods.dump());

    return response;
});

// ========================================================
// ADMIN - GET COUPONS
// ========================================================

CROW_ROUTE(app, "/api/admin/coupons")
    .methods("GET"_method)
([](const crow::request& request)
{
    try
    {
        // ------------------------------------------------
        // Admin authentication
        // ------------------------------------------------

        int adminUserId = 0;

        if (!requireAdmin(
                request,
                adminUserId))
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Admin authentication required"})"
            );
        }

        // ------------------------------------------------
        // Load coupons
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json coupons =
            loadJsonFile(COUPON_FILE);

        // ------------------------------------------------
        // Return coupons
        // ------------------------------------------------

        crow::response response(
            coupons.dump()
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
            R"({"status":"error","message":"Unable to load coupons"})"
        );
    }
});

// ========================================================
// ADMIN - CREATE COUPON
// ========================================================

CROW_ROUTE(app, "/api/admin/coupons")
    .methods("POST"_method)
([](const crow::request& request)
{
    try
    {
        // ------------------------------------------------
        // Admin authentication
        // ------------------------------------------------

        int adminUserId = 0;

        if (!requireAdmin(
                request,
                adminUserId))
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Admin authentication required"})"
            );
        }

        // ------------------------------------------------
        // Parse request
        // ------------------------------------------------

        json requestData =
            json::parse(request.body);

        std::string code =
            requestData.value("code", "");

        std::string discountType =
            requestData.value(
                "discountType",
                "percentage"
            );

        double discountValue =
            requestData.value(
                "discountValue",
                0.0
            );

        double minimumOrder =
            requestData.value(
                "minimumOrder",
                0.0
            );

        // ------------------------------------------------
        // Validate
        // ------------------------------------------------

        if (code.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Coupon code is required"})"
            );
        }

        if (
            discountType != "percentage" &&
            discountType != "fixed"
        )
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Invalid discount type"})"
            );
        }

        if (discountValue <= 0)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Discount value must be greater than zero"})"
            );
        }

        if (discountType == "percentage" &&
            discountValue > 100)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Percentage discount cannot exceed 100"})"
            );
        }

        if (minimumOrder < 0)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Minimum order cannot be negative"})"
            );
        }

        // ------------------------------------------------
        // Normalize coupon code
        // ------------------------------------------------

        for (char& character : code)
        {
            character =
                static_cast<char>(
                    std::toupper(
                        static_cast<unsigned char>(
                            character
                        )
                    )
                );
        }

        // ------------------------------------------------
        // Load coupons
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json coupons =
            loadJsonFile(COUPON_FILE);

        // ------------------------------------------------
        // Check duplicate coupon
        // ------------------------------------------------

        for (const auto& coupon : coupons)
        {
            if (
                coupon.contains("code") &&
                coupon["code"].get<std::string>() ==
                    code
            )
            {
                return crow::response(
                    409,
                    R"({"status":"error","message":"Coupon code already exists"})"
                );
            }
        }

        // ------------------------------------------------
        // Generate ID
        // ------------------------------------------------

        int newCouponId = 1;

        if (!coupons.empty())
        {
            newCouponId =
                coupons.back()["id"].get<int>() + 1;
        }

        // ------------------------------------------------
        // Create coupon
        // ------------------------------------------------

        json coupon;

        coupon["id"] =
            newCouponId;

        coupon["code"] =
            code;

        coupon["discountType"] =
            discountType;

        coupon["discountValue"] =
            discountValue;

        coupon["minimumOrder"] =
            minimumOrder;

        coupon["active"] =
            true;

        coupons.push_back(coupon);

        // ------------------------------------------------
        // Save
        // ------------------------------------------------

        if (!saveJsonFile(
                COUPON_FILE,
                coupons))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to save coupon"})"
            );
        }

        // ------------------------------------------------
        // Response
        // ------------------------------------------------

        json responseData;

        responseData["status"] =
            "success";

        responseData["message"] =
            "Coupon created successfully";

        responseData["coupon"] =
            coupon;

        crow::response response(
            responseData.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        return response;
    }
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid request data"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to create coupon"})"
        );
    }
});

// ========================================================
// ADMIN - ACTIVATE / DEACTIVATE COUPON
// ========================================================

CROW_ROUTE(app, "/api/admin/coupons/<int>/status")
    .methods("PUT"_method)
([](const crow::request& request, int couponId)
{
    try
    {
        // ------------------------------------------------
        // Admin authentication
        // ------------------------------------------------

        int adminUserId = 0;

        if (!requireAdmin(
                request,
                adminUserId))
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Admin authentication required"})"
            );
        }

        // ------------------------------------------------
        // Parse request
        // ------------------------------------------------

        json requestData =
            json::parse(request.body);

        bool active =
            requestData.value("active", false);

        // ------------------------------------------------
        // Load coupons
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json coupons =
            loadJsonFile(COUPON_FILE);

        // ------------------------------------------------
        // Find coupon
        // ------------------------------------------------

        bool found = false;

        for (auto& coupon : coupons)
        {
            if (
                coupon.contains("id") &&
                coupon["id"].get<int>() == couponId
            )
            {
                coupon["active"] = active;
                found = true;
                break;
            }
        }

        // ------------------------------------------------
        // Coupon not found
        // ------------------------------------------------

        if (!found)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Coupon not found"})"
            );
        }

        // ------------------------------------------------
        // Save
        // ------------------------------------------------

        if (!saveJsonFile(
                COUPON_FILE,
                coupons))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to save coupon status"})"
            );
        }

        // ------------------------------------------------
        // Response
        // ------------------------------------------------

        json responseData;

        responseData["status"] =
            "success";

        responseData["message"] =
            active
                ? "Coupon activated successfully"
                : "Coupon deactivated successfully";

        responseData["couponId"] =
            couponId;

        responseData["active"] =
            active;

        crow::response response(
            responseData.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        return response;
    }
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid request data"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to update coupon status"})"
        );
    }
});

// ========================================================
// CUSTOMER - GET ACTIVE COUPONS
// ========================================================

CROW_ROUTE(app, "/api/coupons")
    .methods("GET"_method)
([](const crow::request& request)
{
    try
    {
        // ------------------------------------------------
        // Customer authentication
        // ------------------------------------------------

        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Customer access required"})"
            );
        }

        // ------------------------------------------------
        // Load coupons
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json coupons =
            loadJsonFile(COUPON_FILE);

        json activeCoupons =
            json::array();

        // ------------------------------------------------
        // Return only active coupons
        // ------------------------------------------------

        for (const auto& coupon : coupons)
        {
            if (
                coupon.value("active", false)
            )
            {
                activeCoupons.push_back(coupon);
            }
        }

        crow::response response(
            activeCoupons.dump()
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
            R"({"status":"error","message":"Unable to load coupons"})"
        );
    }
});

// ========================================================
// CUSTOMER - APPLY COUPON
// ========================================================

CROW_ROUTE(app, "/api/coupons/apply")
    .methods("POST"_method)
([](const crow::request& request)
{
    try
    {
        // ------------------------------------------------
        // Authenticate customer
        // ------------------------------------------------

        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Customer access required"})"
            );
        }

        // ------------------------------------------------
        // Parse request
        // ------------------------------------------------

        json requestData =
            json::parse(request.body);

        std::string code =
            requestData.value("code", "");

        double subtotal =
            requestData.value("subtotal", 0.0);

        // ------------------------------------------------
        // Validate input
        // ------------------------------------------------

        if (code.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Coupon code is required"})"
            );
        }

        if (subtotal <= 0)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Invalid order subtotal"})"
            );
        }

        // ------------------------------------------------
        // Normalize coupon code
        // ------------------------------------------------

        for (char& character : code)
        {
            character =
                static_cast<char>(
                    std::toupper(
                        static_cast<unsigned char>(
                            character
                        )
                    )
                );
        }

        // ------------------------------------------------
        // Load coupons
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json coupons =
            loadJsonFile(COUPON_FILE);

        // ------------------------------------------------
        // Find coupon
        // ------------------------------------------------

        for (const auto& coupon : coupons)
        {
            std::string couponCode =
                coupon.value("code", "");

            if (couponCode != code)
            {
                continue;
            }

            // --------------------------------------------
            // Check active
            // --------------------------------------------

            bool active =
                coupon.value("active", false);

            if (!active)
            {
                return crow::response(
                    400,
                    R"({"status":"error","message":"This coupon is inactive"})"
                );
            }

            // --------------------------------------------
            // Check minimum order
            // --------------------------------------------

            double minimumOrder =
                coupon.value(
                    "minimumOrder",
                    0.0
                );

            if (subtotal < minimumOrder)
            {
                json responseData;

                responseData["status"] =
                    "error";

                responseData["message"] =
                    "Minimum order value is ₹" +
                    std::to_string(
                        static_cast<int>(
                            minimumOrder
                        )
                    );

                crow::response response(
                    400,
                    responseData.dump()
                );

                response.set_header(
                    "Content-Type",
                    "application/json"
                );

                return response;
            }

            // --------------------------------------------
            // Calculate discount
            // --------------------------------------------

            std::string discountType =
                coupon.value(
                    "discountType",
                    "percentage"
                );

            double discountValue =
                coupon.value(
                    "discountValue",
                    0.0
                );

            double discount = 0.0;

            if (discountType == "percentage")
            {
                discount =
                    subtotal *
                    discountValue /
                    100.0;
            }
            else if (discountType == "fixed")
            {
                discount =
                    discountValue;
            }
            else
            {
                return crow::response(
                    400,
                    R"({"status":"error","message":"Invalid coupon configuration"})"
                );
            }

            // --------------------------------------------
            // Never allow discount above subtotal
            // --------------------------------------------

            if (discount > subtotal)
            {
                discount = subtotal;
            }

            double finalTotal =
                subtotal - discount;

            // --------------------------------------------
            // Response
            // --------------------------------------------

            json responseData;

            responseData["status"] =
                "success";

            responseData["message"] =
                "Coupon applied successfully";

            responseData["couponCode"] =
                code;

            responseData["discountType"] =
                discountType;

            responseData["discountValue"] =
                discountValue;

            responseData["subtotal"] =
                subtotal;

            responseData["discount"] =
                discount;

            responseData["total"] =
                finalTotal;

            crow::response response(
                responseData.dump()
            );

            response.set_header(
                "Content-Type",
                "application/json"
            );

            return response;
        }

        // ------------------------------------------------
        // Coupon not found
        // ------------------------------------------------

        return crow::response(
            404,
            R"({"status":"error","message":"Invalid coupon code"})"
        );
    }
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid request data"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to apply coupon"})"
        );
    }
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
        // ------------------------------------------------
        // Admin authentication
        // ------------------------------------------------

        int adminUserId = 0;

        if (!requireAdmin(request, adminUserId))
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Admin authentication required"})"
            );
        }

        // ------------------------------------------------
        // Parse request
        // ------------------------------------------------

        json requestData =
            json::parse(request.body);

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

        // ------------------------------------------------
        // Validate food data
        // ------------------------------------------------

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

        // ------------------------------------------------
        // Load and update food file
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json foods =
            loadJsonFile("data/foods.json");

        int newId = 1;

        for (const auto& food : foods)
        {
            if (food.contains("id"))
            {
                newId = std::max(
                    newId,
                    food["id"].get<int>() + 1
                );
            }
        }

        // ------------------------------------------------
        // Create new food
        // ------------------------------------------------

        json newFood = {
    {"id", newId},
    {"name", name},
    {"description", description},
    {"price", price},
    {"category", category},
    {"image", image},
    {"available", true}
};

        foods.push_back(newFood);

        // ------------------------------------------------
        // Save food data
        // ------------------------------------------------

        if (!saveJsonFile(
                "data/foods.json",
                foods))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to save food"})"
            );
        }

        // ------------------------------------------------
        // Response
        // ------------------------------------------------

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
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid JSON request"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Internal server error"})"
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
        // ------------------------------------------------
        // Admin authentication
        // ------------------------------------------------

        int adminUserId = 0;

        if (!requireAdmin(
                request,
                adminUserId))
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Admin authentication required"})"
            );
        }

        // ------------------------------------------------
        // Parse request
        // ------------------------------------------------

        json requestData =
            json::parse(request.body);

        // ------------------------------------------------
        // Load food data
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json foods =
            loadJsonFile("data/foods.json");

        bool found = false;
        json updatedFood;

        // ------------------------------------------------
        // Find and update food
        // ------------------------------------------------

        for (auto& food : foods)
        {
            if (
                food.contains("id") &&
                food["id"].get<int>() == foodId
            )
            {
                if (requestData.contains("name"))
                {
                    food["name"] =
                        requestData["name"];
                }

                if (requestData.contains("description"))
                {
                    food["description"] =
                        requestData["description"];
                }

                if (requestData.contains("price"))
                {
                    food["price"] =
                        requestData["price"];
                }

                if (requestData.contains("category"))
                {
                    food["category"] =
                        requestData["category"];
                }

                if (requestData.contains("image"))
                {
                    food["image"] =
                        requestData["image"];
                }

                if (requestData.contains("available"))
{
    food["available"] =
        requestData["available"];
}

                updatedFood = food;
                found = true;

                break;
            }
        }

        // ------------------------------------------------
        // Food not found
        // ------------------------------------------------

        if (!found)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Food not found"})"
            );
        }

        // ------------------------------------------------
        // Save updated food data
        // ------------------------------------------------

        if (!saveJsonFile(
                "data/foods.json",
                foods))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to save food"})"
            );
        }

        // ------------------------------------------------
        // Response
        // ------------------------------------------------

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
// ADMIN - UPDATE FOOD AVAILABILITY
// ========================================================

CROW_ROUTE(app, "/api/admin/foods/<int>/availability")
    .methods("PUT"_method)
([](const crow::request& request, int foodId)
{
    try
    {
        // ------------------------------------------------
        // Admin authentication
        // ------------------------------------------------

        int adminUserId = 0;

        if (!requireAdmin(
                request,
                adminUserId))
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Admin authentication required"})"
            );
        }

        // ------------------------------------------------
        // Parse request
        // ------------------------------------------------

        json requestData =
            json::parse(request.body);

        if (!requestData.contains("available"))
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Availability value is required"})"
            );
        }

        bool available =
            requestData["available"].get<bool>();

        // ------------------------------------------------
        // Load foods
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json foods =
            loadJsonFile("data/foods.json");

        bool found = false;
        json updatedFood;

        // ------------------------------------------------
        // Find food
        // ------------------------------------------------

        for (auto& food : foods)
        {
            if (
                food.contains("id") &&
                food["id"].get<int>() == foodId
            )
            {
                food["available"] = available;

                updatedFood = food;
                found = true;

                break;
            }
        }

        // ------------------------------------------------
        // Food not found
        // ------------------------------------------------

        if (!found)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Food not found"})"
            );
        }

        // ------------------------------------------------
        // Save foods
        // ------------------------------------------------

        if (!saveJsonFile(
                "data/foods.json",
                foods))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to save food availability"})"
            );
        }

        // ------------------------------------------------
        // Response
        // ------------------------------------------------

        crow::response response(200);

        response.set_header(
            "Content-Type",
            "application/json"
        );

        response.write(
            json({
                {"status", "success"},
                {
                    "message",
                    available
                        ? "Food marked as available"
                        : "Food marked as out of stock"
                },
                {"food", updatedFood}
            }).dump()
        );

        return response;
    }
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid JSON request"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to update food availability"})"
        );
    }
});

// ========================================================
// ADMIN - DELETE FOOD
// ========================================================

CROW_ROUTE(app, "/api/admin/foods/<int>")
    .methods("DELETE"_method)
([](const crow::request& request, int foodId)
{
    try
    {
        // ------------------------------------------------
        // Admin authentication
        // ------------------------------------------------

        int adminUserId = 0;

        if (!requireAdmin(
                request,
                adminUserId))
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Admin authentication required"})"
            );
        }

        // ------------------------------------------------
        // Load food data
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json foods =
            loadJsonFile("data/foods.json");

        json updatedFoods =
            json::array();

        bool found = false;

        // ------------------------------------------------
        // Find food and remove it
        // ------------------------------------------------

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

        // ------------------------------------------------
        // Food not found
        // ------------------------------------------------

        if (!found)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Food not found"})"
            );
        }

        // ------------------------------------------------
        // Save updated food data
        // ------------------------------------------------

        if (!saveJsonFile(
                "data/foods.json",
                updatedFoods))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to save food data"})"
            );
        }

        // ------------------------------------------------
        // Success response
        // ------------------------------------------------

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
// CUSTOMER PROFILE
// ========================================================

CROW_ROUTE(app, "/api/profile")
    .methods("GET"_method)
([](const crow::request& request)
{
    try
    {
        // ----------------------------------------
        // Authenticate customer
        // ----------------------------------------

        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        // ----------------------------------------
        // Only customers
        // ----------------------------------------

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Customer access required"})"
            );
        }

        // ----------------------------------------
        // Load users
        // ----------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json users =
            loadJsonFile(USER_FILE);

        // ----------------------------------------
        // Find authenticated user
        // ----------------------------------------

        for (const auto& user : users)
        {
            if (
                user.contains("id") &&
                user["id"].get<int>() ==
                    authenticatedUserId
            )
            {
                json profile;

                profile["id"] =
                    user.value("id", 0);

                profile["name"] =
                    user.value("name", "");

                profile["email"] =
                    user.value("email", "");

                profile["phone"] =
                    user.value("phone", "");

                profile["role"] =
                    user.value("role", "customer");

                // Never return password/hash

                json responseData;

                responseData["status"] =
                    "success";

                responseData["profile"] =
                    profile;

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

        // ----------------------------------------
        // User not found
        // ----------------------------------------

        return crow::response(
            404,
            R"({"status":"error","message":"User profile not found"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to load profile"})"
        );
    }
});


// ========================================================
// UPDATE CUSTOMER PROFILE
// ========================================================

CROW_ROUTE(app, "/api/profile")
    .methods("PUT"_method)
([](const crow::request& request)
{
    try
    {
        // ----------------------------------------
        // Authenticate user
        // ----------------------------------------

        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        // ----------------------------------------
        // Only customers can update profile
        // ----------------------------------------

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Customer access required"})"
            );
        }

        // ----------------------------------------
        // Parse request
        // ----------------------------------------

        json requestData =
            json::parse(request.body);

        std::string name =
            requestData.value("name", "");

        std::string phone =
            requestData.value("phone", "");

        // ----------------------------------------
        // Validate fields
        // ----------------------------------------

        if (name.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Name is required"})"
            );
        }

        if (phone.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Phone number is required"})"
            );
        }

        // ----------------------------------------
        // Load users
        // ----------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json users =
            loadJsonFile(USER_FILE);

        // ----------------------------------------
        // Find authenticated user
        // ----------------------------------------

        for (auto& user : users)
        {
            if (
                user.contains("id") &&
                user["id"].get<int>() ==
                    authenticatedUserId
            )
            {
                // --------------------------------
                // Update allowed fields
                // --------------------------------

                user["name"] = name;
                user["phone"] = phone;

                // --------------------------------
                // Save users
                // --------------------------------

                if (!saveJsonFile(
                        USER_FILE,
                        users))
                {
                    return crow::response(
                        500,
                        R"({"status":"error","message":"Unable to save profile"})"
                    );
                }

                // --------------------------------
                // Response
                // --------------------------------

                json profile;

                profile["id"] =
                    user.value("id", 0);

                profile["name"] =
                    user.value("name", "");

                profile["email"] =
                    user.value("email", "");

                profile["phone"] =
                    user.value("phone", "");

                profile["role"] =
                    user.value("role", "customer");

                json responseData;

                responseData["status"] =
                    "success";

                responseData["message"] =
                    "Profile updated successfully";

                responseData["profile"] =
                    profile;

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

        // ----------------------------------------
        // User not found
        // ----------------------------------------

        return crow::response(
            404,
            R"({"status":"error","message":"User profile not found"})"
        );
    }
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid request data"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to update profile"})"
        );
    }
});


// ========================================================
// CUSTOMER ADDRESSES
// ========================================================

// --------------------------------------------------------
// GET CUSTOMER ADDRESSES
// --------------------------------------------------------

CROW_ROUTE(app, "/api/addresses")
    .methods("GET"_method)
([](const crow::request& request)
{
    try
    {
        // Authenticate customer
        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Customer access required"})"
            );
        }

        // Load addresses
        json addresses =
            loadJsonFile("data/addresses.json");

        json userAddresses =
            json::array();

        // Return only this customer's addresses
        for (const auto& address : addresses)
        {
            if (
                address.contains("userId") &&
                address["userId"].get<int>() ==
                    authenticatedUserId
            )
            {
                userAddresses.push_back(address);
            }
        }

        json responseData;

        responseData["status"] = "success";
        responseData["addresses"] = userAddresses;

        crow::response response(
            200,
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
            500,
            R"({"status":"error","message":"Unable to load addresses"})"
        );
    }
});


// --------------------------------------------------------
// ADD CUSTOMER ADDRESS
// --------------------------------------------------------

CROW_ROUTE(app, "/api/addresses")
    .methods("POST"_method)
([](const crow::request& request)
{
    try
    {
        // Authenticate customer
        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Customer access required"})"
            );
        }

        // Parse request
        json requestData =
            json::parse(request.body);

        std::string label =
            requestData.value("label", "");

        std::string address =
            requestData.value("address", "");

        // Validate
        if (label.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Address label is required"})"
            );
        }

        if (address.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Address is required"})"
            );
        }

        // Protect file access
        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        // Load addresses
        json addresses =
            loadJsonFile("data/addresses.json");

        // Generate new ID
        int newId = 1;

        for (const auto& existing : addresses)
        {
            if (existing.contains("id"))
            {
                newId = std::max(
                    newId,
                    existing["id"].get<int>() + 1
                );
            }
        }

        // Create address
        json newAddress;

        newAddress["id"] =
            newId;

        newAddress["userId"] =
            authenticatedUserId;

        newAddress["label"] =
            label;

        newAddress["address"] =
            address;

        // Save
        addresses.push_back(newAddress);

        if (!saveJsonFile(
                "data/addresses.json",
                addresses))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to save address"})"
            );
        }

        // Response
        json responseData;

        responseData["status"] =
            "success";

        responseData["message"] =
            "Address added successfully";

        responseData["address"] =
            newAddress;

        crow::response response(
            201,
            responseData.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        return response;
    }
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid JSON request"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to add address"})"
        );
    }
});

// ========================================================
// UPDATE CUSTOMER ADDRESS
// ========================================================

CROW_ROUTE(app, "/api/addresses/<int>")
    .methods("PUT"_method)
([](const crow::request& request, int addressId)
{
    try
    {
        // Authenticate customer
        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Customer access required"})"
            );
        }

        // Parse request
        json requestData =
            json::parse(request.body);

        std::string label =
            requestData.value("label", "");

        std::string address =
            requestData.value("address", "");

        if (label.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Address label is required"})"
            );
        }

        if (address.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Address is required"})"
            );
        }

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json addresses =
            loadJsonFile("data/addresses.json");

        bool found = false;
        json updatedAddress;

        for (auto& existing : addresses)
        {
            if (
                existing.contains("id") &&
                existing["id"].get<int>() == addressId &&
                existing.contains("userId") &&
                existing["userId"].get<int>() ==
                    authenticatedUserId
            )
            {
                existing["label"] = label;
                existing["address"] = address;

                updatedAddress = existing;
                found = true;

                break;
            }
        }

        if (!found)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Address not found"})"
            );
        }

        if (!saveJsonFile(
                "data/addresses.json",
                addresses))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to save address"})"
            );
        }

        json responseData;

        responseData["status"] =
            "success";

        responseData["message"] =
            "Address updated successfully";

        responseData["address"] =
            updatedAddress;

        crow::response response(
            200,
            responseData.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        return response;
    }
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid JSON request"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to update address"})"
        );
    }
});


// ========================================================
// DELETE CUSTOMER ADDRESS
// ========================================================

CROW_ROUTE(app, "/api/addresses/<int>")
    .methods("DELETE"_method)
([](const crow::request& request, int addressId)
{
    try
    {
        // Authenticate customer
        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Customer access required"})"
            );
        }

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json addresses =
            loadJsonFile("data/addresses.json");

        json updatedAddresses =
            json::array();

        bool found = false;

        for (const auto& existing : addresses)
        {
            if (
                existing.contains("id") &&
                existing["id"].get<int>() == addressId &&
                existing.contains("userId") &&
                existing["userId"].get<int>() ==
                    authenticatedUserId
            )
            {
                found = true;
                continue;
            }

            updatedAddresses.push_back(existing);
        }

        if (!found)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Address not found"})"
            );
        }

        if (!saveJsonFile(
                "data/addresses.json",
                updatedAddresses))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to delete address"})"
            );
        }

        return crow::response(
            200,
            R"({"status":"success","message":"Address deleted successfully"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to delete address"})"
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
// Check food availability
// -----------------------------

json foods = loadJsonFile(FOOD_FILE);

for (const auto& item : items)
{
    int foodId =
        item.value("id", 0);

    if (foodId <= 0)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid food item"})"
        );
    }

    bool foodFound = false;

    for (const auto& food : foods)
    {
        if (
            food.contains("id") &&
            food["id"].get<int>() == foodId
        )
        {
            foodFound = true;

            // Older food records are considered available
            bool available =
                food.value("available", true);

            if (!available)
            {
                std::string foodName =
                    food.value("name", "Food");

                json errorResponse;

                errorResponse["status"] =
                    "error";

                errorResponse["message"] =
                    foodName +
                    " is currently unavailable";

                return crow::response(
                    400,
                    errorResponse.dump()
                );
            }

            break;
        }
    }

    if (!foodFound)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Food item not found"})"
        );
    }
}

        // -----------------------------
        // Calculate total
        // -----------------------------

        // -----------------------------
// Calculate subtotal
// -----------------------------

double subtotal = 0;

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

    subtotal += price * quantity;
}

// -----------------------------
// Apply coupon discount
// -----------------------------

std::string couponCode =
    requestData.value("couponCode", "");

double discount = 0.0;

if (!couponCode.empty())
{
    // Normalize coupon code
    for (char& character : couponCode)
    {
        character =
            static_cast<char>(
                std::toupper(
                    static_cast<unsigned char>(
                        character
                    )
                )
            );
    }

    // Load coupons
    json coupons =
        loadJsonFile("data/coupons.json");

    bool couponFound = false;

    for (const auto& coupon : coupons)
    {
        std::string storedCode =
            coupon.value("code", "");

        // Normalize stored code
        for (char& character : storedCode)
        {
            character =
                static_cast<char>(
                    std::toupper(
                        static_cast<unsigned char>(
                            character
                        )
                    )
                );
        }

        if (storedCode != couponCode)
        {
            continue;
        }

        couponFound = true;

        // Check active
        bool active =
            coupon.value("active", false);

        if (!active)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"This coupon is inactive"})"
            );
        }

        // Check minimum order
        double minimumOrder =
            coupon.value(
                "minimumOrder",
                0.0
            );

        if (subtotal < minimumOrder)
        {
            json errorResponse;

            errorResponse["status"] =
                "error";

            errorResponse["message"] =
                "Minimum order value is ₹" +
                std::to_string(
                    static_cast<int>(
                        minimumOrder
                    )
                );

            return crow::response(
                400,
                errorResponse.dump()
            );
        }

        // Get discount configuration
        std::string discountType =
            coupon.value(
                "discountType",
                "percentage"
            );

        double discountValue =
            coupon.value(
                "discountValue",
                0.0
            );

        // Calculate discount
        if (discountType == "percentage")
        {
            discount =
                subtotal *
                discountValue /
                100.0;
        }
        else if (discountType == "fixed")
        {
            discount =
                discountValue;
        }
        else
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Invalid coupon configuration"})"
            );
        }

        // Never discount more than subtotal
        if (discount > subtotal)
        {
            discount = subtotal;
        }

        break;
    }

    if (!couponFound)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid coupon code"})"
        );
    }
}

// -----------------------------
// Final total
// -----------------------------

double total =
    subtotal - discount;

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

newOrder["subtotal"] = subtotal;
newOrder["couponCode"] = couponCode;
newOrder["discount"] = discount;
newOrder["total"] = total;

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
        // ------------------------------------------------
        // Authenticate customer
        // ------------------------------------------------

        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        // ------------------------------------------------
        // Get requested userId
        // ------------------------------------------------

        const char* userIdParam =
            request.url_params.get("userId");

        if (userIdParam == nullptr)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"userId is required"})"
            );
        }

        int requestedUserId =
            std::stoi(userIdParam);

        // ------------------------------------------------
        // Prevent users from viewing another user's orders
        // ------------------------------------------------

        if (requestedUserId != authenticatedUserId)
        {
            return crow::response(
                403,
                R"({"status":"error","message":"You can only view your own orders"})"
            );
        }

        // ------------------------------------------------
        // Load orders
        // ------------------------------------------------

        json orders =
            loadJsonFile("data/orders.json");

        json userOrders =
            json::array();

        // ------------------------------------------------
        // Filter orders
        // ------------------------------------------------

        for (const auto& order : orders)
        {
            if (
                order.contains("userId") &&
                order["userId"].get<int>() ==
                    authenticatedUserId
            )
            {
                userOrders.push_back(order);
            }
        }

        // ------------------------------------------------
        // Response
        // ------------------------------------------------

        json responseData;

        responseData["status"] =
            "success";

        responseData["orders"] =
            userOrders;

        crow::response response(
            200,
            responseData.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        return response;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "GET customer orders error: "
            << error.what()
            << std::endl;

        return crow::response(
            400,
            R"({"status":"error","message":"Invalid user ID"})"
        );
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
// CUSTOMER - CANCEL ORDER
// ========================================================

CROW_ROUTE(app, "/api/orders/<int>/cancel")
    .methods("PUT"_method)
([](const crow::request& request, int orderId)
{
    try
    {
        // ------------------------------------------------
        // Authenticate customer
        // ------------------------------------------------

        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        // ------------------------------------------------
        // Only customers can cancel their own orders
        // ------------------------------------------------

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Only customers can cancel orders"})"
            );
        }

        // ------------------------------------------------
        // Load orders
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json orders =
            loadJsonFile("data/orders.json");

        // ------------------------------------------------
        // Find order
        // ------------------------------------------------

        for (auto& order : orders)
        {
            if (
                order.contains("id") &&
                order["id"].get<int>() == orderId
            )
            {
                // ----------------------------------------
                // Make sure order belongs to customer
                // ----------------------------------------

                if (
                    !order.contains("userId") ||
                    order["userId"].get<int>() !=
                        authenticatedUserId
                )
                {
                    return crow::response(
                        403,
                        R"({"status":"error","message":"You can only cancel your own orders"})"
                    );
                }

                // ----------------------------------------
                // Get current status
                // ----------------------------------------

                std::string currentStatus =
                    order.value(
                        "status",
                        ""
                    );

                // ----------------------------------------
                // Check whether cancellation is allowed
                // ----------------------------------------

                if (
                    currentStatus != "PLACED" &&
                    currentStatus != "CONFIRMED"
                )
                {
                    return crow::response(
                        400,
                        R"({"status":"error","message":"This order can no longer be cancelled"})"
                    );
                }

                // ----------------------------------------
                // Cancel order
                // ----------------------------------------

                order["status"] =
                    "CANCELLED";

                // ------------------------------------------------------------
// Linux IPC: notify the event logger
// ------------------------------------------------------------

std::stringstream eventStream;

eventStream
    << "ORDER_CANCELLED"
    << " | Order ID: " << orderId
    << " | User ID: " << authenticatedUserId;

sendOrderEvent(eventStream.str());

                // ----------------------------------------
                // Save orders
                // ----------------------------------------

                if (!saveJsonFile(
                        "data/orders.json",
                        orders))
                {
                    return crow::response(
                        500,
                        R"({"status":"error","message":"Unable to save cancelled order"})"
                    );
                }

                // ----------------------------------------
                // Response
                // ----------------------------------------

                json responseData;

                responseData["status"] =
                    "success";

                responseData["message"] =
                    "Order cancelled successfully";

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

        // ------------------------------------------------
        // Order not found
        // ------------------------------------------------

        return crow::response(
            404,
            R"({"status":"error","message":"Order not found"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to cancel order"})"
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
// FOOD REVIEWS & RATINGS
// ========================================================

// --------------------------------------------------------
// ADD REVIEW
// --------------------------------------------------------

CROW_ROUTE(app, "/api/reviews")
    .methods("POST"_method)
([](const crow::request& request)
{
    try
    {
        // ------------------------------------------------
        // Authenticate customer
        // ------------------------------------------------

        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Only customers can submit reviews"})"
            );
        }

        // ------------------------------------------------
        // Parse request
        // ------------------------------------------------

        json requestData =
            json::parse(request.body);

        int foodId =
            requestData.value("foodId", 0);

        int rating =
            requestData.value("rating", 0);

        std::string comment =
            requestData.value("comment", "");

        // ------------------------------------------------
        // Validate
        // ------------------------------------------------

        if (foodId <= 0)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Invalid food ID"})"
            );
        }

        if (rating < 1 || rating > 5)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Rating must be between 1 and 5"})"
            );
        }

        if (comment.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Review comment is required"})"
            );
        }

        // ------------------------------------------------
        // Verify food exists
        // ------------------------------------------------

        json foods =
            loadJsonFile(FOOD_FILE);

        bool foodFound = false;

        for (const auto& food : foods)
        {
            if (
                food.contains("id") &&
                food["id"].get<int>() == foodId
            )
            {
                foodFound = true;
                break;
            }
        }

        if (!foodFound)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Food not found"})"
            );
        }

        // ------------------------------------------------
        // Load reviews
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json reviews =
            loadJsonFile("data/reviews.json");

        // ------------------------------------------------
        // Prevent duplicate review
        // ------------------------------------------------

        for (const auto& review : reviews)
        {
            if (
                review.value("userId", 0) ==
                    authenticatedUserId &&
                review.value("foodId", 0) ==
                    foodId
            )
            {
                return crow::response(
                    409,
                    R"({"status":"error","message":"You have already reviewed this food"})"
                );
            }
        }

        // ------------------------------------------------
        // Generate review ID
        // ------------------------------------------------

        int newReviewId = 1;

        for (const auto& review : reviews)
        {
            if (review.contains("id"))
            {
                newReviewId = std::max(
                    newReviewId,
                    review["id"].get<int>() + 1
                );
            }
        }

        // ------------------------------------------------
        // Create review
        // ------------------------------------------------

        json newReview;

        newReview["id"] =
            newReviewId;

        newReview["userId"] =
            authenticatedUserId;

        newReview["foodId"] =
            foodId;

        newReview["rating"] =
            rating;

        newReview["comment"] =
            comment;

        // ------------------------------------------------
        // Save review
        // ------------------------------------------------

        reviews.push_back(newReview);

        if (!saveJsonFile(
                "data/reviews.json",
                reviews))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to save review"})"
            );
        }

        // ------------------------------------------------
        // Response
        // ------------------------------------------------

        json responseData;

        responseData["status"] =
            "success";

        responseData["message"] =
            "Review submitted successfully";

        responseData["review"] =
            newReview;

        crow::response response(
            201,
            responseData.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        return response;
    }
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid JSON request"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to submit review"})"
        );
    }
});


// --------------------------------------------------------
// GET REVIEWS FOR FOOD
// --------------------------------------------------------

CROW_ROUTE(app, "/api/foods/<int>/reviews")
    .methods("GET"_method)
([](int foodId)
{
    try
    {
        json reviews =
            loadJsonFile("data/reviews.json");

        json foodReviews =
            json::array();

        int totalRating = 0;

        for (const auto& review : reviews)
        {
            if (
                review.value("foodId", 0) ==
                foodId
            )
            {
                foodReviews.push_back(review);

                totalRating +=
                    review.value("rating", 0);
            }
        }

        double averageRating = 0.0;

        if (!foodReviews.empty())
        {
            averageRating =
                static_cast<double>(totalRating) /
                foodReviews.size();
        }

        json responseData;

        responseData["status"] =
            "success";

        responseData["foodId"] =
            foodId;

        responseData["averageRating"] =
            averageRating;

        responseData["reviewCount"] =
            foodReviews.size();

        responseData["reviews"] =
            foodReviews;

        crow::response response(
            200,
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
            500,
            R"({"status":"error","message":"Unable to load reviews"})"
        );
    }
});


// --------------------------------------------------------
// UPDATE REVIEW
// --------------------------------------------------------

CROW_ROUTE(app, "/api/reviews/<int>")
    .methods("PUT"_method)
([](const crow::request& request, int reviewId)
{
    try
    {
        // ------------------------------------------------
        // Authenticate customer
        // ------------------------------------------------

        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Only customers can update reviews"})"
            );
        }

        // ------------------------------------------------
        // Parse request
        // ------------------------------------------------

        json requestData =
            json::parse(request.body);

        int rating =
            requestData.value("rating", 0);

        std::string comment =
            requestData.value("comment", "");

        if (rating < 1 || rating > 5)
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Rating must be between 1 and 5"})"
            );
        }

        if (comment.empty())
        {
            return crow::response(
                400,
                R"({"status":"error","message":"Review comment is required"})"
            );
        }

        // ------------------------------------------------
        // Load reviews
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json reviews =
            loadJsonFile("data/reviews.json");

        for (auto& review : reviews)
        {
            if (
                review.value("id", 0) ==
                    reviewId
            )
            {
                // Only owner can update
                if (
                    review.value("userId", 0) !=
                    authenticatedUserId
                )
                {
                    return crow::response(
                        403,
                        R"({"status":"error","message":"You can only update your own review"})"
                    );
                }

                review["rating"] =
                    rating;

                review["comment"] =
                    comment;

                if (!saveJsonFile(
                        "data/reviews.json",
                        reviews))
                {
                    return crow::response(
                        500,
                        R"({"status":"error","message":"Unable to save review"})"
                    );
                }

                json responseData;

                responseData["status"] =
                    "success";

                responseData["message"] =
                    "Review updated successfully";

                responseData["review"] =
                    review;

                crow::response response(
                    200,
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
            R"({"status":"error","message":"Review not found"})"
        );
    }
    catch (const json::parse_error&)
    {
        return crow::response(
            400,
            R"({"status":"error","message":"Invalid JSON request"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to update review"})"
        );
    }
});


// --------------------------------------------------------
// DELETE REVIEW
// --------------------------------------------------------

CROW_ROUTE(app, "/api/reviews/<int>")
    .methods("DELETE"_method)
([](const crow::request& request, int reviewId)
{
    try
    {
        // ------------------------------------------------
        // Authenticate customer
        // ------------------------------------------------

        int authenticatedUserId = 0;
        std::string role;

        if (!authenticateRequest(
                request,
                authenticatedUserId,
                role))
        {
            return crow::response(
                401,
                R"({"status":"error","message":"Authentication required"})"
            );
        }

        if (role != "customer")
        {
            return crow::response(
                403,
                R"({"status":"error","message":"Only customers can delete reviews"})"
            );
        }

        // ------------------------------------------------
        // Load reviews
        // ------------------------------------------------

        std::lock_guard<std::mutex> lock(
            userFileMutex
        );

        json reviews =
            loadJsonFile("data/reviews.json");

        json updatedReviews =
            json::array();

        bool found = false;

        for (const auto& review : reviews)
        {
            if (
                review.value("id", 0) ==
                reviewId
            )
            {
                if (
                    review.value("userId", 0) !=
                    authenticatedUserId
                )
                {
                    return crow::response(
                        403,
                        R"({"status":"error","message":"You can only delete your own review"})"
                    );
                }

                found = true;
                continue;
            }

            updatedReviews.push_back(review);
        }

        if (!found)
        {
            return crow::response(
                404,
                R"({"status":"error","message":"Review not found"})"
            );
        }

        if (!saveJsonFile(
                "data/reviews.json",
                updatedReviews))
        {
            return crow::response(
                500,
                R"({"status":"error","message":"Unable to delete review"})"
            );
        }

        return crow::response(
            200,
            R"({"status":"success","message":"Review deleted successfully"})"
        );
    }
    catch (...)
    {
        return crow::response(
            500,
            R"({"status":"error","message":"Unable to delete review"})"
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
