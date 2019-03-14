#include <SFML/Graphics.hpp>
#include <cmath>

#include <iostream>
#include <thread>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

std::vector<cv::Point> centers;
cv::Point lastPoint;
cv::Point mousePoint;

cv::Vec3f getEyeball(cv::Mat &eye, std::vector<cv::Vec3f> &circles)
{
  std::vector<int> sums(circles.size(), 0);
  for (int y = 0; y < eye.rows; y++)
  {
      uchar *ptr = eye.ptr<uchar>(y);
      for (int x = 0; x < eye.cols; x++)
      {
          int value = static_cast<int>(*ptr);
          for (int i = 0; i < circles.size(); i++)
          {
              cv::Point center((int)std::round(circles[i][0]), (int)std::round(circles[i][1]));
              int radius = (int)std::round(circles[i][2]);
              if (std::pow(x - center.x, 2) + std::pow(y - center.y, 2) < std::pow(radius, 2))
              {
                  sums[i] += value;
              }
          }
          ++ptr;
      }
  }
  int smallestSum = 9999999;
  int smallestSumIndex = -1;
  for (int i = 0; i < circles.size(); i++)
  {
      if (sums[i] < smallestSum)
      {
          smallestSum = sums[i];
          smallestSumIndex = i;
      }
  }
  return circles[smallestSumIndex];
}

cv::Rect getLeftmostEye(std::vector<cv::Rect> &eyes)
{
  int leftmost = 99999999;
  int leftmostIndex = -1;
  for (int i = 0; i < eyes.size(); i++)
  {
      if (eyes[i].tl().x < leftmost)
      {
          leftmost = eyes[i].tl().x;
          leftmostIndex = i;
      }
  }
  return eyes[leftmostIndex];
}

cv::Point stabilize(std::vector<cv::Point> &points, int windowSize)
{
  float sumX = 0;
  float sumY = 0;
  int count = 0;
  for (int i = std::max(0, (int)(points.size() - windowSize)); i < points.size(); i++)
  {
      sumX += points[i].x;
      sumY += points[i].y;
      ++count;
  }
  if (count > 0)
  {
      sumX /= count;
      sumY /= count;
  }
  return cv::Point(sumX, sumY);
}

void detectEyes(cv::Mat &frame, cv::CascadeClassifier &faceCascade, cv::CascadeClassifier &eyeCascade)
{
  cv::Mat grayscale;
  cv::cvtColor(frame, grayscale, cv::COLOR_RGB2GRAY); // convert image to grayscale
  cv::equalizeHist(grayscale, grayscale); // enhance image contrast 
  std::vector<cv::Rect> faces;
  faceCascade.detectMultiScale(grayscale, faces, 1.1, 2, 0 | 2, cv::Size(150, 150));
  if (faces.size() == 0) return; // none face was detected
  cv::Mat face = grayscale(faces[0]); // crop the face
  std::vector<cv::Rect> eyes;
  eyeCascade.detectMultiScale(face, eyes, 1.1, 2, 0 | 2, cv::Size(30, 30)); // same thing as above    
  rectangle(frame, faces[0].tl(), faces[0].br(), cv::Scalar(255, 0, 0), 2);
  if (eyes.size() != 2) return; // both eyes were not detected
  for (cv::Rect &eye : eyes)
  {
      rectangle(frame, faces[0].tl() + eye.tl(), faces[0].tl() + eye.br(), cv::Scalar(0, 255, 0), 2);
  }
  cv::Rect eyeRect = getLeftmostEye(eyes);
  cv::Mat eye = face(eyeRect); // crop the leftmost eye
  cv::equalizeHist(eye, eye);
  std::vector<cv::Vec3f> circles;
  cv::HoughCircles(eye, circles, 3, 1, eye.cols / 8, 250, 15, eye.rows / 8, eye.rows / 3);
  if (circles.size() > 0)
  {
      cv::Vec3f eyeball = getEyeball(eye, circles);
      cv::Point center(eyeball[0], eyeball[1]);
      centers.push_back(center);
      center = stabilize(centers, 5);
      if (centers.size() > 1)
      {
          cv::Point diff;
          diff.x = (center.x - lastPoint.x) * 20;
          diff.y = (center.y - lastPoint.y) * -30;
          mousePoint += diff;
      }
      lastPoint = center;
      int radius = (int)eyeball[2];
      cv::circle(frame, faces[0].tl() + eyeRect.tl() + center, radius, cv::Scalar(0, 0, 255), 2);
      cv::circle(eye, center, radius, cv::Scalar(255, 255, 255), 2);
  }
  //cv::imshow("Eye", eye);
}

