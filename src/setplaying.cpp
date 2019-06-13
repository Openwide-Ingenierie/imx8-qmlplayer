#include "setplaying.h"

SetPlaying::SetPlaying(GstElement * pipeline)
{
    this->pipeline_ = pipeline ? static_cast<GstElement *> (gst_object_ref (pipeline)) : nullptr;

}

SetPlaying::~SetPlaying ()
{
  if (this->pipeline_) {
    gst_object_unref (this->pipeline_);
    if (msg != nullptr)
      gst_message_unref (msg);
    gst_object_unref (bus);
  }
}

void
SetPlaying::run ()
{
  if (this->pipeline_) {
    gst_element_set_state (this->pipeline_, GST_STATE_PAUSED);
    bus = gst_element_get_bus (this->pipeline_);
    msg = gst_bus_timed_pop (bus, GST_CLOCK_TIME_NONE);
  }
}
