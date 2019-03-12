EyeTracking: EyeTracking.cpp
	g++ EyeTracking.cpp -o EyeTracking.exe -I /usr/local/include -L /usr/local/lib -lopencv_core -lopencv_highgui -lopencv_videoio

clean:
	rm -f EyeTracking.exe