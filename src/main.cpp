/*****************************************************************************
MIT License

Copyright (c) 2019 Muyao Chen <muyao.chen@smile.fr>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickItem>
#include <QRunnable>
#include <gst/gst.h>
#include <gst/gstbus.h>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtCore/QDir>
#include "qmlplayer.h"
#include <QObject>

int main(int argc, char *argv[])
{
    int ret;
    gst_init (&argc, &argv);
    qmlRegisterType<Qmlplayer>("qmlplayerLib", 1, 0, "Qmlplayer");
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Video Widget Example");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("url", "The URL to open.");
    parser.process(app);

    Qmlplayer *qplayer = new Qmlplayer ();

    if (!parser.positionalArguments().isEmpty()) {
        const QUrl url =
            QUrl::fromUserInput(parser.positionalArguments().constFirst(),
                                QDir::currentPath(), QUrl::AssumeLocalFile);
        qplayer->qmlplayer_init(url);
    } else {
        g_print("No Input File. Please enter a valid file path as argument.\n");
        return -1;
    }


    QQmlApplicationEngine engine(QUrl("qrc:///main.qml"));

    QQuickItem *videoItem;
    QQuickWindow *rootObject;

    /* find and set the videoItem on the sink */
    rootObject = static_cast<QQuickWindow *> (engine.rootObjects().first());
    videoItem = rootObject->findChild<QQuickItem *> ("videoItem");
    g_assert (videoItem);
    g_object_set(qplayer->data.qmlglsink, "widget", videoItem, NULL);
    rootObject->scheduleRenderJob (qplayer->play,
        QQuickWindow::BeforeSynchronizingStage);

    qplayer->set_qmlobject(rootObject);

    ret = app.exec();

    qplayer->qmlplayer_deinit();
    return ret;
}
