#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "../Problem/Problem.h"
#include "../Solution/Solution.h"
#include "../Route/Route.h"

class Visualizer
{
public:
    static void Show(
        const Problem& problem,
        const std::vector<Solution>& paretoFront
    );

private:
    static void DrawMap(
        sf::RenderWindow& window,
        const Problem& problem,
        const Solution& solution
    );

    static void DrawParetoFront(
        sf::RenderWindow& window,
        const std::vector<Solution>& paretoFront
    );

    static sf::Vector2f ScalePoint(
        double x, double y, double minX,
        double maxX, double minY, double maxY,
        float left, float top, float width, float height
    );
};