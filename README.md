# JS Bookmark Calls yt-dlp
This allows the user to, as the name kinda suggests, click on a javascript bookmarklet while on a YouTube video page to download the video in the background using yt-dlp and open it afterwards in VLC.
VLC will be paused and minimized so you can click on the tray icon to watch the video whenever you want.
For your convenience, the downloaded video file is automagically deleted once you close VLC.

## Prerequisites
- [Firefox](https://www.mozilla.org/en-US/firefox/download/thanks)
- [yt-dlp](https://github.com/yt-dlp/yt-dlp) and [VLC](https://www.videolan.org/vlc) both installed and available in the PATH environment variable

Sadly, this only works on Windows. Look at [How it works](#how-it-works) for an explanation.

## Setup
1. Clone this repository or download the individual files and put them wherever you want them to reside.
2. Compile [protocol handler.c](protocol%20handler.c).
3. Edit the last line of [add protocol to registry.reg](add%20protocol%20to%20registry.reg) to reflect the location of the resultant ```protocol handler.exe```. <br>
   Pay attention to escaping quotes and backslashes properly!
4. Merge [add protocol to registry.reg](add%20protocol%20to%20registry.reg) with the registry.
5. In Firefox, import the bookmark from the provided [HTML file](bookmark%20to%20import%20into%20firefox.html) and put it wherever you want. <br>
   As long as you don't modify the JS code inside of the bookmarklet, even the icon will stay!

## How it works
When you click on the bookmarklet, a small snippet of javascript code is executed. This will grab the video ID from the current page (and check whether you even are on a valid video page)
and attempt to open a special url: ```yt-dlp://``` followed by the video ID. <br>
This is a special protocol defined only on your computer that will call ```protocol handler.exe```. All the protocol magic is built into Windows and can be set up using the registry. <br>
Once ```protocol handler.exe``` is run, a Command Prompt window will pop up. You can minimize it or use it to moniter the progress of the download. <br>
This will call yt-dlp with a set of predefined parameters:
  - The video's resolution will be 1080p or the best available resolution below that.
  - The final file will be in a .mp4 container.
  - All temporary files will be stored at ```%USERPROFILE%\Videos\yt-dlp_tmp\```. <br>
Afterwards, VLC will be opened, but remain paused and minimized to the tray icon.

That's pretty much it!

## Thanks!
Thank you to [Endermanch](https://www.youtube.com/@Endermanch) for his [video on Windows protocols](https://www.youtube.com/watch?v=46pBeyHKQuQ), which taught me how they work and how to create one yourself. <br>
Thank you to [Tsoding](https://github.com/tsoding/) for his [nob.h](https://github.com/tsoding/musializer/blob/master/nob.h), from which I ~~stole~~ learned how to run new processes on Windows.
