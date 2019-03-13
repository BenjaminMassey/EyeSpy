#include <SFML/Graphics.hpp>
#include <cmath>

bool inRadius(float radius, sf::Vector2i mousePos, sf::Vector2f targetPos, sf::Vector2f targetSize) {
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
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML works!", sf::Style::Default);
    sf::Vector2u windowSize = window.getSize();

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
    sf::RectangleShape target(sf::Vector2f(20.0f, 20.0f));
    target.setPosition(std::rand() % (windowSize.x - 20) + 1, std::rand() % (windowSize.y - 20) + 1);
    target.setFillColor(sf::Color::Red);

    // Font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
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

    // Setup calibration box
    int state = 0;
    sf::RectangleShape calibration(sf::Vector2f(50.0f, 50.0f));
    calibration.setOrigin(25.0f, 25.0f);
    calibration.setPosition(25.0f + 1.0f, 25.0f + 1.0f);
    calibration.setFillColor(sf::Color::Transparent);
    calibration.setOutlineColor(sf::Color::Red);
    calibration.setOutlineThickness(1);

    // Calibration message
    sf::Text calibrateMessage;
    calibrateMessage.setFont(font);
    calibrateMessage.setCharacterSize(16);
    calibrateMessage.setString("Look at the red squares and press space to calibrate the eye tracking");
    calibrateMessage.setFillColor(sf::Color::White);
    calibrateMessage.setOutlineColor(sf::Color::Black);
    calibrateMessage.setOutlineThickness(1);
    sf::FloatRect cmBoundingBox = calibrateMessage.getLocalBounds();
    calibrateMessage.setOrigin(cmBoundingBox.left + cmBoundingBox.width / 2, cmBoundingBox.top + cmBoundingBox.height / 2);
    calibrateMessage.setPosition(windowSize.x / 2, windowSize.y / 4);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseMoved) {
                viewport.setPosition(event.mouseMove.x, event.mouseMove.y);
            }

            if (event.type == sf::Event::KeyPressed) {
            	if (event.key.code == sf::Keyboard::Space) {
            		switch(state) {
            			case 0:
            				calibration.setPosition(windowSize.x / 2, 25.0f + 1.0f);
            				state = 1;
            				break;
            			case 1:
            				calibration.setPosition(windowSize.x - 25.0f - 1.0f, 25.0f + 1.0f);
            				state = 2;
            				break;
            			case 2:
            				calibration.setPosition(25.0f + 1.0f, windowSize.y / 2);
            				state = 3;
            				break;
            			case 3:
            				calibration.setPosition(windowSize.x / 2, windowSize.y / 2);
            				state = 4;
            				break;
            			case 4:
            				calibration.setPosition(windowSize.x - 25.0f - 1.0f, windowSize.y / 2);
            				state = 5;
            				break;
            			case 5:
            				calibration.setPosition(25.0f + 1.0f, windowSize.y - 25.0f - 1.0f);
            				state = 6;
            				break;
            			case 6:
            				calibration.setPosition(windowSize.x / 2, windowSize.y - 25.0f - 1.0f);
            				state = 7;
            				break;
            			case 7:
            				calibration.setPosition(windowSize.x - 25.0f - 1.0f, windowSize.y - 25.0f - 1.0f);
            				state = 8;
            				break;
            			default:
            				state = -1;
            				break;
            		}
            	}
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

        if (state != -1) {
        	window.draw(calibration);
        	window.draw(calibrateMessage);
        } else {
	        window.draw(target);
	        window.draw(viewport);
	        window.draw(score);
	    }
	    window.display();
    }

    return 0;
}