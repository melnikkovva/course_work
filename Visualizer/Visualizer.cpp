#include "Visualizer.h"
#include <algorithm>
#include <exception>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include "../Decoder/Decoder.h"
#include "../CareCenter/CareCenter.h"
#include "../CareGiver/CareGiver.h"
#include "../Customer/Customer.h"
void Visualizer::Show(const Problem& problem, const std::vector<Solution>& paretoFront)
{
    if (paretoFront.empty())
    {
        std::cout << "Pareto front is empty. Nothing to visualize.\n";
        return;
    }

    sf::RenderWindow window(sf::VideoMode(1200, 700), "Home Health Care");

    window.setFramerateLimit(60);
    int selectedSolution = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Right)
                {
                    selectedSolution = (selectedSolution + 1) % static_cast<int>(paretoFront.size());
                }

                if (event.key.code == sf::Keyboard::Left)
                {
                    selectedSolution = (selectedSolution - 1 + static_cast<int>(paretoFront.size())) %
                                        static_cast<int>(paretoFront.size());
                }
            }
        }

        window.clear(sf::Color::White);

        DrawMap(window, problem, paretoFront[selectedSolution]);
        DrawParetoFront(window, paretoFront);

        window.display();
    }
}

sf::Vector2f Visualizer::ScalePoint(
    double x, double y, double minX,
    double maxX, double minY, double maxY,
    float left, float top, float width, float height)
{
    double dx = (std::abs(maxX - minX) < 1e-6) ? 1.0 : (maxX - minX);
    double dy = (std::abs(maxY - minY) < 1e-6) ? 1.0 : (maxY - minY);
    float sx = left + (std::abs(maxX - minX) < 1e-6 ? 0.5f : static_cast<float>((x - minX) / dx)) * width;
    float sy = top + height - (std::abs(maxY - minY) < 1e-6 ? 0.5f : static_cast<float>((y - minY) / dy)) * height;

    return sf::Vector2f(sx, sy);
}

void Visualizer::DrawMap(sf::RenderWindow& window,
    const Problem& problem, const Solution& solution)
{
    const float left = 40.f;
    const float top = 40.f;
    const float width = 720.f;
    const float height = 600.f;

    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto& c : problem.customers())
    {
        minX = std::min(minX, c.GetX());
        maxX = std::max(maxX, c.GetX());
        minY = std::min(minY, c.GetY());
        maxY = std::max(maxY, c.GetY());
    }

    for (const auto& center : problem.centers())
    {
        minX = std::min(minX, center.GetX());
        maxX = std::max(maxX, center.GetX());
        minY = std::min(minY, center.GetY());
        maxY = std::max(maxY, center.GetY());
    }

    minX -= 2;
    maxX += 2;
    minY -= 2;
    maxY += 2;

    sf::RectangleShape border(sf::Vector2f(width, height));
    border.setPosition(left, top);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::Black);
    border.setOutlineThickness(2.f);
    window.draw(border);

    Decoder decoder(problem);
    std::vector<Route> routes = decoder.Decode(solution);

    std::vector<sf::Color> colors =
    {
        sf::Color::Red,
        sf::Color::Blue,
        sf::Color::Green,
        sf::Color::Magenta,
        sf::Color::Cyan,
        sf::Color(255, 140, 0),
        sf::Color(120, 60, 200)
    };

    for (size_t r = 0; r < routes.size(); ++r)
    {
        const Route& route = routes[r];

        if (route.GetCustomers().empty()) continue;

        const Caregiver& caregiver = problem.GetCaregiverById(route.GetCaregiverId());
        const CareCenter& center = problem.GetCenterById(caregiver.GetCenterId());

        sf::Color color = colors[r % colors.size()];

        sf::Vector2f previous = ScalePoint(center.GetX(), center.GetY(),
                                minX, maxX, minY, maxY, left, top, width, height);

        for (int customerId : route.GetCustomers())
        {
            const Customer& customer = problem.GetCustomerById(customerId);

            sf::Vector2f current = ScalePoint(customer.GetX(), customer.GetY(),
                minX, maxX, minY, maxY, left, top, width, height );

            sf::Vertex line[] =
            {
                sf::Vertex(previous, color),
                sf::Vertex(current, color)
            };

            window.draw(line, 2, sf::Lines);

            previous = current;
        }

        sf::Vector2f centerPoint = ScalePoint(center.GetX(), center.GetY(),
            minX, maxX, minY, maxY, left, top, width, height);

        sf::Vertex returnLine[] =
        {
            sf::Vertex(previous, color),
            sf::Vertex(centerPoint, color)
        };

        window.draw(returnLine, 2, sf::Lines);
    }

    for (const auto& center : problem.centers())
    {
        sf::Vector2f p = ScalePoint(center.GetX(), center.GetY(),
                                    minX, maxX, minY, maxY, left, top, width, height);

        sf::RectangleShape square(sf::Vector2f(16.f, 16.f));
        square.setOrigin(8.f, 8.f);
        square.setPosition(p);
        square.setFillColor(sf::Color::Black);
        window.draw(square);
    }

    for (const auto& customer : problem.customers())
    {
        sf::Vector2f p = ScalePoint(customer.GetX(), customer.GetY(),
                                    minX, maxX, minY, maxY, left, top, width, height);

        sf::CircleShape circle(6.f);
        circle.setOrigin(6.f, 6.f);
        circle.setPosition(p);
        circle.setFillColor(sf::Color::White);
        circle.setOutlineColor(sf::Color::Black);
        circle.setOutlineThickness(2.f);
        window.draw(circle);
    }
}

void Visualizer::DrawParetoFront(sf::RenderWindow& window, const std::vector<Solution>& paretoFront)
{
    const float left = 820.f;
    const float top = 80.f;
    const float width = 320.f;
    const float height = 260.f;

    if (paretoFront.empty()) return;

    double minService = std::numeric_limits<double>::max();
    double maxService = std::numeric_limits<double>::lowest();
    double minDelay = std::numeric_limits<double>::max();
    double maxDelay = std::numeric_limits<double>::lowest();

    for (const Solution& s : paretoFront)
    {
        minService = std::min(minService, s.GetObjectives().serviceCost);
        maxService = std::max(maxService, s.GetObjectives().serviceCost);
        minDelay = std::min(minDelay, s.GetObjectives().delayCost);
        maxDelay = std::max(maxDelay, s.GetObjectives().delayCost);
    }

    sf::RectangleShape border(sf::Vector2f(width, height));
    border.setPosition(left, top);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::Black);
    border.setOutlineThickness(2.f);
    window.draw(border);

    for (const Solution& s : paretoFront)
    {
        sf::Vector2f p = ScalePoint(s.GetObjectives().delayCost,
            s.GetObjectives().serviceCost, minDelay, maxDelay,
            minService, maxService, left, top, width, height);

        sf::CircleShape point(5.f);
        point.setOrigin(5.f, 5.f);
        point.setPosition(p);
        point.setFillColor(sf::Color::Red);
        window.draw(point);
    }
}