#include <stm32l4xx_hal_can.h>
#include <stm32l4xx_hal_rcc.h>
#include <stm32l4xx_hal_gpio.h>
#include <stm32l4xx_hal_cortex.h>

//Overwrite the weak default IRQ Handlers and callabcks
extern "C" void CAN1_RX0_IRQHandler(void);
extern "C" void CAN1_TX_IRQHandler(void);

//Pointer to user ISRS
void (*CAN_RX_ISR)() = NULL;
void (*CAN_TX_ISR)() = NULL;

//CAN handle struct with initialisation parameters
//Timing from http://www.bittiming.can-wiki.info/ with bit rate = 125kHz and clock frequency = 80MHz
CAN_HandleTypeDef CAN_Handle = {
    CAN1,
    {
        40,           //Prescaler
        CAN_MODE_NORMAL,     //Normal/loopback/silent mode
        CAN_SJW_2TQ,  //SyncJumpWidth
        CAN_BS1_13TQ, //TimeSeg1
        CAN_BS2_2TQ,  //TimeSeg2
        DISABLE,      //TimeTriggeredMode
        DISABLE,      //AutoBusOff
        ENABLE,       //AutoWakeUp
        ENABLE,       //AutoRetransmission
        DISABLE,      //ReceiveFifoLocked
        ENABLE        //TransmitFifoPriority
    },
    HAL_CAN_STATE_RESET,  //State
    HAL_CAN_ERROR_NONE    //Error Code
};


//Initialise CAN dependencies: GPIO and clock
void HAL_CAN_MspInit(CAN_HandleTypeDef* CAN_Handle) {

  //Set up the pin initialisation
  GPIO_InitTypeDef GPIO_InitCAN_TX = {
    GPIO_PIN_12,            //PA12 is CAN TX
    GPIO_MODE_AF_PP,        //Alternate function, push-pull driver
    GPIO_NOPULL,            //No pull-up
    GPIO_SPEED_FREQ_MEDIUM, //Medium slew rate
    GPIO_AF9_CAN1           //Alternate function is CAN
    };

  GPIO_InitTypeDef GPIO_InitCAN_RX = {
    GPIO_PIN_11,            //PA11 is CAN RX
    GPIO_MODE_AF_PP,        //Alternate function, push-pull driver
    GPIO_PULLUP,            //Pull-up enabled
    GPIO_SPEED_FREQ_MEDIUM, //Medium slew rate
    GPIO_AF9_CAN1           //Alternate function is CAN    
    };

  //Enable the CAN and GPIO clocks
  __HAL_RCC_CAN1_CLK_ENABLE(); //Enable the CAN interface clock
  __HAL_RCC_GPIOA_CLK_ENABLE(); //Enable the clock for the CAN GPIOs

  //Initialise the pins
  HAL_GPIO_Init(GPIOA, &GPIO_InitCAN_TX); //Configure CAN pin
  HAL_GPIO_Init(GPIOA, &GPIO_InitCAN_RX); //Configure CAN pin
}


uint32_t CAN_Init(bool loopback=false) {
  if (loopback)
    CAN_Handle.Init.Mode = CAN_MODE_LOOPBACK;
  return (uint32_t) HAL_CAN_Init(&CAN_Handle);
}


uint32_t setCANFilter(uint32_t filterID, uint32_t maskID, uint32_t filterBank) {

  //Set up the filter definition
  CAN_FilterTypeDef filterInfo = {
    (filterID << 5) & 0xffe0,   //Filter ID
    0,                          //Filter ID LSBs = 0
    (maskID << 5) & 0xffe0,     //Mask MSBs
    0,                          //Mask LSBs = 0
    0,                          //FIFO selection
    filterBank & 0xf,           //Filter bank selection
    CAN_FILTERMODE_IDMASK,      //Mask mode
    CAN_FILTERSCALE_32BIT,      //32 bit IDs
    CAN_FILTER_ENABLE,          //Enable filter
    0                           //uint32_t SlaveStartFilterBank
  };

  return (uint32_t) HAL_CAN_ConfigFilter(&CAN_Handle, &filterInfo);
}


