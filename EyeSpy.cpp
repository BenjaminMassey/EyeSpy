// EyeSpy - https://www.github.com/BenjaminMassey/EyeSpy
// Copyright 2019 Ben Massey and Michael Welch

// A large portion of the eye tracking work was taken directly from this tutorial:
// https://picoledelimao.github.io/blog/2017/01/28/eyeball-tracking-for-mouse-control-in-opencv/
// Huge shoutouts to him, really appreciate the easy intro to eye tracking in openCV.

#include <SFML/Graphics.hpp>
#include <cmath>

#include <iostream>
#include <thread>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

// Globals

std::vector<cv::Point> centers;
cv::Point lastPoint;
cv::Point mousePoint; // Current estimated look point (original)

cv::Mat frame; // Holds the face

cv::CascadeClassifier faceCascade;
cv::CascadeClassifier eyeCascade;

sf::CircleShape viewport(50.0f);

int state;

int eyePos[2]; // Where to put the eye

int eyeMin[2]; // Minimum screen pos to look
int eyeMax[2]; // Maximum screen pos to look

int calibs[9][2][2]; // Matrix gathered from calibration

float shiftX[2]; // Equation to correct eye guess x coord [slope, y-intercept]
float shiftY[2]; // Equation to correct eye guess y coord [slope, y-intercept]

