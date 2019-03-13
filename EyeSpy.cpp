#include <SFML/Graphics.hpp>
#include <random>
#include <functional>

int main() {
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML works!", sf::Style::Default, settings);
    sf::Vector2u windowSize = window.getSize();

    sf::CircleShape viewport(50.0f);
    viewport.setOrigin(50.0f, 50.0f);
    viewport.setFillColor(sf::Color(0, 0, 0, 0));
    viewport.setOutlineColor(sf::Color::Black);
    viewport.setOutlineThickness(windowSize.x + windowSize.y);

    sf::RectangleShape player(sf::Vector2f(20.0f, 20.0f));
    player.setFillColor(sf::Color::Red);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distributionX(0, windowSize.x - 20);
    std::uniform_int_distribution<int> distributionY(0, windowSize.y - 20);
    auto randX = std::bind(distributionX, generator);
    auto randY = std::bind(distributionY, generator);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseMoved) {
                viewport.setPosition(event.mouseMove.x, event.mouseMove.y);
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f playerPos = player.getPosition();
            sf::Vector2f playerSize = player.getSize();
            if (mousePos.x > playerPos.x && mousePos.x < playerPos.x + playerSize.x && mousePos.y > playerPos.y && mousePos.y < playerPos.y + playerSize.y) {
                printf("CLICKED\n");
                player.setPosition(randX(), randY());
            }
        }

        window.clear(sf::Color::White);
        window.draw(player);
        window.draw(viewport);
        window.display();
    }

    return 0;
}