uint32_t CAN_Start() {
  return (uint32_t) HAL_CAN_Start(&CAN_Handle);
}


uint32_t CAN_TX(uint32_t ID, uint8_t data[8]) {

  //Set up the message header
  CAN_TxHeaderTypeDef txHeader = {
    ID & 0x7ff,                 //Standard ID
    0,                          //Ext ID = 0
    CAN_ID_STD,                 //Use Standard ID
    CAN_RTR_DATA,               //Data Frame
    8,                          //Send 8 bytes
    DISABLE                     //No time triggered mode
  };

  //Wait for free mailbox
  while (!HAL_CAN_GetTxMailboxesFreeLevel(&CAN_Handle));

  //Start the transmission
  return (uint32_t) HAL_CAN_AddTxMessage(&CAN_Handle, &txHeader, data, NULL);
}


uint32_t CAN_CheckRXLevel() {
  return HAL_CAN_GetRxFifoFillLevel(&CAN_Handle, 0);
}


uint32_t CAN_RX(uint32_t &ID, uint8_t data[8]) {
  CAN_RxHeaderTypeDef rxHeader;

  //Wait for message in FIFO
  while (!HAL_CAN_GetRxFifoFillLevel(&CAN_Handle, 0));
  
  //Get the message from the FIFO
  uint32_t result = (uint32_t) HAL_CAN_GetRxMessage(&CAN_Handle, 0, &rxHeader, data);

  //Store the ID from the header
  ID = rxHeader.StdId;

  return result;
}


uint32_t CAN_RegisterRX_ISR(void(& callback)()) {
  //Store pointer to user ISR
  CAN_RX_ISR = &callback;

  //Enable message received interrupt in HAL
  uint32_t status = (uint32_t) HAL_CAN_ActivateNotification (&CAN_Handle, CAN_IT_RX_FIFO0_MSG_PENDING);

  //Switch on the interrupt
  HAL_NVIC_SetPriority (CAN1_RX0_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ (CAN1_RX0_IRQn);

  return status;
}


uint32_t CAN_RegisterTX_ISR(void(& callback)()) {
  //Store pointer to user ISR
  CAN_TX_ISR = &callback;

  //Enable message received interrupt in HAL
  uint32_t status = (uint32_t) HAL_CAN_ActivateNotification (&CAN_Handle, CAN_IT_TX_MAILBOX_EMPTY);

  //Switch on the interrupt
  HAL_NVIC_SetPriority (CAN1_TX_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ (CAN1_TX_IRQn);

  return status;
}


void HAL_CAN_RxFifo0MsgPendingCallback (CAN_HandleTypeDef * hcan){

  //Call the user ISR if it has been registered
  if (CAN_RX_ISR)
    CAN_RX_ISR();
}


void HAL_CAN_TxMailbox0CompleteCallback (CAN_HandleTypeDef * hcan){

  //Call the user ISR if it has been registered
  if (CAN_TX_ISR)
    CAN_TX_ISR();
}


void HAL_CAN_TxMailbox1CompleteCallback (CAN_HandleTypeDef * hcan){

  //Call the user ISR if it has been registered
  if (CAN_TX_ISR)
    CAN_TX_ISR();
}


void HAL_CAN_TxMailbox2CompleteCallback (CAN_HandleTypeDef * hcan){

  //Call the user ISR if it has been registered
  if (CAN_TX_ISR)
    CAN_TX_ISR();
}


//This is the base ISR at the interrupt vector
void CAN1_RX0_IRQHandler(void){

  //Use the HAL interrupt handler
  HAL_CAN_IRQHandler(&CAN_Handle);
}


//This is the base ISR at the interrupt vector
void CAN1_TX_IRQHandler(void){

  //Use the HAL interrupt handler
  HAL_CAN_IRQHandler(&CAN_Handle);
}
