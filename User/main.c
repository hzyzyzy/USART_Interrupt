/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/25
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *USART interrupt routine:
 *USART1_Tx(PD5)\USART1_Rx(PD6).

 *This routine demonstrates that two boards use query to send and interrupt to
 *receive. After successful sending and receiving, PD0 is connected to LED,
 *and the LED light flashes.
 *
 *Hardware connection:PD5 -- PD6
 *                    PD6 -- PD5
 *                    PD0 -- LED
 *
 */

#include "debug.h"
#include "string.h"
#include "ws2812.h"
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

// ring buffer size
#define RING_BUFFER_LEN     (1024u)

// The length of a single buffer used by DMA
#define RX_BUFFER_LEN       (128u)

#define USART_RX_CH         DMA1_Channel5
#define DATA_LENGTH 13
#define SEGMENT1_LENGTH 4
#define SEGMENT2_LENGTH 3
#define SEGMENT3_LENGTH 3
#define SEGMENT4_LENGTH 3

int numberone= 0;
int numbertwo= 0;
int numberthree= 0;

int numberone1= 0;
int numbertwo2= 0;
int numberthree3= 0;

int numberone11= 0;
int numbertwo22= 0;
int numberthree33= 0;



    int ione = 0;
    int itwo =0;
    int ithree =0;

    int ioneone = 0;
        int itwoone =0;
        int ithreeone =0;

        int ioneone1 = 0;
        int itwoone2 =0;
        int ithreeone3 =0;


char segments[2][4][5];

void chuli_proc(char message[]);


struct
{
    volatile uint8_t DMA_USE_BUFFER;
    uint8_t          Rx_Buffer[2][RX_BUFFER_LEN];

} USART_DMA_CTRL = {
    .DMA_USE_BUFFER = 0,
    .Rx_Buffer      = {0},
};

struct
{
    uint8_t           buffer[RING_BUFFER_LEN];
    volatile uint16_t RecvPos;  //
    volatile uint16_t SendPos;  //
    volatile uint16_t RemainCount;

} ring_buffer = {{0}, 0, 0, 0};
uint8_t email[100];
/*********************************************************************
 * @fn      USARTx_CFG
 *
 * @brief   Initializes the USART1 peripheral.
 *
 * @return  none
 */
void USARTx_CFG(uint32_t baudrate)
{
    GPIO_InitTypeDef  GPIO_InitStructure  = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef  NVIC_InitStructure  = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD |RCC_APB2Periph_USART1, ENABLE);

    /* USART1 TX-->D.5   RX-->D.6.1 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�򿪴��ڿ����ж�

    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);



}


/*********************************************************************
 * @fn      DMA_INIT
 *
 * @brief   Configures the DMA for USART1.
 *
 * @return  none
 */
