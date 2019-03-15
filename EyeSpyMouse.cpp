#include <SFML/Graphics.hpp>
#include <cmath>

bool inRadius(float radius, sf::Vector2i mousePos, sf::Vector2f targetPos, sf::Vector2f targetSize) {
    // Use distance equation to make sure that every corner of the target is inside the radius of the circle
	if (sqrt(pow(targetPos.x - mousePos.x, 2) + pow(targetPos.y - mousePos.y, 2)) > radius) {
		return false;
	} else if (sqrt(pow((targetPos.x + targetSize.x) - mousePos.x, 2) + pow(targetPos.y - mousePos.y, 2)) > radius) {
		return false;
	} else if (sqrt(pow((targetPos.x + targetSize.x) - mousePos.x, 2) + pow((targetPos.y + targetSize.y) - mousePos.y, 2)) > radius) {
		return false;
	} else if (sqrt(pow((targetPos.x + targetSize.x) - mousePos.x, 2) + pow((targetPos.y + targetSize.y) - mousePos.y, 2)) > radius) {
		return false;
	}

	return true;
}

int main() {
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "EyeSpy (Mouse)", sf::Style::Default);
    sf::Vector2u windowSize = window.getSize();

    // Setup background image
    sf::Texture backgroundTexture;
    backgroundTexture.loadFromFile("./images/waldoBackground.png");
    sf::Sprite background;
    background.setScale(0.25f, 0.25f);
    background.setTexture(backgroundTexture);

    // Setup viewport
    sf::CircleShape viewport(50.0f);
    viewport.setOrigin(50.0f, 50.0f);
    viewport.setPosition(windowSize.x / 2, windowSize.y / 2);
    viewport.setFillColor(sf::Color(0, 0, 0, 0));
    viewport.setOutlineColor(sf::Color::Black);
    viewport.setOutlineThickness(windowSize.x + windowSize.y);

    // Setup rng seed for random placement of target
    std::srand(std::time(NULL));

    // Setup target to click
    sf::RectangleShape target(sf::Vector2f(28.0f, 48.0f));
    target.setPosition(std::rand() % (windowSize.x - 20) + 1, std::rand() % (windowSize.y - 20) + 1);

    // Add Texture
    sf::Texture targetTexture;
    targetTexture.loadFromFile("./images/waldo.png");
    target.setTexture(&targetTexture);

    // Font
    sf::Font font;
    if (!font.loadFromFile("./fonts/arial.ttf"))
    {
      printf("OOF\n");
    }

    // Score
    int points = 0;
    sf::Text score;
    score.setFont(font);
    score.setCharacterSize(24);
    score.setString("Score: 0");
    score.setFillColor(sf::Color::White);
    score.setOutlineColor(sf::Color::Black);
    score.setOutlineThickness(1);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseMoved) {
                viewport.setPosition(event.mouseMove.x, event.mouseMove.y);
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f targetPos = target.getPosition();
            sf::Vector2f targetSize = target.getSize();
            if (inRadius(viewport.getRadius(), mousePos, targetPos, targetSize)) {
                points++;
                score.setString("Score: " + std::to_string(points));
                target.setPosition(std::rand() % (windowSize.x - 20) + 1, std::rand() % (windowSize.y - 20) + 1);
            }
        }

        window.clear(sf::Color::White);

        window.draw(background);
        window.draw(target);
        window.draw(viewport);
        window.draw(score);

        window.display();
    }

    return 0;
}