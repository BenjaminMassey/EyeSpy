EyeTracking: EyeTracking.cpp
	g++ EyeTracking.cpp -o EyeTracking.exe -I /usr/local/include -L /usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_objdetect -lopencv_highgui -lopencv_videoio

EyeSpy.exe: EyeSpy.o
	g++ EyeSpy.o -o EyeSpy.exe -lsfml-graphics -lsfml-window -lsfml-system

EyeSpy.o:
	g++ -c EyeSpy.cpp

clean:
	rm -f EyeTracking.exe EyeSpy.o EyeSpy.exe