#ifndef ANIMATION_FRAMES_PLAYER_H
#define ANIMATION_FRAMES_PLAYER_H

#include <QScrollArea>
#include <QPixmap>
namespace ady{
class AnimationFramesPlayerPrivate;
class AnimationFramesPlayer : public QScrollArea
{
    Q_OBJECT
public:
    explicit AnimationFramesPlayer(QWidget *parent = nullptr);
    ~AnimationFramesPlayer();

    void load(const QPixmap& image);
    void setText(const QString& text);
signals:

protected:
    virtual void resizeEvent(QResizeEvent* e) override;
    //virtual void scrollContentsBy(int dx, int dy) override;
private:
    AnimationFramesPlayerPrivate* d;
};
}
#endif // ANIMATION_FRAMES_PLAYER_H
