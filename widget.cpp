#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    StartCamera();
}

Widget::~Widget()
{
    delete ui;
}

//断线回调函数
static void __stdcall DisConnectFunc(LONG lLoginID, char *pchDVRIP, LONG nDVRPort, DWORD dwUser)
{
    printf("Device disconnect, IP=%s, Port=%d\n", pchDVRIP, nDVRPort);
}

//自动重连回调函数
static void __stdcall HaveReConnectFunc(LONG lLoginID, char *pchDVRIP, LONG nDVRPort, DWORD dwUser)
{
    printf("Device reconnect, IP=%s, Port=%d\n", pchDVRIP);
}

//重连回调
static void CALLBACK HaveReConnect(LLONG lLoginID,char *pchDVRIP,LONG nDVRPort,LDWORD dwUser)
{
    qDebug()<<__FILE__<<"----"<<__LINE__<<"enter CALLBACK HaveReConnect";
    qDebug()<<"lLoginId"<<lLoginID;
    if(NULL != pchDVRIP)
    {
        qDebug()<<"pchDVRIP"<<pchDVRIP;
    }
    qDebug()<<"nDVRPort"<<nDVRPort;
    qDebug()<<"dwUser"<<dwUser;
}

//子连接自动重连回调函数
static void __stdcall SubDisConnectFunc(EM_INTERFACE_TYPE emInterfaceType, BOOL bOnline, LONG lOperateHandle, LONG lLoginID, DWORD dwUser)
{
    switch(emInterfaceType)
    {
    case DH_INTERFACE_REALPLAY:
        printf("实时预览接口: Short connect is %d\n", bOnline);
        break;
    case DH_INTERFACE_PREVIEW:
        printf("多画面预览接口: Short connect is %d\n", bOnline);
        break;
    case DH_INTERFACE_PLAYBACK:
        printf("回放接口: Short connect is %d\n", bOnline);
        break;
    case DH_INTERFACE_DOWNLOAD:
        printf("下载接口: Short connect is %d\n", bOnline);
        break;
    default:
        break;
    }
}

//数据回调
static void __stdcall CALLBACK RealDataCallBackEx2(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LLONG param,LDWORD dwUser)
{
    Widget* pCammerWidget = (Widget*)dwUser;
    if (!pCammerWidget)
    {
        qDebug()<<"pCammerWidget is NULL!";
        return;
    }

    //qDebug() << "RealDataCallBackEx2 Received image data length: " << dwBufSize << "dwDataType is :" << dwDataType;

    //原始音视频混合数据
    if (!PLAY_InputData(pCammerWidget->g_lRealPort,pBuffer,dwBufSize))
    {
        qDebug() << QString("input data error: %1").arg(PLAY_GetLastError(pCammerWidget->g_lRealPort));
    }

}

static void __stdcall DecCBFun(LONG nPort,char * pBuf,LONG nSize,FRAME_INFO * pFrameInfo, void* pUserData, LONG nReserved2)
{

    Widget* pCammerWidget = (Widget*)pUserData;
    if (!pCammerWidget)
    {
        qDebug()<<"pCammerWidget is NULL!";
        //return ;
    }

//    qDebug() << "=============== DecCBFun run ===============";
//    qDebug() << "nWidth :" << pFrameInfo->nWidth << " nHeight :" << pFrameInfo->nHeight << " nSize :" << nSize << " type :" << pFrameInfo->nType;

    if(pFrameInfo->nType == 3){
        QSharedPointer<CameraImageInfo> frameInfo(new CameraImageInfo);
        //frameInfo->cameraId = pCammerWidget->GetCarmeraId();
        frameInfo->nWidth = (int)pFrameInfo->nWidth;
        frameInfo->nHeight = (int)pFrameInfo->nHeight;
        frameInfo->nBufferSize = (int)nSize;
        //frameInfo->ePixelType = pFrame->frameInfo.pixelFormat;
        frameInfo->pImageBuf = new unsigned char[sizeof(unsigned char) * frameInfo->nBufferSize];
        //frameInfo->nTimeStamp = paramInfo->struTime;

        // 内存申请失败，直接返回
        // memory application failed, return directly
        if (frameInfo->pImageBuf != NULL)
        {
            memcpy(frameInfo->pImageBuf, pBuf, frameInfo->nBufferSize);
            pCammerWidget->showImage(frameInfo);
        }
    }
}

static BOOL __stdcall cbMessage(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
//    bool flag{false};
    //qDebug() << "进入报警回调函数";
    switch(lCommand)
    {
    case DH_MOTION_ALARM_EX:		//	0x2102	//Motion detection alarm
    {
        qDebug() << "进入动态检测报警";
        Widget* pCammerWidget = (Widget*)dwUser;
        if (!pCammerWidget)
        {
            qDebug()<<"pCammerWidget is NULL!";
            //return ;
        }
        pCammerWidget->isSave = true;
    }
        break;
    default:
        break;
    }

    return true;
}


