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
#include "qmlplayer.h"

Qmlplayer::Qmlplayer()
{}

Qmlplayer::~Qmlplayer()
{}

int
Qmlplayer::qmlplayer_init(const QUrl url)
{

    /****************************Pipeline*****************************************
                    => audioconvert => audioresample => volume => alsasink
    uridecodebin <
                    => videoconvert => glupload => qmlglsink => videoItem(Qt)
    *****************************************************************************/

    data.pipeline = gst_pipeline_new(nullptr);
    data.decode = gst_element_factory_make("uridecodebin", nullptr);
    g_object_set(data.decode, "uri", url.toEncoded().data(), NULL );

    /***Video Init***/
    data.videoconvert = gst_element_factory_make("videoconvert", nullptr);
    data.glupload = gst_element_factory_make("glupload", nullptr);
    data.qmlglsink = gst_element_factory_make("qmlglsink", nullptr);

    /***Audio Init***/
    data.audioconvert = gst_element_factory_make("audioconvert", nullptr);
    data.audioresample = gst_element_factory_make("audioresample", nullptr);
    data.volume = gst_element_factory_make("volume", nullptr);
    data.alsasink = gst_element_factory_make("alsasink", nullptr);

    g_assert(data.decode && data.videoconvert && data.glupload && data.qmlglsink \
                && data.audioconvert && data.audioresample && data.volume && data.alsasink);

    gst_bin_add_many(GST_BIN (data.pipeline), data.decode, data.videoconvert, data.glupload, \
                        data.qmlglsink, data.audioconvert, data.audioresample, data.volume, data.alsasink, NULL);

    /****Video static link****/
    if (!gst_element_link_many(data.videoconvert, data.glupload, data.qmlglsink, NULL)) {
        g_printerr("video elements could not be linked.\n");
        gst_object_unref(data.pipeline);
        return -1;
    }

    /**** Audio static link****/
    if (!gst_element_link_many(data.audioconvert, data.audioresample, data.volume, data.alsasink, NULL)) {
        g_printerr("audio elements could not be linked.\n");
        gst_object_unref(data.pipeline);
        return -1;
    }

    /**** Link dynamic pad ****/
    g_signal_connect(data.decode, "pad-added", G_CALLBACK(pad_added_handler), &data);

    /**** Wait for playing mode ****/
    play = new SetPlaying(this->data.pipeline);
    return 0;
}

void
Qmlplayer::set_qmlobject(QQuickWindow *rootObject)
{
    this->object = rootObject;
    QQuickItem* button = rootObject->findChild<QQuickItem *> ("button");
    QQuickItem* position = rootObject->findChild<QQuickItem *> ("position");
    QQuickItem* slider = rootObject->findChild<QQuickItem *> ("slider");

    connect(button, SIGNAL(pauseOrPlay()), this, SLOT(pauseOrPlay()));
    connect(position, SIGNAL(getPosition()), this, SLOT(getPosition()));
    connect(slider, SIGNAL(seekPosition()), this, SLOT(changePosition()));

}

void
Qmlplayer::pad_added_handler(GstElement *src, GstPad *new_pad, CustomData *data)
{
    GstPad *videosink_pad = gst_element_get_static_pad(data->videoconvert,"videosink");

    GstPad *audiosink_pad = gst_element_get_static_pad(data->audioconvert,"audiosink");

    GstPadLinkReturn ret;

    GstCaps *new_pad_caps = nullptr;
    GstStructure *new_pad_struct = nullptr;
    const gchar *new_pad_type = nullptr;

    g_print("Received new pad '%s' from '%s':\n",GST_PAD_NAME(new_pad),GST_ELEMENT_NAME(src));

    if (gst_pad_is_linked(videosink_pad) && gst_pad_is_linked(audiosink_pad)) {
        g_print("We are already linked. \n");
        goto exit;
    }

    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps,0);
    new_pad_type = gst_structure_get_name(new_pad_struct);

    if (g_str_has_prefix(new_pad_type,"video/x-raw")) {
        videosink_pad = gst_element_get_compatible_pad (data->videoconvert, new_pad, NULL);
        ret = gst_pad_link(new_pad, videosink_pad);
        if (GST_PAD_LINK_FAILED(ret)) {
            g_print("Type is '%s' but link failed.\n", new_pad_type);
        } else {
            g_print("Link succeeded(type '%s'). \n", new_pad_type);
        }
    } else if (g_str_has_prefix(new_pad_type,"audio/x-raw")) {
        videosink_pad = gst_element_get_compatible_pad (data->audioconvert, new_pad, nullptr);
        ret = gst_pad_link(new_pad, videosink_pad);
        if (GST_PAD_LINK_FAILED(ret)) {
            g_print("Type is '%s' but link failed.\n", new_pad_type);
        } else {
            g_print("Link succeeded(type '%s'). \n", new_pad_type);
        }
    } else {
        g_print("It has type '%s' which is not raw video nor raw audio. Ignoring. \n", new_pad_type);
    }

