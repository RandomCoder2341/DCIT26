#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm> 
#include <cctype>    
#include "httplib.h" 
#include "json.hpp"  

using namespace httplib;
using json = nlohmann::json;

int main() {
    Server svr;

    // 1. CORS Setup
    svr.set_pre_routing_handler([](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS, GET");
        if (req.method == "OPTIONS") {
            res.status = 204;
            return Server::HandlerResponse::Handled;
        }
        return Server::HandlerResponse::Unhandled;
    });

    // 2. Health Check Route (Wakes up the server)
    svr.Get("/health", [](const Request& req, Response& res) {
        res.set_content("OK", "text/plain");
    });

    // 3. Your Grading Route
    svr.Post("/api/grade", [](const Request& req, Response& res) {
        try {
            auto data = json::parse(req.body);
            auto answers = data["answers"];
            auto correctAnswers = data["correctAnswers"];
            
            int score = 0;

            // Helper to make strings lowercase for case-insensitive grading
            auto toLower = [](std::string s) {
                std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
                return s;
            };
            
            for (size_t i = 0; i < answers.size(); ++i) {
                std::string userStr = toLower(answers[i].get<std::string>());
                std::string correctStr = toLower(correctAnswers[i].get<std::string>());

                if (userStr == correctStr && userStr != "") {
                    score++;
                }
            }
            
            int total = correctAnswers.size();
            std::string remark = (score == total) ? "Perfect!" : (score > total / 2) ? "Good job!" : "Keep studying!";
            std::string color = (score > total / 2) ? "green" : "red";

            json response = {
                {"score", score},
                {"total", total},
                {"remark", remark},
                {"color", color}
            };
            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content("{\"error\": \"Invalid JSON\"}", "application/json");
        }
    });

    // 4. Port & Host Binding (For Render)
    int port = 8080;
    if (const char* p = std::getenv("PORT")) {
        port = std::stoi(p);
    }

    std::cout << "Server starting on port " << port << std::endl;
    svr.listen("0.0.0.0", port);
    
    return 0;
}