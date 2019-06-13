# Qmlplayer

## What is it?

Qmlplayer is a simple program developed on **Qt** using the **qtglsink**. This program is inspired by the example of [qmlsink](https://gitlab.freedesktop.org/gstreamer/gst-plugins-good/tree/master/tests/examples/qt/qmlsink) in gst-plugins-good.

To test, you can import the project into QtCreator and let it do the rest. Or you can build the project yourself with **qmake** and **make**. If your environment is correctly set, you can simply do :

```sh
qmake
make
```



The project is developed on **Qt Creator 4.8.2**, which is based on **QT 5.12.1**. As listed in play.pro, we need the dependencies of **Qt5Qml**, **Qt5Quick** and **Qt5Widgets**. 

If you want to build in **Yocto project**, You have to include the source code in to your recipe.



## Why not qtmultimedia?

Qtmultimedia is good but it doesn't work well on my board (i.MX8). In fact, the default sink of qtmultimedia does not support completely openGL, so that my video is processed by CPU, which is horrible.

Therefore, we have written this simple program to  demonstrate how can we use GPU resource in a Qt application to render the video playback.



## How to use?

```sh
/path/to/qmlplayer /path/to/your/video
```

If you test directly in Qt Creator, you can add your path to the video file in the *Command line arguments* of "*Run*".

In case you use another platform : 

```sh
/path/to/qmlplayer /path/to/your/video -platform eglfs
/path/to/qmlplayer /path/to/your/video -platform wayland
...
```

I have created a button start/pause and a slider. You should be able to pause or resume the video and change the position. **Remember to click the button when you open the application, it's on pause mode by default.**

If you try it on a evaluation board, maybe you won't see any text, that is because you haven't installed any fonts. Just follow the output on the command line : 

```sh
QFontDatabase: Cannot find font directory /usr/lib/fonts.
Note that Qt no longer ships fonts. Deploy some (from https://dejavu-fonts.github.io/ for example) or switch to fontconfig.
```

Load some fonts into `/usr/lib/fonts`.


