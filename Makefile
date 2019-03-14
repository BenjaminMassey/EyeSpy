EyeTracking: EyeTracking.cpp
	g++ EyeTracking.cpp -o EyeTracking.exe -I /usr/local/include -L /usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_objdetect -lopencv_highgui -lopencv_videoio

EyeSpy: EyeSpy.o
	g++ -c EyeSpy.cpp
	g++ EyeSpy.o -o EyeSpy.exe -I /usr/local/include -L /usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_objdetect -lopencv_highgui -lopencv_videoio -lsfml-graphics -lsfml-window -lsfml-system

clean:
	rm -f EyeTracking.exe EyeSpy.o EyeSpy.exe