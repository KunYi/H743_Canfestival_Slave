#include "bsp_canopen.h"

/* 外部变量 */
extern CO_Data Slave1_Data;

/* 内部变量 */
static TimerCallback_t init_callback;
//s_BOARD MasterBoard = {"1", "1M"};

_CANOPEN_NODE_HANDLER CANopen_Slave;       /* CANopen对象1 */

/**
  * 函数功能: 初始化CANopen对象
  * 输入参数: void
  *
  * 返回值：  void
  *
  *
  * 说明:
  *
  */
void CANopen_handler_init(void)
{
    /* 初始化CANopen_Master1对象 */
    CANopen_Slave.node_id = 0x01;                														/* Node_ID */
    CANopen_Slave.unpack_Status = wait_msg_head; 														/* 状态机的状态 */
    CANopen_Slave.object_DIC = &Slave1_Data;     														/* 对象字典 */
    CANopen_Slave.object_DIC->canHandle = (CAN_PORT)CANopen_Slave.node_id;  /* 设置接口函数canSend的通道 */
    /* 函数指针 */
    CANopen_Slave.canSend = canSend;
    CANopen_Slave.send_NMT_MSG = send_NMT_Manage_Message;


    /* 初始化CANopen_Master2对象 */
    //CANopen_Master2.node_id = 0x02;                /* Node_ID */
    //CANopen_Master2.unpack_Status = wait_msg_head; /* 状态机的状态 */
    //CANopen_Master2.object_DIC = &Master2_Data;    /* 对象字典 */
    //CANopen_Master2.object_DIC->canHandle = (CAN_PORT)CANopen_Master2.node_id;  /* 设置接口函数canSend的通道 */
    /* 函数指针 */
    //CANopen_Master2.canSend = canSend;
    //CANopen_Master2.send_NMT_MSG = send_NMT_Manage_Message;

}

/**
  * 函数功能: 将canopen协议的报文，从FDCAN发出去。
  * 输入参数: Message *m
  *
  * 返回值：  void
  *
  *
  * 说明:
  *
  */
uint8_t canopen_To_FDCAN_Send(FDCAN_HandleTypeDef *fdcan,FDCAN_TxHeaderTypeDef *fdcan_TX ,Message *m)
{
    uint8_t tx_Buffer[8] = {0};

    fdcan_TX->Identifier = m->cob_id;                   /* 获取COB-ID */
    fdcan_TX->IdType = FDCAN_STANDARD_ID;               /* canopen都是标准帧(canopen淘汰了拓展帧) */
    fdcan_TX->ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    fdcan_TX->BitRateSwitch = FDCAN_BRS_OFF;            /*关闭速率切换(标准CAN不能切换速率) */
    fdcan_TX->FDFormat = FDCAN_CLASSIC_CAN;             /* 标准CAN模式（非FDCAN） */
    fdcan_TX->TxEventFifoControl = FDCAN_NO_TX_EVENTS;  /* 没有发送事件 */
    fdcan_TX->MessageMarker= 0;                         /* 设0 */

    /* 根据所接收的CAN报文长度，将没用的Buffer清0.(比如当收到的长度为0是，Buffer[0]~Bufer[7]都必须设为0x00) */
    switch (m->len)
    {
        case 0x00:
            fdcan_TX->DataLength = FDCAN_DLC_BYTES_0;    /* 设置CAN报文长度 */
            break;

        case 0x01:
            fdcan_TX->DataLength = FDCAN_DLC_BYTES_1;    /* 设置CAN报文长度 */
            memcpy(tx_Buffer,m->data,1);                 /* 按照DCL长度，获取报文的内容 */
            break;

        case 0x02:
            fdcan_TX->DataLength = FDCAN_DLC_BYTES_2;
            memcpy(tx_Buffer,m->data,2);
            break;

        case 0x03:
            fdcan_TX->DataLength = FDCAN_DLC_BYTES_3;
            memcpy(tx_Buffer,m->data,3);
            break;

        case 0x04:
            fdcan_TX->DataLength = FDCAN_DLC_BYTES_4;
            memcpy(tx_Buffer,m->data,4);
            break;

        case 0x05:
            fdcan_TX->DataLength = FDCAN_DLC_BYTES_5;
            memcpy(tx_Buffer,m->data,5);
            break;

        case 0x06:
            fdcan_TX->DataLength = FDCAN_DLC_BYTES_6;
            memcpy(tx_Buffer,m->data,6);
            break;

        case 0x07:
            fdcan_TX->DataLength = FDCAN_DLC_BYTES_7;
            memcpy(tx_Buffer,m->data,7);
            break;

        case 0x08:
            fdcan_TX->DataLength = FDCAN_DLC_BYTES_8;
            memcpy(tx_Buffer,m->data,8);
            break;
    }

    /* 判断需要发送的报文是远程帧还是数据帧 */
    if(1 == m->rtr)
        fdcan_TX->TxFrameType = FDCAN_REMOTE_FRAME;    /* 远程帧 */
    else
        fdcan_TX->TxFrameType = FDCAN_DATA_FRAME;      /* 数据帧 */

    /* 调用底层将报文发出去 */
    if(HAL_FDCAN_AddMessageToTxFifoQ(fdcan,fdcan_TX,tx_Buffer)== HAL_OK)
    {
        return 0xff;
    }
    else
    {
        return 0x00;
    }
}