void DMA_INIT(void)
{
    DMA_InitTypeDef  DMA_InitStructure  = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(USART_RX_CH);
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;

    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)USART_DMA_CTRL.Rx_Buffer[0];
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize         = RX_BUFFER_LEN;
    DMA_Init(USART_RX_CH, &DMA_InitStructure);

    DMA_ITConfig(USART_RX_CH, DMA_IT_TC, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_Cmd(USART_RX_CH, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
}


/*********************************************************************
 * @fn      ring_buffer_push_huge
 *
 * @brief   Put a large amount of data into the ring buffer.
 *
 * @return  none
 */
void ring_buffer_push_huge(uint8_t *buffer, uint16_t len)
{
    const uint16_t bufferRemainCount = RING_BUFFER_LEN - ring_buffer.RemainCount;
    if (bufferRemainCount < len)
    {
        len = bufferRemainCount;
    }//��黷�λ�������ʣ��ռ�

    const uint16_t bufferSize = RING_BUFFER_LEN - ring_buffer.RecvPos;
    if (bufferSize >= len)
    {
        memcpy(&(ring_buffer.buffer[ring_buffer.RecvPos]), buffer, len);
        ring_buffer.RecvPos += len;
    }
    else
    {
        uint16_t otherSize = len - bufferSize;
        memcpy(&(ring_buffer.buffer[ring_buffer.RecvPos]), buffer, bufferSize);
        memcpy(ring_buffer.buffer, &(buffer[bufferSize]), otherSize);
        ring_buffer.RecvPos = otherSize;
    }//���㵱ǰ����λ�õ����λ�����ĩβ�Ŀռ�
    ring_buffer.RemainCount += len;//���»��λ�������ʣ�����ݼ���
}

/*********************************************************************
 * @fn      ring_buffer_pop
 *
 * @brief   Get a data from the ring buffer.
 *
 * @return  the Data
 */
uint8_t ring_buffer_pop()
{
    uint8_t data = ring_buffer.buffer[ring_buffer.SendPos];//��ȡ����

    ring_buffer.SendPos++;
    if (ring_buffer.SendPos >= RING_BUFFER_LEN)
    {
        ring_buffer.SendPos = 0;
    }//���·���λ��
    ring_buffer.RemainCount--;//����ʣ�����ݼ���
    return data;
}

void uint8ArrayToCharArray(uint8_t* uint8Array, int length, char* charArray) {
    for (int i = 0; i < length; ++i) {
        charArray[i] = (char)uint8Array[i];
    }
    charArray[length] = '\0';  // ����ַ�����ֹ��
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    //u8 i;

    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Delay_Init();
    USART_Printf_Init(115200);
    USARTx_CFG(115200);
    DMA_INIT();
   //printf("SystemClk:%d\r\n", SystemCoreClock);
//    printf("This is an example\r\n");
    ws281x_init();

    int i=0;
   //uint16_t message=3;
        //ws281x_show();
   // chuli_proc();
//    ws281x_setPixelRGB(0,0,0,0);
//                                                                                                   ws281x_setPixelRGB(1,0,0,0);
//                                                                                                   ws281x_setPixelRGB(2,0,0,0);
//                     ws281x_show();
    while (1)
    {
        //ws281x_closeAll();
        if (ring_buffer.RemainCount > 0)
        {
            while (ring_buffer.RemainCount > 0)
            {
               email[i]= ring_buffer_pop();

                 i++;
                 ws281x_setPixelRGB(0,0,0,0);
                                                                                               ws281x_setPixelRGB(1,0,0,0);
                                                                                               ws281x_setPixelRGB(2,0,0,0);
                // ws281x_closeAll();


            }
               i=0;
//               for (int j = 0; j <strlen(email); ++j) {
//                               // printf("%c",email[j]);
//               }
               char message[51];
               uint8ArrayToCharArray(email, strlen(email), message);
//               for (int j = 0; j <strlen(message); ++j) {
//                   printf("%c", message[j]);
//                   //ws281x_closeAll();
//
//
//            }
               //ws281x_closeAll();
               chuli_proc(message);

        }


        Delay_Ms(1000);
    }


}


/*********************************************************************
 * @fn      Send_Byte
 *
 * @brief   This function handles USART1 global interrupt request.
 *
 * @return  none
 */

//void Send_Byte(uint32_t Dat)
//{
//    USART_SendData(USART1, Dat);
//    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//}

/*********************************************************************
 * @fn      Send_array
 *
 * @brief   This function handles USART1 global interrupt request.
 *
 * @return  none
 */
//void Send_array(uint32_t *arr,uint32_t len)
//{
//   uint8_t i=0;
//   for (i = 0; i < len; i++) {
//    Send_Byte(arr[i]);
//
//}
//
//}
/*********************************************************************
 * @fn      USART1_IRQHandler
 *
 * @brief   This function handles USART1 global interrupt request.
 *
 * @return  none
 */


void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)//���USART1�Ŀ����жϱ�־λ�Ƿ���λ���Ƿ�������״̬��������ǣ������жϴ���
    {
        // IDLE
        uint16_t rxlen     = (RX_BUFFER_LEN - USART_RX_CH->CNTR);//���㵱ǰ�������н��յ������ݳ��ȣ�RX_BUFFER_LEN�ǻ��������ܳ��ȣ�USART_RX_CH->CNTR��DMA��ǰʣ��Ĵ������������ʾ��δ���յ������ݳ��ȡ���ˣ����ܳ��ȼ�ȥ��������ֵ�͵õ��ѽ��յ����ݳ��ȡ�
        uint8_t  oldbuffer = USART_DMA_CTRL.DMA_USE_BUFFER;//���浱ǰʹ�õĻ���������

        USART_DMA_CTRL.DMA_USE_BUFFER = !oldbuffer;//�л�������

        DMA_Cmd(USART_RX_CH, DISABLE);
        DMA_SetCurrDataCounter(USART_RX_CH, RX_BUFFER_LEN);//ͣ��DMA���䣬����DMA�����ݼ���������Ϊ�������ܳ��ȣ���׼����һ�δ���
        // Switch buffer
        USART_RX_CH->MADDR = (uint32_t)(USART_DMA_CTRL.Rx_Buffer[USART_DMA_CTRL.DMA_USE_BUFFER]);//�л�DMA���ڴ����ַ���µĻ�������ʹ��DMA���µĻ�������ʼ��������
        DMA_Cmd(USART_RX_CH, ENABLE);//��������DMA���䣬�Կ�ʼ����������

        USART_ReceiveData(USART1); // clear IDLE flag ��ȡUSART1�����ݼĴ���������������жϱ�־λ�����Ǳ�Ҫ�Ĳ��裬��������жϱ�־λ���ᱻ������ᵼ���жϴ�������ٴδ�����
        ring_buffer_push_huge(USART_DMA_CTRL.Rx_Buffer[oldbuffer], rxlen);//���ոս��յ������ݣ��洢�ھɻ������У����͵����λ������У�����������ʹ��
    }
}

