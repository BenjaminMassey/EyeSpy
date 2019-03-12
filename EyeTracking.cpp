// https://picoledelimao.github.io/blog/2017/01/28/eyeball-tracking-for-mouse-control-in-opencv/

#include <iostream>
#include <stdio.h>

#include <opencv2/opencv.hpp>

#include <opencv/cv.h>
#include <opencv/cv.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/videoio/videoio.hpp>

using namespace cv;

using namespace std;

int main()
{
  cout << "OpenCV version : " << CV_VERSION << endl;
  
  VideoCapture cap(0); // the fist webcam connected to your PC
  
  if (!cap.isOpened())
  {
      cerr << "Webcam not detected." << endl;
      return -1;
  }
  Mat frame;
  while (1)
  {
      cap >> frame; // outputs the webcam image to a Mat
      imshow("Webcam", frame); // displays the Mat
      if (waitKey(30) >= 0) break; // takes 30 frames per second. if the user presses any button, it stops from showing the webcam
  }
  
  return 0;
  
}