/**
  * 函数功能: 发送NMT管理报文
  * 输入参数: data：对象字典结构体
              nmt:  NMT管理报文
  * 返 回 值: 无
  * 说    明:
  *
  */
void send_NMT_Manage_Message(CO_Data* data,_NMT_MODULE_CONTORL nmt)
{
    switch (nmt)
    {
        case START_REMOTE_NODE:
            masterSendNMTstateChange(data,0x00,nmt);
            break;
        case STOP_REMOTE_NODE:
            masterSendNMTstateChange(data,0x00,nmt);
            break;
        case ENTER_PRE_OPERATIONAL_STATE:
            masterSendNMTstateChange(data,0x00,nmt);
            break;
        case RESET_NODE:
            masterSendNMTstateChange(data,0x00,nmt);
            break;
        case RESET_COMMUNICATION:
            masterSendNMTstateChange(data,0x00,nmt);
            break;
    }
}


/**
  * 函数功能: CANopen，Node-ID初始化
  * 输入参数: notused：结构体 m：数据
  * 返 回 值: 无
  * 说    明:
  *         1. 只初始化FDCAN1的CANopen Node,如果需要FDCAN2的CANopen Node就需要在文件夹Object_dir添加对象文件(.c与.h)
  *            然后，将这个函数的CANopen 节点2初始化的备注删除即可。
  */
void InitNodes(CO_Data* d, UNS32 id)
{
	/****************************** INITIALISATION MASTER *******************************/

    /* Canopen 节点1 初始化 */
    printf("CANopen Node of FDCAN1 is going to initialize.\r\n");
    setNodeId(CANopen_Slave.object_DIC,CANopen_Slave.node_id);
    /* Master1 init */
    setState(CANopen_Slave.object_DIC,Initialisation);
    setState(&Slave1_Data, Pre_operational);
    printf("CANopen Node of FDCAN1 was successfully initialized.\r\n");

	/*********** FDCAN2暂时不运行CANopen协议 **********/
    /* Canopen 节点2 初始化 */
    //printf("主机2端状态设置\n");
    //setNodeId(CANopen_Master2.object_DIC,CANopen_Master2.node_id);
    /* Master2 init */
    //setState(&Master2_Data, Initialisation);
    //setState(&Master2_Data, Operational);
    //printf("主机2初始化完毕\n");

}


/**
  * 函数功能: 开启定时器
  * 输入参数:
  *
  * 返回值：
  *
  *
  * 说明:
  *
  */
void StartTimerLoop(TimerCallback_t _init_callback)
{
	init_callback = _init_callback;

	SetAlarm(NULL, 0, init_callback, 0, 0);

}

/**
  * 函数功能: 初始化Canopen节点
  * 输入参数:
  *
  * 返回值：
  *
  *
  * 说明:
  *
  */
//void CANopen_Init(void *parameter)
void CANopen_Init(void)
{
    printf("Starting to Init the CANopen object. \n");
    CANopen_handler_init();
    StartTimerLoop(&InitNodes);
}

/**
  * 函数功能: canfestival的接口函数(canSend)
  * 输入参数: notused：结构体 m：数据
  * 返 回 值: 无
  * 说    明:
  *       1. 通过入口参数canHandler，决定使用FDCAN1硬件还是FDCAN2硬件
  *       2. 第一个入口参数，默认是CO_Data结构体的成员canHandle
  */
unsigned char canSend(CAN_PORT canHandle, Message *m)
{

    /* 看看是哪一个canopen Node需要发送报文 */
    if(CANopen_Slave.object_DIC->canHandle == canHandle)
    {
        return canopen_To_FDCAN_Send(&hfdcan1,&hfdcan1_TX,m);
    }

	/*********** FDCAN2暂时不运行CANopen协议 **********/
//    else if(CANopen_Master2.object_DIC->canHandle == canHandle)
//    {
//        return canopen_To_FDCAN_Send(&hfdcan2,&hfdcan2_TX,m);
//    }

    return 0;
}


/**
  * 函数功能: 将接收到的CAN报文，移交给canopen协议
  * 输入参数: _CANMSG *msg
  *
  * 返回值：  void
  *
  *
  * 说明:
  *
  */
void CANopen_Get_MSG(struct canopen_Node_handler *CANopen_Handler,_CANMSG *msg)
{
    if(&CANopen_Slave == CANopen_Handler)
    {
        CANopen_Slave.RxMSG.cob_id = (UNS16)msg->id;    /* 获取cob id */
        CANopen_Slave.RxMSG.len =    (UNS8)msg->len;    /* 获取报文长度 */
        CANopen_Slave.RxMSG.rtr =    (UNS8)msg->rtr;    /* 获取报文的类型 */
        memcpy(CANopen_Slave.RxMSG.data,(uint8_t *)msg->buffer,8); /* 获取8为报文 */

        /* canopen接口函数 */
        canDispatch(&Slave1_Data,&CANopen_Slave.RxMSG);
    }

	/*********** FDCAN2暂时不运行CANopen协议 **********/

    //if(&CANopen_Master2 == CANopen_Handler)
    //{
        //
    //}

}














