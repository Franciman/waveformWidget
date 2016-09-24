#ifndef RENDERER_H
#define RENDERER_H

#include <QObject>

class QMediaPlayer;

struct mpv_handle;
struct mpv_event;

class AbstractRenderer : public QObject
{
    Q_OBJECT
public:
    AbstractRenderer();

    virtual ~AbstractRenderer();

    virtual int getPositionMs() const = 0;

    virtual void loadMedia(const char *filename) = 0;

    virtual void setVideoOutput(QWidget *widget) = 0;
};

class Renderer : public AbstractRenderer
{
    Q_OBJECT
public:
    Renderer();
    ~Renderer();

    int getPositionMs() const override
    {
        return TimePosMs;
    }

    void loadMedia(const char *filename);

    void setVideoOutput(QWidget *widget);

signals:
    void mpv_events();

private slots:
    void on_mpv_events();

private:
    void handle_mpv_event(mpv_event *event);

private:
    mpv_handle *mpv;
    int TimePosMs;
};

class QRenderer : public AbstractRenderer
{
    Q_OBJECT
public:
    QRenderer();
    ~QRenderer();

    int getPositionMs() const override;

    void loadMedia(const char *filename);

    void setVideoOutput(QWidget *widget);

private:
    QMediaPlayer *player;
};

#endif // RENDERER_H
