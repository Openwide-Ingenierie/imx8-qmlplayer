#ifndef QMLPLAYER_H
#define QMLPLAYER_H

#include <QObject>
#include <QQuickItem>
#include <QQmlApplicationEngine>
#include <gst/gst.h>
#include <QUrl>
#include "setplaying.h"
#include <QQuickWindow>

class Qmlplayer : public QObject
{
    Q_OBJECT
public:
    explicit Qmlplayer();
    ~Qmlplayer();
    typedef struct _CustomData {
        GstElement *pipeline;
        GstElement *decode;
        GstElement *videoconvert;
        GstElement *glupload;
        GstElement *qmlglsink;
        GstElement *audioconvert;
        GstElement *audioresample;
        GstElement *volume;
        GstElement *alsasink;
    } CustomData;

    CustomData data;
    SetPlaying* play;
    gint64 duration;
    QQuickWindow *object;


    int qmlplayer_init(const QUrl url);
    int qmlplayer_deinit();

    static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data);

    void set_qmlobject(QQuickWindow *rootObject);

public slots:
    int pauseOrPlay();
    gint64 getDuration();
    gint64 getPosition();
    void changePosition();

private:

};





#endif // QMLPLAYER_H