exit :
    if (new_pad_caps != nullptr) {
        gst_caps_unref(new_pad_caps);
    }
    if (videosink_pad != nullptr) {
        gst_object_unref(videosink_pad);
    }
    if (audiosink_pad != nullptr) {
        gst_object_unref(audiosink_pad);
    }
}

int
Qmlplayer::pauseOrPlay()
{
    GstElement* pipeline = this->data.pipeline ? static_cast<GstElement *> (gst_object_ref (this->data.pipeline)) : nullptr;
    GstState current;
    if(pipeline) {
        gst_element_get_state(pipeline, &current, nullptr, GST_CLOCK_TIME_NONE);
    } else {
        g_print("NULL pipeline!\n");
        return -1;
    }
    if(current == GST_STATE_PLAYING) {
        gst_element_set_state(pipeline,GST_STATE_PAUSED);
    } else if(current == GST_STATE_PAUSED) {
        gst_element_set_state(pipeline,GST_STATE_PLAYING);
    }

    // Print DOT File if the environment varialbe GST_DEBUG_DUMP_DOT_DIR is set
    gst_debug_bin_to_dot_file_with_ts(GST_BIN(pipeline),
                                      GstDebugGraphDetails(GST_DEBUG_GRAPH_SHOW_ALL /* GST_DEBUG_GRAPH_SHOW_MEDIA_TYPE | GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS | GST_DEBUG_GRAPH_SHOW_STATES*/),
                                      "playbin_set");

    QQuickItem* item = this->object->findChild<QQuickItem *> ("rootItem");
    gint64 dur;
    dur = this->getDuration();
    QVariant v((unsigned long long)(dur/1000000));
    item->setProperty("dur",v);
    return 0;
}

gint64
Qmlplayer::getDuration()
{
    GstElement* pipeline = this->data.pipeline ? static_cast<GstElement *> (gst_object_ref (this->data.pipeline)) : nullptr;
    gint64 dur;
    if(pipeline) {
        gst_element_query_duration(pipeline, GST_FORMAT_TIME, &dur); //time in nanosecond
        this->duration = dur;
        g_print("duration : %ld\n",dur);
        return dur;
    }
    else {
        g_print("NULL pipeline!\n");
        return 0;
    }
}

gint64
Qmlplayer::getPosition()
{
    GstElement* pipeline = this->data.pipeline ? static_cast<GstElement *> (gst_object_ref (this->data.pipeline)) : nullptr;
    gint64 pos;
    GstState current_state;
    if(pipeline) {
        gst_element_get_state(pipeline, &current_state, nullptr, GST_CLOCK_TIME_NONE);
        if (current_state != GST_STATE_PLAYING) {
            return 1; // The video is over (pos = 1.0)
        }
        gst_element_query_position(pipeline, GST_FORMAT_TIME, &pos); //time in nanosecond
//        g_print("%ld\n", pos); //Debug : Print position information
        QQuickItem* item = this->object->findChild<QQuickItem *> ("rootItem");
        QVariant v((unsigned long long)(pos/1000000));
        item->setProperty("pos", v);
        return pos;
    }
    else {
        g_print("NULL pipeline!\n");
        return 0;
    }
}

void
Qmlplayer::changePosition()
{
    GstElement* pipeline = this->data.pipeline ? static_cast<GstElement *> (gst_object_ref (this->data.pipeline)) : nullptr;
    gint64 pos;
    gint64 dur=this->duration;
    double var;
    QQuickItem* slider = this->object->findChild<QQuickItem *> ("slider");
    var = slider->property("value").toDouble();
    pos = (gint64)(var*dur);
    if(pipeline) {
        gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,GST_SEEK_TYPE_SET, pos, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
//        g_print("%ld\n",pos); // Debug : Print position information
        QQuickItem* item = this->object->findChild<QQuickItem *> ("rootItem");
        QVariant v((unsigned long long)(pos/1000000));
        item->setProperty("pos",v);
    }
    else {
        g_print("NULL pipeline!\n");
    }
}


int
Qmlplayer::qmlplayer_deinit()
{
    gst_element_set_state (this->data.pipeline, GST_STATE_NULL);
    gst_object_unref (this->data.pipeline);
    gst_deinit ();
    return 0;
}
