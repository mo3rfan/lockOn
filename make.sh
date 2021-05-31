g++ opticalflow.cpp -I/usr/local/include/opencv4 -std=c++20 -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_video -Wno-deprecated-anon-enum-enum-conversion &&\
#./a.out ~/Movies/test.mov
#open out1.mp4
echo 'Done'