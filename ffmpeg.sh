ffmpeg -i dnce.mp4 -i output.mp4 -filter_complex '[0:v]pad=iw*2:ih[int];[int][1:v]overlay=W/2:0[vid]' -map '[vid]' -t 40 -map 0:a:0 -strict -2 output.webm
# -t