/*********************************************************************
 * @fn      DMA1_Channel5_IRQHandler
 *
 * @brief   This function handles DMA1 Channel 5 global interrupt request.
 *
 * @return  none
 */
void DMA1_Channel5_IRQHandler(void)
{
    uint16_t rxlen     = RX_BUFFER_LEN;//���趨ΪDMA����������
    uint8_t  oldbuffer = USART_DMA_CTRL.DMA_USE_BUFFER;//���浱ǰ����ʹ�õ�DMA����������
    // FULL

    USART_DMA_CTRL.DMA_USE_BUFFER = !oldbuffer;//�л�������

    DMA_Cmd(USART_RX_CH, DISABLE);//��ͣ��ǰDMA����
    DMA_SetCurrDataCounter(USART_RX_CH, RX_BUFFER_LEN);//����DMA����������ΪRX_BUFFER_LEN��128�ֽڣ�
    // Switch buffer
    USART_RX_CH->MADDR = (uint32_t)(USART_DMA_CTRL.Rx_Buffer[USART_DMA_CTRL.DMA_USE_BUFFER]);//�л�DMA�ڴ��ַ���µĻ�������ʹ���л����������
    DMA_Cmd(USART_RX_CH, ENABLE);//��������DMA����

    ring_buffer_push_huge(USART_DMA_CTRL.Rx_Buffer[oldbuffer], rxlen);//���ɻ����������ݣ�USART_DMA_CTRL.Rx_Buffer[oldbuffer]�����͵����λ�����

    DMA_ClearITPendingBit(DMA1_IT_TC5);//���DMA1ͨ��5�Ĵ�������жϹ���λ���Ա������µ��ж�����
}

//uint8_t char_to_int(char c) {
//    if (c >= '0' && c <= '9') {
//        return c - '0';
//    } else {
//        // �Ƿ��ַ���������0��������������
//        return 0;
//    }
//}

