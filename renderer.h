#ifndef RENDERER_H
#define RENDERER_H

struct mpv_handle;

class Renderer
{
public:
    Renderer(const char *filename);
    ~Renderer();

    int getPositionMs();

private:
    mpv_handle *mpv;
};

#endif // RENDERER_H
