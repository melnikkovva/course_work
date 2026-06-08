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
#include "../Mobso/Mobso.h"
#include "../ProblemLoader/ProblemLoader.h"
#include <optional>

static void AddCustomerFromConsole(Problem& problem)
{
    int id;
    double x, y;
    int requiredSkill;
    double serviceTime;
    double appointmentTime;
    double baseRate;

    std::cout << "Customer id: ";
    std::cin >> id;

    std::cout << "x y: ";
    std::cin >> x >> y;

    std::cout << "required skill: ";
    std::cin >> requiredSkill;

    std::cout << "service time: ";
    std::cin >> serviceTime;

    std::cout << "appointment time: ";
    std::cin >> appointmentTime;

    std::cout << "base rate: ";
    std::cin >> baseRate;

    problem.AddCustomer(
        Customer(
            id,
            x,
            y,
            requiredSkill,
            serviceTime,
            appointmentTime,
            baseRate
        )
    );
}

static void RemoveCustomerFromConsole(Problem& problem)
{
    int id;

    std::cout << "Customer id to remove: ";
    std::cin >> id;

    problem.RemoveCustomerById(id);
}

static void AddCaregiverFromConsole(Problem& problem)
{
    int id;
    int centerId;
    int skill;

    std::cout << "Caregiver id: ";
    std::cin >> id;

    std::cout << "center id: ";
    std::cin >> centerId;

    std::cout << "skill: ";
    std::cin >> skill;

    problem.AddCaregiver(Caregiver(id, centerId, skill));
}

static void RemoveCaregiverFromConsole(Problem& problem)
{
    int id;

    std::cout << "Caregiver id to remove: ";
    std::cin >> id;

    problem.RemoveCaregiverById(id);
}

static void AddCenterFromConsole(Problem& problem)
{
    int id;
    double x, y;
    int capacity;

    std::cout << "Center id: ";
    std::cin >> id;

    std::cout << "x y: ";
    std::cin >> x >> y;

    std::cout << "capacity: ";
    std::cin >> capacity;

    problem.AddCenter(CareCenter(id, x, y, capacity));
}

static void RemoveCenterFromConsole(Problem& problem)
{
    int id;

    std::cout << "Center id to remove: ";
    std::cin >> id;

    problem.RemoveCenterById(id);
}

enum class ScreenState
{
    FileInput,
    Calculating,
    Results
};

static std::vector<Solution> RunOptimization(const Problem& problem)
{
    Mobso::Params params;

    params.populationSize = 100;
    params.maxEvaluations = 1000;
    params.pg = 0.6;
    params.po = 0.6;
    params.pt = 0.2;
    params.seed = 42;

    Mobso optimizer(problem, params);
    return optimizer.run();
}

static void DrawText(
    sf::RenderWindow& window,
    sf::Font& font,
    const std::string& text,
    float x,
    float y,
    unsigned int size = 22)
{
    sf::Text drawable;
    drawable.setFont(font);
    drawable.setString(text);
    drawable.setCharacterSize(size);
    drawable.setFillColor(sf::Color::Black);
    drawable.setPosition(x, y);
    window.draw(drawable);
}

