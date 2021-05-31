#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
    const string about =
        "This sample demonstrates Lucas-Kanade Optical Flow calculation.\n"
        "The example file can be downloaded from:\n"
        "  https://www.bogotobogo.com/python/OpenCV_Python/images/mean_shift_tracking/slow_traffic_small.mp4";
    const string keys =
        "{ h help |      | print this help message }"
        "{ @image | vtest.avi | path to image file }"
        "{ @output | output.mp4 | output filename }";
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    string filename = samples::findFile(parser.get<string>("@image"));
    string output_filename = parser.get<string>("@output");
    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }

    VideoCapture capture(filename);
    if (!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open file!" << endl;
        return 0;
    }

    // Create some random colors
    vector<Scalar> colors;
    RNG rng;
    for(int i = 0; i < 100; i++)
    {
        int r = rng.uniform(0, 256);
        int g = rng.uniform(0, 256);
        int b = rng.uniform(0, 256);
        colors.push_back(Scalar(r,g,b));
    }

    Mat old_frame, old_gray;
    vector<Point2f> p0, p1;

    // Take first frame and find corners in it
    capture >> old_frame;
    cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
    //goodFeaturesToTrack(old_gray, p0, 1, 0.5, 70, Mat(), 7, false, 0.04);

    //cout << p0.front().x << ", " << p0.front().y << "\n";
    p0.insert(p0.begin(), Point2f(old_frame.size().width * 0.3, old_frame.size().height * 0.35));

    // Create a mask image for drawing purposes
    Mat mask = Mat::zeros(old_frame.size(), old_frame.type());
    VideoWriter vw;
    Size frameSize;
    frameSize.width = capture.get(CAP_PROP_FRAME_WIDTH);
    frameSize.height = capture.get(CAP_PROP_FRAME_HEIGHT);
    cout << "FPS is "; cout << capture.get(CAP_PROP_FPS) << "\n";
    int i = 0;
    Point2f distance_from_center = Point2f(1, 1);
    float scaleFactor = 1;
    vw.open(output_filename, VideoWriter::fourcc('M', 'J', 'P', 'G'), capture.get(CAP_PROP_FPS), frameSize);
    while(true){
        i++;
        Mat frame, frame_gray;

        capture >> frame;
        if (frame.empty())
            break;
        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

        // calculate optical flow
        vector<uchar> status;
        vector<float> err;
        TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), 10, 0.03);
        calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err, Size(15,55), 1, criteria);
        vector<Point2f> good_new;
        for(uint i = 0; i < p0.size(); i++)
        {
            // Select good points
            //if(status[i] == 1) {
                good_new.push_back(p1[i]);
                // draw the tracks
                //line(mask,p1[i], p0[i], colors[i], 2);
                //circle(frame, p1[i], 5, colors[i], -1);
                break;
                // p1[i] is the center point of each frame. they are computed from p0.
            //}
        }
        Mat img;
        add(frame, mask, img);
        Mat dst;

        //scaleFactor += distance_from_center.x / 10000;
        //cout << scaleFactor << "\n";
        distance_from_center.x = p1[0].x - (img.size().width / 2) / scaleFactor;
        distance_from_center.y = p1[0].y - (img.size().height / 2) / scaleFactor;
        // 2x3 matrix with last row being our translation
        //Mat transform_mtrx(2, 3, CV_64F);

        /*
        1  0  10
        0  1  10

        in the transformation matrix above, 10, 10 will be our translation

        but for some reason I had to write the array as follows lol
        */
        Point2f trsfm_vals[3]; // array vals denote 3 columns

        //while(true) {
            //scaleFactor = scaleFactor + 0.1; // incr scale factor as long as we don't detect a border
            distance_from_center *= scaleFactor;
            trsfm_vals[0] = Point2f(scaleFactor, 0);
            trsfm_vals[1] = Point2f(-distance_from_center.x, 0);
            trsfm_vals[2] = Point2f(scaleFactor, -distance_from_center.y);
            Mat transform_mtrx(2, 3, CV_32F, trsfm_vals);
            warpAffine(img, dst, transform_mtrx, dst.size(), 1, BORDER_CONSTANT, 0);
            // break if dst does not have 0xF.
            //cout << dst;
            //checkRange(dst, false, 0, 0, 256);
        //}
        //imshow("Frame", dst);
        vw << dst;
        /*
        int keyboard = waitKey(capture.get(CAP_PROP_FPS));
        if (keyboard == 'q' || keyboard == 27)
            break;
        */
        //if (i == 30) {
        //    break;
        //}
        // Now update the previous frame and previous points
        old_gray = frame_gray.clone();
        //cout << "GOOD NEW: " << good_new << "\n";
        p0 = good_new;
    }
    cout << "Released\n";
    vw.release();
}
