#ifndef CAMERAIMAGEINFO_H
#define CAMERAIMAGEINFO_H


#include "include/Common/dhnetsdk.h"
#include "include/Common/dhconfigsdk.h"
//#include "include/Common/avglobal.h"

#include <QObject>
#include <QDebug>

enum CameraID
{
    InvalidCamera = 0,
    FirstCamera = 1,
    SecondCamera = 2,
    ThirdCamera = 3,
    FourCamera = 4
};

Q_DECLARE_METATYPE(CameraID)

struct CameraImageInfo
{
    unsigned int    cameraId;
    unsigned char   *pImageBuf;
    unsigned int	nBufferSize;
    int				nWidth;
    int				nHeight;
    //IMV_EPixelType	ePixelType{gvspPixelMono8};
    int				nPaddingX;
    int				nPaddingY;
    uint64_t		nTimeStamp;

    ~CameraImageInfo()
    {
        if(pImageBuf)
        {
            //qDebug()<<"CameraImageInfo delete imageData";
            delete[] pImageBuf;
            pImageBuf = nullptr;
        }
    }
};

Q_DECLARE_METATYPE(CameraImageInfo)


#endif // CAMERAIMAGEINFO_H