void Visualizer::Show()
{
    sf::RenderWindow window(
        sf::VideoMode(1200, 700),
        "Home Health Care Scheduler"
    );

    window.setFramerateLimit(60);

    sf::Font font;

    if (!font.loadFromFile("assets/arial.ttf"))
    {
        std::cout << "Cannot load font: assets/arial.ttf\n";
        return;
    }

    ScreenState state = ScreenState::FileInput;

    std::string filename;
    std::string status = "Enter file name and press Enter";

    std::optional<Problem> problem;
    std::vector<Solution> paretoFront;

    int selectedSolution = 0;

    while (window.isOpen())
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::TextEntered &&
                state == ScreenState::FileInput)
            {
                if (event.text.unicode == '\b')
                {
                    if (!filename.empty())
                    {
                        filename.pop_back();
                    }
                }
                else if (event.text.unicode >= 32 &&
                         event.text.unicode < 128)
                {
                    filename += static_cast<char>(event.text.unicode);
                }
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }

                if (state == ScreenState::FileInput &&
                    event.key.code == sf::Keyboard::Enter)
                {
                    try
                    {
                        state = ScreenState::Calculating;

                        window.clear(sf::Color::White);
                        DrawText(window, font, "Calculating...", 480.f, 320.f, 32);
                        window.display();

                        problem = ProblemLoader::LoadFromFile(filename);
                        paretoFront = RunOptimization(*problem);

                        selectedSolution = 0;
                        state = ScreenState::Results;
                    }
                    catch (const std::exception& e)
                    {
                        status = std::string("Error: ") + e.what();
                        state = ScreenState::FileInput;
                    }
                }

                if (state == ScreenState::Results)
                {
                    if (event.key.code == sf::Keyboard::Right &&
                        !paretoFront.empty())
                    {
                        selectedSolution =
                            (selectedSolution + 1) %
                            static_cast<int>(paretoFront.size());
                    }

                    if (event.key.code == sf::Keyboard::Left &&
                        !paretoFront.empty())
                    {
                        selectedSolution =
                            (selectedSolution - 1 +
                             static_cast<int>(paretoFront.size())) %
                            static_cast<int>(paretoFront.size());
                    }

                    if (event.key.code == sf::Keyboard::L)
                    {
                        filename.clear();
                        paretoFront.clear();
                        problem.reset();
                        status = "Enter file name and press Enter";
                        state = ScreenState::FileInput;
                    }

                    if (event.key.code == sf::Keyboard::R && problem.has_value())
                    {
                        state = ScreenState::Calculating;

                        window.clear(sf::Color::White);
                        DrawText(window, font, "Recalculating...", 460.f, 320.f, 32);
                        window.display();

                        paretoFront = RunOptimization(*problem);
                        selectedSolution = 0;
                        state = ScreenState::Results;
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left &&
                state == ScreenState::Results &&
                !paretoFront.empty())
            {
                selectedSolution =
                    (selectedSolution + 1) %
                    static_cast<int>(paretoFront.size());
            }
        }

        window.clear(sf::Color::White);

        if (state == ScreenState::FileInput)
        {
            DrawText(window, font, "Home Health Care Scheduler", 360.f, 80.f, 34);

            DrawText(window, font, "Input file:", 250.f, 220.f, 24);

            sf::RectangleShape inputBox(sf::Vector2f(600.f, 45.f));
            inputBox.setPosition(250.f, 260.f);
            inputBox.setFillColor(sf::Color::White);
            inputBox.setOutlineColor(sf::Color::Black);
            inputBox.setOutlineThickness(2.f);
            window.draw(inputBox);

            DrawText(window, font, filename, 260.f, 268.f, 22);

            DrawText(window, font, status, 250.f, 330.f, 20);

            DrawText(window, font, "Example: problem.txt", 250.f, 390.f, 20);
            DrawText(window, font, "Enter - load and solve", 250.f, 430.f, 20);
            DrawText(window, font, "Esc - exit", 250.f, 470.f, 20);
        }

        if (state == ScreenState::Calculating)
        {
            DrawText(window, font, "Calculating...", 480.f, 320.f, 32);
        }

        if (state == ScreenState::Results)
        {
            if (problem.has_value() && !paretoFront.empty())
            {
                DrawMap(window, *problem, paretoFront[selectedSolution]);
                DrawParetoFront(window, paretoFront);

                const Solution& solution = paretoFront[selectedSolution];

                DrawText(window, font, "Results", 820.f, 370.f, 26);

                DrawText(
                    window,
                    font,
                    "Solution: " + std::to_string(selectedSolution + 1) +
                    " / " + std::to_string(paretoFront.size()),
                    820.f,
                    410.f,
                    20
                );

                DrawText(
                    window,
                    font,
                    "Service cost: " +
                    std::to_string(solution.GetObjectives().serviceCost),
                    820.f,
                    445.f,
                    20
                );

                DrawText(
                    window,
                    font,
                    "Delay cost: " +
                    std::to_string(solution.GetObjectives().delayCost),
                    820.f,
                    480.f,
                    20
                );

                DrawText(window, font, "Left / Right - switch solution", 820.f, 540.f, 18);
                DrawText(window, font, "Mouse click - next solution", 820.f, 565.f, 18);
                DrawText(window, font, "R - recalculate", 820.f, 590.f, 18);
                DrawText(window, font, "L - load another file", 820.f, 615.f, 18);
                DrawText(window, font, "Esc - exit", 820.f, 640.f, 18);
            }
            else
            {
                DrawText(window, font, "No solutions found", 450.f, 320.f, 30);
                DrawText(window, font, "Press L to load another file", 430.f, 370.f, 22);
            }
        }

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

