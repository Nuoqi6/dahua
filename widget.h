#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QDateTime>

#include "include/Common/dhnetsdk.h"
#include "include/Common/dhconfigsdk.h"
#include "include/CameraImageInfo.h"
#include "include/play.h"
#include "opencv2/opencv.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

#pragma execution_character_set("utf-8")

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    int StartCamera();
    bool openCamera();
    bool startCamera();
    void showImage(QSharedPointer<CameraImageInfo> frameInfo);
    LONG g_lRealPort;// 全局播放库port号
    bool isSave{false};

private:
    Ui::Widget *ui;

    char   m_cameraIp[64]{"182.168.0.122"};
    LLONG LoginHandle;//登录句柄

    LLONG lHandle;//监视句柄
};
#endif // WIDGET_H
