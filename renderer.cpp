#include "renderer.h"

#include <mpv/client.h>

#include <stdexcept>

Renderer::Renderer(const char *filename) :
    mpv(mpv_create())
{
    if(!mpv)
        throw std::runtime_error("Can't initialize mpv");

    mpv_set_option_string(mpv, "vo", "null");

    if(mpv_initialize(mpv) < 0)
        throw std::runtime_error("mpv failed to initialize");

    const char *args[] = {"loadfile", filename, nullptr};
    mpv_command(mpv, args);
    //mpv_set_property_string(mpv, "pause", "true");
}

Renderer::~Renderer()
{
    if(mpv)
        mpv_terminate_destroy(mpv);
}

int Renderer::getPositionMs()
{
    double pos;
    mpv_get_property(mpv, "time-pos", MPV_FORMAT_DOUBLE, &pos);
    return pos * 1000.0;
}
