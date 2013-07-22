#ifndef TIMELINEGL_H
#define TIMELINEGL_H

#include <QGLWidget>
#include <QMouseEvent>
#include <misc/global.h>
#include "items/uifileitem.h"
#include "core/project.h"

class TimelineGL : public GlWidget {
    Q_OBJECT
public:
    explicit TimelineGL(QWidget *parent = 0);

private:
    qreal showLegend;
    QList<GlText> categories;

protected:
    void timerEvent(QTimerEvent *);
    void initializeGL();
    void resizeGL(int, int);
    void wheelEvent(QWheelEvent *);
    void paintGL();

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mouseMove(QMouseEvent *, bool, bool);
    void leaveEvent(QEvent *);
public slots:
    void mouseMoveLong();

protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    
signals:
    
public slots:
    
};

#endif // TIMELINEGL_H
