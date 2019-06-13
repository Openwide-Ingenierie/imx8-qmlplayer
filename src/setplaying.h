#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QObject>
#include <QRunnable>
#include <gst/gst.h>

class SetPlaying : public QObject, public QRunnable
{
    Q_OBJECT
public:    
    explicit SetPlaying(GstElement * pipeline);
    ~SetPlaying();
    void run ();


private:
  GstElement * pipeline_;
  GstBus *bus;
  GstMessage *msg;
};




#endif // PLAYERCONTROLLER_H