cv::Vec3f getEyeball(cv::Mat &eye, std::vector<cv::Vec3f> &circles) {
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

cv::Rect getLeftmostEye(std::vector<cv::Rect> &eyes) {
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

cv::Point stabilize(std::vector<cv::Point> &points, int windowSize) {
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

void detectEyes(cv::Mat &frame, cv::CascadeClassifier &faceCascade, cv::CascadeClassifier &eyeCascade) {
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
  int x1, x2, y1, y2, r;
  int index = 0;
  for (cv::Rect &eye : eyes)
  {
    if (index == 0) { 
        x1 = (faces[0].tl() + eye.tl()).x;
        x2 = (faces[0].tl() + eye.br()).x;
        y1 = (faces[0].tl() + eye.tl()).y;
        y2 = (faces[0].tl() + eye.br()).y;
    }
      rectangle(frame, faces[0].tl() + eye.tl(), faces[0].tl() + eye.br(), cv::Scalar(0, 255, 0), 2);
      index++;
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
      lastPoint = center;
      int radius = (int)eyeball[2];
      cv::Point c = faces[0].tl() + eyeRect.tl() + center;
      float cx = c.x;
      float cy = c.y;
      float x3 = c.x - radius;
      if (x3 < 0) { x3 = -x3; }
      float y3 = c.y - radius;
      if (y3 < 0) { y3 = -y3; }
      float a1 = ((x2 - x1) - (x2 - x3));
      float b1 = ((y2 - y1) - (y2 - y3));
      mousePoint.x = (a1 / (x2 - x1)) * 100.0f;
      mousePoint.y = (b1 / (y2 - y1)) * 100.0f;
      cv::circle(frame, faces[0].tl() + eyeRect.tl() + center, radius, cv::Scalar(0, 0, 255), 2);
  }
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

void createXEquation() {
    int points[9][2];
    for (int i = 0; i < 9; i++) {
        points[i][0] = calibs[i][0][0];
        points[i][1] = calibs[i][1][0];
    }
    /* DEBUG
    for(int i = 0; i < 9; i++){
        std::cout << "(" << points[i][0] << ", " << points[i][1] << ")\n";
    }
    */

    //https://www.varsitytutors.com/hotmath/hotmath_help/topics/line-of-best-fit
    int sum = 0;
    for (int i = 0; i < 9; i++) {
        sum += points[i][0];
    }
    float avgX = std::round(sum / 9);
    sum = 0;
    for (int i = 0; i < 9; i++) {
        sum += points[i][1];
    }
    float avgY = std::round(sum / 9);
    float sumTop = 0;
    float sumBottom = 0;
    for (int i = 0; i < 9; i++) {
        sumTop += (points[i][0] - avgX) * (points[i][1] - avgY);
        sumBottom += (points[i][0] - avgX) * (points[i][0] - avgX);
    }
    shiftX[0] = sumTop / sumBottom;
    shiftX[1] = avgY - (shiftX[0] * avgX);
    std::cout << "y = " << shiftX[0] << "x + " << shiftX[1] << "\n\n";
}

void createYEquation() {
    int points[9][2];
    for (int i = 0; i < 9; i++) {
        points[i][0] = calibs[i][0][1];
        points[i][1] = calibs[i][1][1];
    }

    /* DEBUG
    for(int i = 0; i < 9; i++){
        std::cout << "(" << points[i][0] << ", " << points[i][1] << ")\n";
    }
    */

    //https://www.varsitytutors.com/hotmath/hotmath_help/topics/line-of-best-fit
    int sum = 0;
    for (int i = 0; i < 9; i++) {
        sum += points[i][0];
    }
    float avgX = std::round(sum / 9);
    sum = 0;
    for (int i = 0; i < 9; i++) {
        sum += points[i][1];
    }
    float avgY = std::round(sum / 9);
    float sumTop = 0;
    float sumBottom = 0;
    for (int i = 0; i < 9; i++) {
        sumTop += (points[i][0] - avgX) * (points[i][1] - avgY);
        sumBottom += (points[i][0] - avgX) * (points[i][0] - avgX);
    }
    shiftY[0] = sumTop / sumBottom;
    shiftY[1] = avgY - (shiftX[0] * avgX);
    std::cout << "y = " << shiftY[0] << "x + " << shiftY[1] << "\n\n";
}

int* calibrationConvert(int original[]) {
    static int converted[2];
    if (shiftX[0] != 0.0f) {
        converted[0] = std::round((shiftX[0] * original[0]) + shiftX[1]);
    } else {
        converted[0] = original[0];
    }
    if (shiftY[0] != 0.0f) {
        converted[1] = std::round((shiftY[0] * original[1]) + shiftY[1]);
    } else {
        converted[1] = original[1];
    }
    return converted;
}

void cameraLoop() {
    cv::VideoCapture cap(0);
    while (1) {
        cap >> frame; // outputs the webcam image to a Mat
        detectEyes(frame, faceCascade, eyeCascade);
        cv::imshow("Webcam", frame); // displays the Mat
        eyePos[0] = mousePoint.x;
        eyePos[1] = mousePoint.y;
        std::cout << "Original Position: (" << eyePos[0] << ", " << eyePos[1] << ")\n";
        if (state == -1) {
            int* conversion = calibrationConvert(eyePos);
            eyePos[0] = conversion[0];
            eyePos[1] = conversion[1];
            std::cout << "Converted Position: (" << eyePos[0] << ", " << eyePos[1] << ")\n";
            if (eyePos[0] < eyeMin[0]) { eyePos[0]= eyeMin[0]; }
            if (eyePos[0] > eyeMax[0]) { eyePos[0] = eyeMax[0]; }
            if (eyePos[1] < eyeMin[1]) { eyePos[1] = eyeMin[1]; }
            if (eyePos[1] > eyeMax[1]) { eyePos[1] = eyeMax[1]; }
            std::cout << "Capped Position: (" << eyePos[0] << ", " << eyePos[1] << ")\n";
        }
        viewport.setPosition(eyePos[0], eyePos[1]);
        if (cv::waitKey(30) >= 0) break;
    }
    cv::destroyAllWindows();
}

void niceCalibsPrint() {
    std::cout << "\n\nCalibration Matrix:\n";
    for (int i = 0; i < 9; i++) {
        std::cout << "(" << calibs[i][0][0] << ", " << calibs[i][0][1] << ")";
        std::cout << " -> ";
        std::cout << "(" << calibs[i][1][0] << ", " << calibs[i][1][1] << ")";
        std::cout << "\n";
    }
    std::cout << "\n\n";
}

int main() {

    // Default equation values
    shiftX[0] = 0.0f;
    shiftX[1] = 0.0f;
    shiftY[0] = 0.0f;
    shiftY[1] = 0.0f;

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
    
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "EyeSpy (Eye-Tracking)", sf::Style::Default);
    sf::Vector2u windowSize = window.getSize();

    eyeMin[0] = 50;
    eyeMin[1] = 50;

    eyeMax[0] = windowSize.x - 50;
    eyeMax[1] = windowSize.y - 50;


    // Setup viewport
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
    state = 0;
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

    float sum;

    while (window.isOpen()) {
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                //DEBUG std::cout << "Trying...";
            	if (event.key.code == sf::Keyboard::Space) {
                    std::cout << "Space!\n";
            		switch(state) {
            			case 0:
            				calibration.setPosition(windowSize.x / 2, 25.0f + 1.0f);
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[0];
                            }
                            calibs[state][0][0] = sum / 100000.0f;
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[1];
                            }
                            calibs[state][0][1] = sum / 100000.0f;
                            calibs[state][1][0] = 26;
                            calibs[state][1][1] = 26;
            				state = 1;
            				break;
            			case 1:
            				calibration.setPosition(windowSize.x - 25.0f - 1.0f, 25.0f + 1.0f);
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[0];
                            }
                            calibs[state][0][0] = sum / 100000.0f;
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[1];
                            }
                            calibs[state][0][1] = sum / 100000.0f;
                            calibs[state][1][0] = std::round(windowSize.x / 2);
                            calibs[state][1][1] = 26;
            				state = 2;
            				break;
            			case 2:
            				calibration.setPosition(25.0f + 1.0f, windowSize.y / 2);
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[0];
                            }
                            calibs[state][0][0] = sum / 100000.0f;
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[1];
                            }
                            calibs[state][0][1] = sum / 100000.0f;
                            calibs[state][1][0] = std::round(windowSize.x - 25.0f - 1.0f);
                            calibs[state][1][1] = 26;
            				state = 3;
            				break;
            			case 3:
            				calibration.setPosition(windowSize.x / 2, windowSize.y / 2);
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[0];
                            }
                            calibs[state][0][0] = sum / 100000.0f;
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[1];
                            }
                            calibs[state][0][1] = sum / 100000.0f;
                            calibs[state][1][0] = 26;
                            calibs[state][1][1] = std::round(windowSize.y / 2);
            				state = 4;
            				break;
            			case 4:
            				calibration.setPosition(windowSize.x - 25.0f - 1.0f, windowSize.y / 2);
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[0];
                            }
                            calibs[state][0][0] = sum / 100000.0f;
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[1];
                            }
                            calibs[state][0][1] = sum / 100000.0f;
                            calibs[state][1][0] = std::round(windowSize.x / 2);
                            calibs[state][1][1] = std::round(windowSize.y / 2);
            				state = 5;
            				break;
            			case 5:
            				calibration.setPosition(25.0f + 1.0f, windowSize.y - 25.0f - 1.0f);
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[0];
                            }
                            calibs[state][0][0] = sum / 100000.0f;
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[1];
                            }
                            calibs[state][0][1] = sum / 100000.0f;
                            calibs[state][1][0] = std::round(windowSize.x - 25.0f - 1.0f);
                            calibs[state][1][1] = std::round(windowSize.y / 2);
            				state = 6;
            				break;
            			case 6:
            				calibration.setPosition(windowSize.x / 2, windowSize.y - 25.0f - 1.0f);
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[0];
                            }
                            calibs[state][0][0] = sum / 100000.0f;
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[1];
                            }
                            calibs[state][0][1] = sum / 100000.0f;
                            calibs[state][1][0] = 26;
                            calibs[state][1][1] = std::round(windowSize.y - 25.0f - 1.0f);
            				state = 7;
            				break;
            			case 7:
            				calibration.setPosition(windowSize.x - 25.0f - 1.0f, windowSize.y - 25.0f - 1.0f);
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[0];
                            }
                            calibs[state][0][0] = sum / 100000.0f;
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[1];
                            }
                            calibs[state][0][1] = sum / 100000.0f;
                            calibs[state][1][0] = std::round(windowSize.x / 2);
                            calibs[state][1][1] = std::round(windowSize.y - 25.0f - 1.0f);
            				state = 8;
            				break;
            			default:
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[0];
                            }
                            calibs[state][0][0] = sum / 100000.0f;
                            sum = 0.0f;
                            for (int i = 0; i < 100000; i++) {
                                sum += eyePos[1];
                            }
                            calibs[state][0][1] = sum / 100000.0f;
                            calibs[state][1][0] = std::round(windowSize.x - 25.0f - 1.0f);
                            calibs[state][1][1] = std::round(windowSize.y - 25.0f - 1.0f);
                            state = -1;
                            niceCalibsPrint();
                            createXEquation();
                            createYEquation();
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