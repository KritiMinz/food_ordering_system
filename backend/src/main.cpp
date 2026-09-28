#include "crow.h"
#include "crow/middlewares/cors.h"
#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

const std::string FOOD_FILE = "data/foods.json";

// Load food data from JSON file
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
    // Enable CORS
    crow::App<crow::CORSHandler> app;

    auto& cors = app.get_middleware<crow::CORSHandler>();

    cors.global()
        .origin("http://localhost:5173")
        .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method)
        .headers("Content-Type");

    // Health check
    CROW_ROUTE(app, "/api/health")
    ([]()
    {
        crow::json::wvalue response;

        response["status"] = "success";
        response["message"] =
            "Food Ordering System backend is running";

        return response;
    });

    // Application information
    CROW_ROUTE(app, "/api")
    ([]()
    {
        crow::json::wvalue response;

        response["application"] = "Food Ordering System";
        response["backend"] = "C++";
        response["version"] = "1.0";

        return response;
    });

    // Food menu loaded from JSON file
    CROW_ROUTE(app, "/api/foods")
    ([]()
    {
        json foods = loadFoods();

        crow::json::wvalue response =
            crow::json::load(foods.dump());

        return response;
    });

    std::cout << "====================================\n";
    std::cout << " Food Ordering System Backend\n";
    std::cout << " C++ HTTP Server\n";
    std::cout << " File-based Food Persistence\n";
    std::cout << " CORS Enabled\n";
    std::cout << "====================================\n";
    std::cout << "Server running on http://localhost:18080\n";

    app.port(18080)
       .multithreaded()
       .run();

    return 0;
}