Uses Lucas-Kanade [Optical Flow](https://www.youtube.com/watch?v=5AUypv5BNbI) method to track a pixel on the video and [stabilize](https://www.youtube.com/watch?v=iPlpAuY7nO4) it around the center of the frame.
Also auto-scales the frames to remove black borders.

![demo](https://github.com/mo3rfan/lockOn/blob/main/shuffle.gif)

# Usage

- Build and run the executable over the video, also specifying the output video filename.
```
./make.sh && ./a.out MyVideo.mp4 OutputVideo.mp4
```

- Click on a pixel to lock-on to in the first frame of the video on the window that pops up. There will be no visual feedback whatsoever (lol). Hit any key to continue the processing. No loading bars or nothing again. Just observe your CPU load and wait for the program to end.

- Use ffmpeg or something to merge audio track (if any) with the output video (since opencv does not deal with audio)

# Notes

- Included ffmpeg.sh script is added for reference. Allows you to combine two videos side by side and takes the audio of first input video and maps it to the output video's audio track.

Tested on M1 mac with OpenCV 4.5.2.