#ifndef DEBUGIMAGEOBJECT_H
#define DEBUGIMAGEOBJECT_H

#include <QObject>

#include <opencv2/core.hpp>

class DebugImageObject:
        public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool debugEnabled READ debugEnabled WRITE setDebugEnabled NOTIFY debugEnabledChanged)
public:
    DebugImageObject();

    bool debugEnabled() const;
    void setDebugEnabled(bool debugEnabled);

    virtual cv::Mat debugImage() const = 0;

signals:
    void debugEnabledChangled();

private:
    bool m_debugEnabled;
};

#endif // DEBUGIMAGEOBJECT_H
