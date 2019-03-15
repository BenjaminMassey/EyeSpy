# Eye Spy

Copyright 2019 Ben Massey and Michael Welch

## Idea
This is a video game based on the classic game of “Where’s Waldo?”. This game had the idea of a focus on the gimmick of eye tracking: the majority of the input will be gotten from where the player is looking on the screen. The screen will be mainly hidden except for where the player is looking - further adding to challenge of finding the wanted object.

## Implementation
This project is written entirely in C++. The eye-tracking is done using OpenCV, and largely taken from here: https://picoledelimao.github.io/blog/2017/01/28/eyeball-tracking-for-mouse-control-in-opencv/. The graphics and window handling are done using SFML.

## Installation
You will need to have SFML installed, along with OpenCV and the necessary sub libraries. This may have to be a complex process on your own if you are not in a standard Linux environment. Good luck.

Here are the necessary installs that you can sudo apt-get install in a standard Linux environment:

build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev
libavformat-dev libswscale-dev libopencv-dev libopencv-core-dev 
libopencv-highgui-dev libopencv-objdetect-dev libopencv-video-dev
libopencv-imgproc-dev libopencv-objdetect-dev ffmpeg

## Results
The eye-tracking portion ended up not working very well. It was a fun part of the project to go down, but it is far from usable for game purposes. Feel free to use it if you're curious, it runs fully but tracks extremely poorly.

With eye-tracking sucking, the game was written with mouse control so that the actual game vision was playable. Kind of sucks, but we believe that the game design is interesting enough to stand alone without the eye-tracking gimmick.

## Requirments
The eye-tracking version of this game requires a webcam. Any reasonable webcam should work as OpenCV is pretty good at it, but no guarantees. We have only tested with a Logitech C920.

There is also a mouse version included, which obviously does not require any special hardware.

## Running
Make the eye tracking version with "make EyeSpy" and then run EyeSpy.exe

Make the mouse version with "make EyeSpyMouse" and then run EyeSpyMouse.exe

## Contact
Feel free to email bmassey@uoregon.edu if you need something