int Widget::StartCamera()
{
    //    this->m_streamStart = true;
    //    m_streamThread->start();

    if(! openCamera())
    {
        qDebug()<<"Open Camera failed";
        return -1;
    }

    if(! startCamera())
    {
        qDebug()<<"Start Camera failed";
    }

    return 0;
}

// 注册相机句柄
bool Widget::openCamera()
{

    //获取界面登录信息
    //    strcpy(D_Ip,laddr->text().toLatin1().data());
    //    strcpy(D_UserName,luser->text().toLatin1().data());
    //    strcpy(D_Pasdwd,lpasswd->text().toLatin1().data());
    //    D_Port = lport->text().toInt();

    char D_UserName[64] = "admin";
    char D_Pasdwd[64] = "admin123";
    int D_Port = 37777;
    //SDK初始化
    if(CLIENT_Init((fDisConnect)DisConnectFunc, (LDWORD)0))
    {
        qDebug()<<"SDK INIT OK!";
    }
    else
    {
        qDebug()<<"SDK INIT FAIL!";
    }
    //获取SDK版本信息
    DWORD dwNetSdkVersion = CLIENT_GetSDKVersion();

    qDebug()<<"NetSDK version"<<dwNetSdkVersion;

    //设置断线重连回调接口，设置过断线重连成功回调函数后，当设备出现断线情况，SDK内部会自动进行重连操作
    CLIENT_SetAutoReconnect(HaveReConnect,0);

    //设置登录超时时间和尝试次数
    int nWaitTime = 5000; //登录请求响应超时时间设置为5s
    int nTryTimes = 3; //登录时尝试建立连接3次
    CLIENT_SetConnectTime(nWaitTime,nTryTimes);

    //设置更多网络参数，NET_PARAM的nWaittime,nConnectTryNum成员与CLIENT_SetConnectTime 接口设置的登录设备超时时间和尝试次数的意义相同
    NET_PARAM stuNetparm ={0};
    stuNetparm.nConnectTime = 3000;//登录时尝试建立连接的超时时间
    CLIENT_SetNetworkParam(&stuNetparm);
    // 登录
    NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
    memset(&stInparam, 0, sizeof(stInparam));
    stInparam.dwSize = sizeof(stInparam);
    strncpy(stInparam.szIP, m_cameraIp, sizeof(stInparam.szIP) - 1);
    strncpy(stInparam.szPassword, D_Pasdwd, sizeof(stInparam.szPassword) - 1);
    strncpy(stInparam.szUserName, D_UserName, sizeof(stInparam.szUserName) - 1);
    stInparam.nPort = D_Port;
    stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;
    tagNET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
    memset(&stOutparam, 0, sizeof(stOutparam));
    stOutparam.dwSize = sizeof(stOutparam);
    LoginHandle = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);
    if (LoginHandle)
    {
        qDebug()<<"登陆句柄LoginHandle"<<LoginHandle;
    }
    else
    {
        qDebug()<<"login fail :" << stOutparam.nError;
    }

//    // 设置参数功能实现
//    char *szOutBuffer = new char[32*1024];
//    if (szOutBuffer == NULL)
//    {

//    }
//    memset(szOutBuffer, 0, 32*1024);

//    int nerror = 0;
//    CFG_ENCODE_INFO stuEncodeInfo = {0};
//    int nrestart = 0;

//    BOOL bSuccess = CLIENT_GetNewDevConfig(LoginHandle, (char *)CFG_CMD_ENCODE, 0, szOutBuffer, 32*1024, &nerror, 5000);
//    if (bSuccess)
//    {
//        int nRetLen = 0;
//        //解析
//        BOOL bRet = CLIENT_ParseData((char *)CFG_CMD_ENCODE, (char *)szOutBuffer, &stuEncodeInfo, sizeof(CFG_ENCODE_INFO), &nRetLen);
//        if (bRet == FALSE)
//        {
//            printf("CLIENT_ParseData: CFG_CMD_ENCODE failed!\n");
//        }
//    }
//    else
//    {
//        printf("CLIENT_GetNewDevConfig: CFG_CMD_ENCODE failed!\n");

//    }

//    //stuEncodeInfo.stuSnapFormat[0].stuVideoFormat.emCompression;
//    stuEncodeInfo.stuMainStream[0].stuVideoFormat.nWidth = 1920;
//    stuEncodeInfo.stuMainStream[0].stuVideoFormat.nHeight = 1080;
//    // 修改抓图分辨率
//    qDebug() << "抓图通道视频编码格式 :" << stuEncodeInfo.stuMainStream[0].stuVideoFormat.emCompression;
//    qDebug() << "抓图通道视频宽度 :" << stuEncodeInfo.stuMainStream[0].stuVideoFormat.nWidth;
//    qDebug() << "抓图通道视频高度 :" << stuEncodeInfo.stuMainStream[0].stuVideoFormat.nHeight;
//    memset(szOutBuffer, 0, 32*1024);

