#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

using namespace cv;
using namespace std;

int x = 0, y = 0;

void mouse_cb(int event, int p, int q, int flags, void *userdata) {
    x = p;
    y = q;
}

bool ugly_borders(Mat &dst) {
    Mat thresh, dst_greyframe;
    vector<Mat> contours;
    cvtColor(dst, dst_greyframe, COLOR_BGR2GRAY);
    threshold(dst_greyframe, thresh, 1, 255, THRESH_BINARY);
    findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    //cout << contours[0] << "\n";
    bool retval = (contours[0].at<int>(2, 0) == dst.size().width - 1) && (contours[0].at<int>(2, 1) == dst.size().height - 1)
    && (contours[0].at<int>(0, 0) == 0) && (contours[0].at<int>(0, 1) == 0);
    return !retval;
}

int main(int argc, char **argv)
{
    const string about = "";
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
        if (i == 1) {
            imshow("win1", frame);
            setMouseCallback("win1", mouse_cb);
            waitKey(0);
            destroyWindow("win1");
            cout << x << ", " << y << "\n";
            p0.insert(p0.begin(), Point2f(x, y));
        }
        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

        // calculate optical flow
        vector<uchar> status;
        vector<float> err;
        TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), 10, 0.03);
        calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err);
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
        Point2f trsfm_vals[3]; // array vals denote 3 columns
        do {
            distance_from_center.x = p1[0].x - (img.size().width / 2) / scaleFactor;
            distance_from_center.y = p1[0].y - (img.size().height / 2) / scaleFactor;
            scaleFactor = scaleFactor + 0.01; // incr scale factor as long as we don't detect a border
            distance_from_center *= scaleFactor;
            trsfm_vals[0] = Point2f(scaleFactor, 0);
            trsfm_vals[1] = Point2f(-distance_from_center.x, 0);
            trsfm_vals[2] = Point2f(scaleFactor, -distance_from_center.y);
            Mat transform_mtrx(2, 3, CV_32F, trsfm_vals);
            warpAffine(img, dst, transform_mtrx, dst.size(), 1, BORDER_CONSTANT, 0);
        } while(ugly_borders(dst));
        //imshow("Frame", dst);
        scaleFactor = 1;
        vw << dst;
        old_gray = frame_gray.clone();
        p0 = good_new;
    }
    cout << "Released\n";
    vw.release();
}
