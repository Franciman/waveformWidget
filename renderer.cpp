#include "renderer.h"

#include <mpv/client.h>

#include <QMediaPlayer>

#include <stdexcept>
#include <cstring>

#include <iostream>

class QVideoWidget;

static void wakeup(void *ctx)
{
    Renderer *rend = (Renderer*)ctx;
    emit rend->mpv_events();
}

static const int TimePosReplyCode = 124;

AbstractRenderer::AbstractRenderer()
{ }

AbstractRenderer::~AbstractRenderer()
{ }

Renderer::Renderer() :
    AbstractRenderer(),
    mpv(mpv_create()),
    TimePosMs(0)
{
    if(!mpv)
        throw std::runtime_error("Can't initialize mpv");

    mpv_set_option_string(mpv, "vo", "null");


    connect(this, SIGNAL(mpv_events()), this, SLOT(on_mpv_events()), Qt::QueuedConnection);
    mpv_set_wakeup_callback(mpv, wakeup, this);
    mpv_observe_property(mpv, TimePosReplyCode, "playback-time", MPV_FORMAT_DOUBLE);

    if(mpv_initialize(mpv) < 0)
        throw std::runtime_error("mpv failed to initialize");


    //const char *args[] = {"loadfile", filename, nullptr};
    //mpv_command(mpv, args);
    mpv_set_property_string(mpv, "pause", "yes");
}

void Renderer::loadMedia(const char *filename)
{
    const char *args[] = {"loadfile", filename, nullptr};
    mpv_command(mpv, args);
}

void Renderer::setVideoOutput(QWidget *widget)
{
    Q_UNUSED(widget)
}

int Renderer::getPositionMs() const
{
    /*double time;
    mpv_get_property(mpv, "playback-time", MPV_FORMAT_DOUBLE, &time);
    time *= 1000.0;
    return (int)time;*/
    return TimePosMs;
}

Renderer::~Renderer()
{
    if(mpv)
        mpv_terminate_destroy(mpv);
}

void Renderer::handle_mpv_event(mpv_event *event)
{
    switch(event->event_id)
    {
    case MPV_EVENT_PROPERTY_CHANGE:
        if(event->reply_userdata == TimePosReplyCode)
        {
            mpv_event_property *prop = (mpv_event_property*)event->data;
            if(prop->format == MPV_FORMAT_DOUBLE)
            {
                TimePosMs = *static_cast<double*>(prop->data) * 1000.0;
                // Avoid TimePosMs being negative
                TimePosMs = std::max(0, TimePosMs);
                std::cout << TimePosMs << std::endl;
                emit positionChanged(TimePosMs);
            }
        }
        break;
    default:
        break;
    }
}

void Renderer::on_mpv_events()
{
    while(mpv)
    {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if(event->event_id == MPV_EVENT_NONE)
            break;
        handle_mpv_event(event);
    }
}

QRenderer::QRenderer() :
    AbstractRenderer(),
    player(new QMediaPlayer(nullptr))
{
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(emitPositionChanged(qint64)));
}

QRenderer::~QRenderer()
{
    delete player;
}

void QRenderer::loadMedia(const char *filename)
{
    player->setMedia(QUrl::fromLocalFile(filename));
    player->play();
}

void QRenderer::setVideoOutput(QWidget *widget)
{
    player->setVideoOutput((QVideoWidget*)widget);
}

int QRenderer::getPositionMs() const
{
    return player->position();
}

void QRenderer::emitPositionChanged(qint64 PosMs)
{
    emit positionChanged(PosMs);
}