void chuli_proc(char message[])
{
    int length = strlen(message);  // �����ַ�������
    int result = length / 13;   // ���㳤�ȳ���13����
    int b=result+1;
   //ws281x_closeAll();
    for(int a=1;a<b;a=a+1)
    {
        //ws281x_closeAll();
    strncpy(segments[a-1][0], message+13*(a-1), SEGMENT1_LENGTH);
    segments[a-1][0][5] = '\0'; // Null-terminate the string

        strncpy( segments[a-1][1], message + 4+13*(a-1), SEGMENT2_LENGTH);
        segments[a-1][1][4]= '\0';

        strncpy(segments[a-1][2], message + 7+13*(a-1), SEGMENT3_LENGTH);
        segments[a-1][2][4] = '\0';

        strncpy( segments[a-1][3], message + 10+13*(a-1), SEGMENT4_LENGTH);
        segments[a-1][3][4] = '\0';


        if(strcmp(segments[a-1][0], "0102") == 0)
               {

                   while (segments[a-1][1][ione] != '\0') {
                                  // ���ַ�ת��Ϊ��Ӧ�����ֲ��ۼӵ������
                                  numberone = numberone * 10 + (segments[a-1][1][ione] - '0');
                                  ione++;
                              }

                           while (segments[a-1][2][itwo] != '\0') {
                                         // ���ַ�ת��Ϊ��Ӧ�����ֲ��ۼӵ������
                                         numbertwo = numbertwo * 10 + (segments[a-1][2][itwo] - '0');
                                         itwo++;
                                     }

                           while (segments[a-1][3][ithree] != '\0') {
                                         // ���ַ�ת��Ϊ��Ӧ�����ֲ��ۼӵ������
                                         numberthree = numberthree * 10 + (segments[a-1][3][ithree] - '0');
                                         ithree++;
                                     }
                   ws281x_setPixelRGB(1,numberone,numbertwo,numberthree);

               }




                                   if(strcmp(segments[a-1][0], "0203") == 0)
                                   {
//                                       numberone1=0;
//                                       numbertwo2=0;
//                                       numberthree3=0;
                                       while (segments[a-1][1][ioneone] != '\0') {
                                                            // ���ַ�ת��Ϊ��Ӧ�����ֲ��ۼӵ������
                                                            numberone1 = numberone1 * 10 + (segments[a-1][1][ioneone] - '0');
                                                            ioneone++;
                                                        }

                                                     while (segments[a-1][2][itwoone] != '\0') {
                                                                   // ���ַ�ת��Ϊ��Ӧ�����ֲ��ۼӵ������
                                                                   numbertwo2 = numbertwo2 * 10 + (segments[a-1][2][itwoone] - '0');
                                                                   itwoone++;
                                                               }

                                                     while (segments[a-1][3][ithreeone] != '\0') {
                                                                   // ���ַ�ת��Ϊ��Ӧ�����ֲ��ۼӵ������
                                                                   numberthree3 = numberthree3 * 10 + (segments[a-1][3][ithreeone] - '0');
                                                                   ithreeone++;
                                                               }

                                       //ws281x_setPixelRGB(1,numberone1,numbertwo2,numberthree3);
                                       ws281x_setPixelRGB(2,numberone1,numbertwo2,numberthree3);
                                   }


//                                   if(strcmp(segments[a-1][0], "1111") == 0)
//                                   {
////                                       ws281x_setPixelRGB(0,0,0,0);
////                                                                                                                 ws281x_setPixelRGB(1,0,0,0);
////                                                                                                                 ws281x_setPixelRGB(2,0,0,0);
//
//                                       while (segments[a-1][1][ioneone1] != '\0') {
//                                                      // ���ַ�ת��Ϊ��Ӧ�����ֲ��ۼӵ������
//                                                      numberone11 = numberone11 * 10 + (segments[a-1][1][ioneone1] - '0');
//                                                      ioneone1++;
//                                                  }
//
//                                               while (segments[a-1][2][itwoone2] != '\0') {
//                                                             // ���ַ�ת��Ϊ��Ӧ�����ֲ��ۼӵ������
//                                                             numbertwo22 = numbertwo22 * 10 + (segments[a-1][2][itwoone2] - '0');
//                                                             itwoone2++;
//                                                         }
//
//                                               while (segments[a-1][3][ithreeone3] != '\0') {
//                                                             // ���ַ�ת��Ϊ��Ӧ�����ֲ��ۼӵ������
//                                                             numberthree33 = numberthree33 * 10 + (segments[a-1][3][ithreeone3] - '0');
//                                                             ithreeone3++;
//                                                         }
//
//                                       ws281x_setPixelRGB(0,numberone11,numbertwo22,numberthree33);
//                                       ws281x_setPixelRGB(1,numberone11,numbertwo22,numberthree33);
//                                       ws281x_setPixelRGB(2,numberone11,numbertwo22,numberthree33);
//                                                                          }





    }

    ws281x_show();
   // ws281x_closeAll();
//    ws281x_setPixelRGB(0,0,0,0);
//                                                                              ws281x_setPixelRGB(1,0,0,0);
//                                                                              ws281x_setPixelRGB(2,0,0,0);
//numberone=0;
//numbertwo=0;
//numberthree=0;
//numberone1=0;
//numbertwo2=0;
//numberthree3=0;
//numberone11=0;
//numbertwo22=0;
//numberthree33=0;
}

