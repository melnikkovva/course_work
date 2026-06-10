#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "../Problem/Problem.h"
#include "../Solution/Solution.h"

class Visualizer
{
public:
    static void Show();

private:
    static sf::Vector2f ScalePoint(
        double x, double y,
        double minX, double maxX,
        double minY, double maxY,
        float left, float top,
        float width, float height
    );

    static void DrawMap(
        sf::RenderWindow& window,
        const Problem& problem,
        const Solution& solution,
        int selectedRouteIndex,
        sf::Font& font
    );

    static void DrawParetoFront(
        sf::RenderWindow& window,
        const std::vector<Solution>& paretoFront
    );
};