//    bSuccess = CLIENT_PacketData((char *)CFG_CMD_ENCODE, (char *)&stuEncodeInfo, sizeof(CFG_ENCODE_INFO), szOutBuffer, 32*1024);
//    if (bSuccess == FALSE)
//    {
//        qDebug() << "CLIENT_PacketData: CFG_CMD_ENCODE failed!";
//    }
//    else
//    {
//        bSuccess = CLIENT_SetNewDevConfig(LoginHandle, (char *)CFG_CMD_ENCODE, 0, szOutBuffer, 32*1024, &nerror, &nrestart, 3000);
//        if (bSuccess)
//        {
//            qDebug() << "nrestart :" << nrestart;
//            if (nrestart == 1)
//            {
//                qDebug() << "Save config info successfully!devide need restart!";
//            }else if(nrestart == 0){
//                qDebug() << "Save config info successfully!";
//            }
//        }
//        else
//        {
//            qDebug() << "CLIENT_SetNewDevConfig CFG_CMD_ENCODE failed!";
//        }
//    }
//    delete []szOutBuffer;


    DWORD dwRetLen = 0;
    DH_MOTION_DETECT_CFG_EX *pChannelInfo = new DH_MOTION_DETECT_CFG_EX;

    memset(pChannelInfo, 0, sizeof(DH_MOTION_DETECT_CFG_EX));
    BOOL bSuccess = CLIENT_GetDevConfig(LoginHandle, DH_DEV_MOTIONALARM_CFG, 0, pChannelInfo, sizeof(DH_MOTION_DETECT_CFG_EX), &dwRetLen);
    if (!(bSuccess&&dwRetLen == sizeof(DH_MOTION_DETECT_CFG_EX)))
    {
        printf("CLIENT_GetDevConfig: DH_DEV_CHANNELCFG failed!\n");
    }

    qDebug() << "动态检测使能开关： " << pChannelInfo->byMotionEn;
    qDebug() << "动态检测灵敏度:" << pChannelInfo->wSenseLevel;
    pChannelInfo->byMotionEn = 1;
    pChannelInfo->wSenseLevel = 6;

    bSuccess = CLIENT_SetDevConfig(LoginHandle, DH_DEV_MOTIONALARM_CFG, 0, pChannelInfo, sizeof(DH_MOTION_DETECT_CFG_EX));
    if (bSuccess == FALSE)
    {
        printf("CLIENT_SetDevConfig: DH_DEV_CHANNELCFG failed!\n");
    }

    delete []pChannelInfo;




    return true;
}

// 开始取流
bool Widget::startCamera()
{

    if(FALSE == LoginHandle)
    {
        qDebug() << "连接失败";
        return false;
    }
//    if(lplay == nullptr){
//        qDebug() << "没有显示句柄";
//        return false;
//    }
    //获取Label句柄
    //HWND hWnd = (HWND)lplay->winId();
    HWND hWnd = nullptr;

    // 获取播放通道
    PLAY_GetFreePort(&g_lRealPort);
    // 初始化播放库
    PLAY_OpenStream(g_lRealPort,0,0,1024*500);
    // 开始播放
    PLAY_Play(g_lRealPort, NULL);

    CLIENT_StartListenEx(LoginHandle);
    CLIENT_SetDVRMessCallBack(cbMessage, (LDWORD)this);

    DH_RealPlayType emRealPlayType = DH_RType_Realplay; // 实时预览
    // 预览预览
    lHandle = CLIENT_RealPlayEx(LoginHandle, 0, hWnd, emRealPlayType);
    if(0 != lHandle)
    {
        // 原始数据
        DWORD dwFlag = REALDATA_FLAG_RAW_DATA;
        // 设置回调函数处理数据
        if(!CLIENT_SetRealDataCallBackEx2(lHandle, RealDataCallBackEx2, (LDWORD)this,dwFlag)){
            qDebug() << "设置回调函数失败！！";
        }else{
            qDebug() << "设置回调函数成功";
        }
        // 设置播放库回调函数
        if(!PLAY_SetDecCallBackEx(g_lRealPort, DecCBFun, this)){
            qDebug() << "设置play回调函数失败！！";
        }else{
            qDebug() << "设置play回调函数成功 ！！";
        }

    }
    else
    {
        printf("Fail to play!\n");
    }

    return true;
}

void Widget::showImage(QSharedPointer<CameraImageInfo> frameInfo)
{
    cv::Mat frame(frameInfo->nHeight + frameInfo->nHeight / 2, frameInfo->nWidth, CV_8UC1, frameInfo->pImageBuf);
    cv::Mat bgr;
    cv::cvtColor(frame, bgr, cv::COLOR_YUV2RGB_YV12);

    if(isSave){
        QDateTime date = QDateTime::currentDateTime();
        QString dateString = date.toString("yyyyMMdd_hhmmss");
        cv::imwrite(QString("./jpg/%1.jpg").arg(dateString).toStdString(), bgr);
        isSave = false;
    }

    QImage image((const unsigned char*)bgr.data, bgr.cols, bgr.rows, bgr.step, QImage::Format_RGB888);
    // BRG转为RGB
    image = image.rgbSwapped();

    QImage imageScale = image.scaled(QSize(ui->showLabel->width(), ui->showLabel->height()));
    QPixmap pixmap = QPixmap::fromImage(imageScale);
    ui->showLabel->setPixmap(pixmap);
}

