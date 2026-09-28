#include "crow.h"
#include <iostream>
#include "crow/middlewares/cors.h"

int main()
{
    crow::App<crow::CORSHandler> app;

auto& cors = app.get_middleware<crow::CORSHandler>();

cors.global()
    .headers("Content-Type")
    .methods("GET"_method, "POST"_method);

    // Health check
    CROW_ROUTE(app, "/api/health")
    ([]()
    {
        crow::json::wvalue response;

        response["status"] = "success";
        response["message"] = "Food Ordering System backend is running";

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

    // Food menu
    CROW_ROUTE(app, "/api/foods")
    ([]()
    {
        crow::json::wvalue foods = crow::json::wvalue::list();

        foods[0]["id"] = 1;
        foods[0]["name"] = "Chicken Burger";
        foods[0]["description"] =
            "Delicious chicken burger with fresh ingredients.";
        foods[0]["price"] = 120;
        foods[0]["category"] = "Burger";
        foods[0]["image"] = "🍔";

        foods[1]["id"] = 2;
        foods[1]["name"] = "Veg Pizza";
        foods[1]["description"] =
            "Fresh vegetable pizza with delicious toppings.";
        foods[1]["price"] = 180;
        foods[1]["category"] = "Pizza";
        foods[1]["image"] = "🍕";

        foods[2]["id"] = 3;
        foods[2]["name"] = "Italian Pasta";
        foods[2]["description"] =
            "Creamy pasta prepared with fresh ingredients.";
        foods[2]["price"] = 150;
        foods[2]["category"] = "Pasta";
        foods[2]["image"] = "🍝";

        foods[3]["id"] = 4;
        foods[3]["name"] = "French Fries";
        foods[3]["description"] =
            "Crispy golden fries served hot and fresh.";
        foods[3]["price"] = 80;
        foods[3]["category"] = "Snacks";
        foods[3]["image"] = "🍟";

        foods[4]["id"] = 5;
        foods[4]["name"] = "Cold Coffee";
        foods[4]["description"] =
            "Refreshing cold coffee prepared with milk.";
        foods[4]["price"] = 70;
        foods[4]["category"] = "Drinks";
        foods[4]["image"] = "🥤";

        return foods;
    });

    std::cout << "====================================\n";
    std::cout << " Food Ordering System Backend\n";
    std::cout << " C++ HTTP Server\n";
    std::cout << "====================================\n";
    std::cout << "Server running on http://localhost:18080\n";

    app.port(18080).multithreaded().run();

    return 0;
}