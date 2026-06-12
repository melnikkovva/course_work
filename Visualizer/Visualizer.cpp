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
#include "../ProblemIO/ProblemIO.h"
#include <optional>

enum class ScreenState
{
    FileInput,
    Calculating,
    Results
};

static std::vector<Solution> RunOptimization(const Problem& problem)
{
    Mobso::Params params;

    params.populationSize = POPULATION_SIZE;
    params.maxEvaluations = MAX_EVALUATIONS;
    params.pg = PG;
    params.po = PO;
    params.pt = PT;
    params.seed = std::random_device{}();

    Mobso optimizer(problem, params);
    return optimizer.run();
}

static void DrawText(
    sf::RenderWindow& window,
    sf::Font& font,
    const sf::String& text,
    float x,
    float y,
    unsigned int size = 20)
{
    sf::Text drawable;
    drawable.setFont(font);
    drawable.setString(text);
    drawable.setCharacterSize(size);
    drawable.setFillColor(sf::Color::Black);
    drawable.setPosition(x, y);

    window.draw(drawable);
}

static sf::String Utf8(const std::string& text)
{
    return sf::String::fromUtf8(text.begin(), text.end());
}

void Visualizer::Show()
{
    sf::RenderWindow window(
        sf::VideoMode(1200, 700),
        Utf8("ЗАДАЧА ПЛАНИРОВАНИЯ СЛУЖБ ДОМАШНЕГО МЕДИЦИНСКОГО ОБСЛУЖИВАНИЯ")
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
    sf::String status = Utf8("Введите название файла и нажмите Enter");

    std::optional<Problem> problem;
    std::vector<Solution> paretoFront;

    int selectedSolution = 0;
    int selectedRouteIndex = -1;

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
                        DrawText(window, font, Utf8("Подсчёт результатов..."), 440.f, 320.f, 32);
                        window.display();

                        problem = ProblemIO::LoadFromFile(filename);
                        paretoFront = RunOptimization(*problem);

                        selectedSolution = 0;
                        selectedRouteIndex = -1;
                        state = ScreenState::Results;
                    }
                    catch (const std::exception& e)
                    {
                        status = Utf8("Ошибка: ") + Utf8(e.what());
                        state = ScreenState::FileInput;
                    }
                }

                if (state == ScreenState::Results)
                {
                    if (event.key.code == sf::Keyboard::Right &&
                        !paretoFront.empty())
                    {
                        if (event.key.shift && problem.has_value())
                        {
                            Decoder decoder(*problem);
                            std::vector<Route> routes = decoder.Decode(paretoFront[selectedSolution]);

                            if (!routes.empty())
                            {
                                selectedRouteIndex = (selectedRouteIndex + 1) % static_cast<int>(routes.size());
                            }
                        }
                        else
                        {
                            selectedSolution = (selectedSolution + 1) % static_cast<int>(paretoFront.size());
                            selectedRouteIndex = -1;
                        }
                    }

                    if (event.key.code == sf::Keyboard::Left && !paretoFront.empty())
                    {
                        if (event.key.shift && problem.has_value())
                        {
                            Decoder decoder(*problem);
                            std::vector<Route> routes = decoder.Decode(paretoFront[selectedSolution]);

                            if (!routes.empty())
                            {
                                selectedRouteIndex =
                                    (selectedRouteIndex - 1 +
                                     static_cast<int>(routes.size())) %
                                    static_cast<int>(routes.size());
                            }
                        }
                        else
                        {
                            selectedSolution =
                                (selectedSolution - 1 +
                                 static_cast<int>(paretoFront.size())) %
                                static_cast<int>(paretoFront.size());

                            selectedRouteIndex = -1;
                        }
                    }

                    if (event.key.code == sf::Keyboard::L)
                    {
                        filename.clear();
                        paretoFront.clear();
                        problem.reset();

                        status = Utf8("Введите название файла и нажмите Enter");

                        selectedSolution = 0;
                        selectedRouteIndex = -1;

                        state = ScreenState::FileInput;
                    }

                    if (event.key.code == sf::Keyboard::R &&
                        problem.has_value())
                    {
                        state = ScreenState::Calculating;

                        window.clear(sf::Color::White);
                        DrawText(window, font, Utf8("Пересчёт результатов..."), 430.f, 320.f, 32);
                        window.display();

                        paretoFront = RunOptimization(*problem);

                        selectedSolution = 0;
                        selectedRouteIndex = -1;

                        state = ScreenState::Results;
                    }

                    if (event.key.code == sf::Keyboard::E)
                    {
                        ProblemIO::SaveSolution(
                            *problem,
                            paretoFront[selectedSolution],
                            "solution.txt"
                        );
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

                selectedRouteIndex = -1;
            }
        }

        window.clear(sf::Color::White);

        if (state == ScreenState::FileInput)
        {
            DrawText(window, font, Utf8("Планирование служб домашнего медицинского обслуживания"), 60.f, 80.f, 34);

            DrawText(window, font, Utf8("Входной файл: "), 250.f, 220.f, 24);

            sf::RectangleShape inputBox(sf::Vector2f(600.f, 45.f));
            inputBox.setPosition(250.f, 260.f);
            inputBox.setFillColor(sf::Color::White);
            inputBox.setOutlineColor(sf::Color::Black);
            inputBox.setOutlineThickness(2.f);
            window.draw(inputBox);

            DrawText(window, font, filename, 260.f, 268.f, 22);

            DrawText(window, font, status, 250.f, 330.f, 20);

            DrawText(window, font, Utf8("Пример: problem.txt"), 250.f, 390.f, 20);

            DrawText(window, font, Utf8("Esc - выход"), 250.f, 430.f, 20);
        }

        if (state == ScreenState::Calculating)
        {
            DrawText(window, font, Utf8("Подсчёт результатов..."), 440.f, 320.f, 32);
        }

        if (state == ScreenState::Results)
        {
            if (problem.has_value() && !paretoFront.empty())
            {
                
                DrawText(
                    window,
                    font,
                    Utf8("E - экспорт решения"),
                    820.f,
                    625.f,
                    18
                );

                DrawMap(window, *problem, paretoFront[selectedSolution], selectedRouteIndex, font);
                DrawParetoFront(window, paretoFront);

                const Solution& solution = paretoFront[selectedSolution];

                DrawText(window, font, Utf8("Результаты"), 820.f, 370.f, 26);

                DrawText(
                    window,
                    font,
                    Utf8("Решение: ") +
                    std::to_string(selectedSolution + 1) +
                    " / " +
                    std::to_string(paretoFront.size()),
                    820.f,
                    410.f,
                    20
                );

                DrawText(
                    window,
                    font,
                    Utf8("Цена обслуживания: ") +
                    std::to_string(solution.GetObjectives().serviceCost),
                    820.f,
                    445.f,
                    20
                );

                DrawText(
                    window,
                    font,
                    Utf8("Цена задержек: ") +
                    std::to_string(solution.GetObjectives().delayCost),
                    820.f,
                    480.f,
                    20
                );

                DrawText(
                    window,
                    font,
                    Utf8("◀ / ▶ - переключить решение"),
                    820.f,
                    520.f,
                    18
                );

                DrawText(
                    window,
                    font,
                    Utf8("Shift + ◀ / ▶ - маршрут специалиста"),
                    820.f,
                    545.f,
                    18
                );

                DrawText(
                    window,
                    font,
                    Utf8("R - пересчитать решение"),
                    820.f,
                    570.f,
                    18
                );

                DrawText(
                    window,
                    font,
                    Utf8("L - загрузить другой файл"),
                    820.f,
                    600.f,
                    18
                );

                DrawText(
                    window,
                    font,
                    Utf8("Esc - выход"),
                    820.f,
                    650.f,
                    18
                );

                if (selectedRouteIndex != -1)
                {
                    Decoder decoder(*problem);
                    std::vector<Route> routes =
                        decoder.Decode(paretoFront[selectedSolution]);

                    if (selectedRouteIndex < static_cast<int>(routes.size()))
                    {
                        const Route& route = routes[selectedRouteIndex];

                        std::string path;

                        for (size_t i = 0; i < route.GetCustomers().size(); ++i)
                        {
                            if (i > 0)
                            {
                                path += " - ";
                            }

                            path += std::to_string(route.GetCustomers()[i]);
                        }

                        if (path.empty())
                        {
                            path = "нет клиентов";
                        }

                        DrawText(
                            window,
                            font,
                            Utf8("Выбран специалист: ") +
                            std::to_string(route.GetCaregiverId()),
                            820.f,
                            665.f,
                            16
                        );

                        DrawText(
                            window,
                            font,
                            Utf8("Маршрут: ") + Utf8(path),
                            50.f,
                            680.f,
                            18
                        );
                    }
                }
            }
            else
            {
                DrawText(
                    window,
                    font,
                    Utf8("Ни одного решения не нашлось"),
                    450.f,
                    320.f,
                    30
                );

                DrawText(
                    window,
                    font,
                    Utf8("Нажмите L чтобы загрузить другой файл"),
                    430.f,
                    370.f,
                    22
                );
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

void Visualizer::DrawParetoFront(
    sf::RenderWindow& window,
    const std::vector<Solution>& paretoFront)
{
    const float left = 820.f;
    const float top = 80.f;
    const float width = 320.f;
    const float height = 260.f;

    if (paretoFront.empty())
    {
        return;
    }

    double minService = std::numeric_limits<double>::max();
    double maxService = std::numeric_limits<double>::lowest();
    double minDelay = std::numeric_limits<double>::max();
    double maxDelay = std::numeric_limits<double>::lowest();

    for (const Solution& solution : paretoFront)
    {
        minService = std::min(minService, solution.GetObjectives().serviceCost);
        maxService = std::max(maxService, solution.GetObjectives().serviceCost);
        minDelay = std::min(minDelay, solution.GetObjectives().delayCost);
        maxDelay = std::max(maxDelay, solution.GetObjectives().delayCost);
    }

    sf::RectangleShape border(sf::Vector2f(width, height));
    border.setPosition(left, top);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::Black);
    border.setOutlineThickness(2.f);
    window.draw(border);

    for (const Solution& solution : paretoFront)
    {
        sf::Vector2f pointPosition = ScalePoint(
            solution.GetObjectives().delayCost,
            solution.GetObjectives().serviceCost,
            minDelay,
            maxDelay,
            minService,
            maxService,
            left,
            top,
            width,
            height
        );

        sf::CircleShape point(5.f);
        point.setOrigin(5.f, 5.f);
        point.setPosition(pointPosition);
        point.setFillColor(sf::Color::Red);
        window.draw(point);
    }
}

void Visualizer::DrawMap(
    sf::RenderWindow& window,
    const Problem& problem,
    const Solution& solution,
    int selectedRouteIndex,
    sf::Font& font)
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
        sf::Color::Blue,
        sf::Color::Green,
        sf::Color::Magenta,
        sf::Color::Cyan,
        sf::Color(255, 140, 0),
        sf::Color(120, 60, 200)
    };

    for (size_t r = 0; r < routes.size(); ++r)
    {
        if (selectedRouteIndex != -1 &&
            static_cast<int>(r) != selectedRouteIndex)
        {
            continue;
        }

        const Route& route = routes[r];

        if (route.GetCustomers().empty())
        {
            continue;
        }

        const Caregiver& caregiver =
            problem.GetCaregiverById(route.GetCaregiverId());

        const CareCenter& center =
            problem.GetCenterById(caregiver.GetCenterId());

        sf::Color color = colors[r % colors.size()];

        sf::Vector2f previous = ScalePoint(center.GetX(), center.GetY(),
                                minX, maxX, minY, maxY, left, top, width, height);

        for (int customerId : route.GetCustomers())
        {
            const Customer& customer =
                problem.GetCustomerById(customerId);

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

        sf::RectangleShape square(sf::Vector2f(18.f, 18.f));
        square.setOrigin(9.f, 9.f);
        square.setPosition(p);
        square.setFillColor(sf::Color::Red);
        window.draw(square);

        DrawText(
            window,
            font,
            std::to_string(center.GetId()),
            p.x + 10.f,
            p.y - 12.f,
            16
        );
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

    if (selectedRouteIndex != -1 &&
        selectedRouteIndex < static_cast<int>(routes.size()))
    {
        const Route& route = routes[selectedRouteIndex];

        DrawText(
            window,
            font,
            Utf8("Показан маршрут специалиста: ") +
            std::to_string(route.GetCaregiverId()),
            50.f,
            650.f,
            18
        );
    }
}