int* getEyes() {

    static int x[2];
    x[0] = 0;
    x[1] = 0;

    return x;
}

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

cv::Mat frame;

cv::CascadeClassifier faceCascade;
cv::CascadeClassifier eyeCascade;

sf::CircleShape viewport(50.0f);

int eyeMin[2];
int eyeMax[2];

void cameraLoop() {
    cv::VideoCapture cap(0);
    while (1) {
        cap >> frame; // outputs the webcam image to a Mat
        detectEyes(frame, faceCascade, eyeCascade);
        cv::imshow("Webcam", frame); // displays the Mat
        int x[2] = { mousePoint.x, mousePoint.y };
        if (x[0] < eyeMin[0]) { x[0]= eyeMin[0]; }
        if (x[0] > eyeMax[0]) { x[0] = eyeMax[0]; }
        if (x[1] < eyeMin[1]) { x[1] = eyeMin[1]; }
        if (x[1] > eyeMax[1]) { x[1] = eyeMax[1]; }
        std::cout << "Pos: (" << x[0] << ", " << x[1] << ")\n";
        viewport.setPosition(x[0], x[1]);
        if (cv::waitKey(30) >= 0) break;
    }
    cv::destroyAllWindows();
}

int main() {
    /*
    cv::VideoCapture cap(0);
    while (1) {
        cap >> frame; // outputs the webcam image to a Mat
        cv::imshow("Webcam", frame); // displays the Mat
        if (cv::waitKey(30) >= 0) break;
    }
    */

    if (!faceCascade.load("./xmls/haarcascade_frontalface_alt.xml"))
    {
      std::cerr << "Could not load face detector." << std::endl;
      return -1;
    }    
    if (!eyeCascade.load("./xmls/haarcascade_eye_tree_eyeglasses.xml"))
    {
      std::cerr << "Could not load eye detector." << std::endl;
      return -1;
    }
    /*
    if (!cap.isOpened())
    {
      std::cerr << "Webcam not detected." << std::endl;
      return -1;
    }

    while (1) {
        cap >> frame; // outputs the webcam image to a Mat
        cv::imshow("Webcam", frame); // displays the Mat
        if (cv::waitKey(30) >= 0) break; // takes 30 frames per second. if the user presses any button, it stops from showing the webcam
    }
    */
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML works!", sf::Style::Default);
    sf::Vector2u windowSize = window.getSize();

    eyeMin[0] = 50;
    eyeMin[1] = 50;

    eyeMax[0] = windowSize.x - 50;
    eyeMax[1] = windowSize.y - 50;


    // Setup viewport
    //sf::CircleShape viewport(50.0f);
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

    mousePoint = cv::Point(800, 800);

    std::thread camth(cameraLoop);

    while (window.isOpen()) {
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            /*
            if (event.type == sf::Event::MouseMoved) {
                viewport.setPosition(event.mouseMove.x, event.mouseMove.y);
            }
            */

            if (state == -1) {
                //cap >> frame;
                //detectEyes(frame, faceCascade, eyeCascade);
                //int* eyes = getEyes();
                //viewport.setPosition(eyes[0], eyes[1]);
                
                //cv::imshow("Webcam", frame);
            }

            if (event.type == sf::Event::KeyPressed) {
                //std::cout << "Trying...";
            	if (event.key.code == sf::Keyboard::Space) {
                    std::cout << "Space!\n";
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

    camth.join();

    return 0;
}