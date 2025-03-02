/**
 ******************************************************************************
 * @file	: LimbsSftyLnFSwtch_Example_05.cpp
 * @brief  : Implements of a Limbs Safety switch using a LimbsSftyLnFSwtch class object
 * 
 * Example for the LimbsSftySw_ESP32 library LimbsSftyLnFSwtch class.
 * 
 * This example builds a Limbs Safety switch for a Launch and Forget device by
 * using the most basic resources provided by the LimbsSftyLnFSwtch class. 
 * 
 * This example uses the ShiftRegGPIOXpander library to manage the output pins
 * connected to several devices:
 * - Visual hints for the left hand switch status
 * - Visual hints for the right hand switch status
 * - Visual hints for the foot switch status
 * 
 * The example creates ONE Task in the standard Arduino setup(), and makes the 
 * LoopTask to delete itself.
 * The task created as the Main Control Task will then proceed with the execution:
 * - Create SEVERAL Task to manage the output resources updates for the underlying DbncdMPBtt subclasses objects
 *    - A task to manage the left hand switch outputs hardware configuration and updates
 *    - A task to manage the right hand switch outputs hardware configuration and updates
 *    - A task to manage the foot switch outputs hardware configuration and updates
 * - Create ONE Task to manage the LimbsSftyLnFSwtch object's outputs' hardware configuration and updates
 * - LimbsSftyLnFSwtch **object instantiation** and timer activation (using the .begin() method)
 *
 * Framework: Arduino
 * Platform: ESP32
 * 
 * @author	: Gabriel D. Goldman
 *
 * @date First release: 28/01/2025 
 *       Last update:   03/03/2025 00:10 GMT+0200
 ******************************************************************************
 * @attention	This library gives no guarantees whatsoever about it's compliance
 * to any expectations but as those from it's own designers. Use under your own 
 * responsibility and risk.
 * 
 * Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
 ******************************************************************************
*/
#include <Arduino.h>
#include <LimbsSafetySw_ESP32.h>
#include <ShiftRegGPIOXpander.h>

//==============================================>> General use definitions BEGIN
// This definitions are placed here for easier testing of important implementation parameters, checking the change of behavior when playing around with different values, etc.
#define LoopDlyTtlTm 25 // Time between task unblocking, time taken from the start of the task execution to the next execution 
#define MainCtrlTskPrrtyLvl 5 // Task priority level
#define UndrlyngMPBttnPollTm 25  // Timer setup time for the underlying MPBttns state update
#define LsSwtchPollTm 50   // Timer setup time for the LsSwitch state update
//================================================>> General use definitions END

//======================================>> General use function prototypes BEGIN
void Error_Handler();
//========================================>> General use function prototypes END

//====================================>> Task Callback function prototypes BEGIN
void mainCtrlTsk(void *pvParameters);
void leftHandCtrlTsk(void *pvParameters);
void rghtHandCtrlTsk(void *pvParameters);
void footCtrlTsk(void *pvParameters);
void lmbSftyCtrlTsk(void *pvParameters);
//======================================>> Task Callback function prototypes END

//===========================================>> Tasks Handles declarations BEGIN
TaskHandle_t mainCtrlTskHndl {NULL};
TaskHandle_t lftHndSwtchTskHndl{NULL};
TaskHandle_t rghtHndSwtchTskHndl{NULL};
TaskHandle_t ftSwtchTskHndl{NULL};
TaskHandle_t lmbSftySwtchTskHndl{NULL};
//=============================================>> Tasks Handles declarations END

//===============================>> Global variables (strictly sanctioned) BEGIN
ShiftRegGPIOXpander* srgxPtr{nullptr};
uint8_t mxVldXpndrPnNm{0};
//=================================>> Global variables (strictly sanctioned) END
 
limbSftyFwConf_t mnCtrlTskConf{
    .lsSwExecTskCore = xPortGetCoreID(),
    .lsSwExecTskPrrtyCnfg = MainCtrlTskPrrtyLvl, //! Arbitrary Task priority selected, chosen to be lower than the software timer update.
 };
  
void setup() { 
   // Create the Main control task for setup and execution of the main code
   xReturned = xTaskCreatePinnedToCore(
      mainCtrlTsk,  // Callback function/task to be called
      "MainControlTask",  // Name of the task
      2048,   // Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  // Pointer to the parameters for the function to work with
      mnCtrlTskConf.lsSwExecTskPrrtyCnfg, // Priority level given to the task
      &mainCtrlTskHndl, // Task handle
      mnCtrlTskConf.lsSwExecTskCore // Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
   );
   if(xReturned != pdPASS)
      Error_Handler();
}
  
void loop() {
   vTaskDelete(NULL); // Delete this task -the ESP-Arduino LoopTask()- and remove it from the execution list
}  
  
//===============================>> User Tasks Implementations BEGIN
void mainCtrlTsk(void *pvParameters){
   delay(10);  //FTPO Part of the WOKWI simulator suggestions for simulation needs
 
   TickType_t loopTmrStrtTm{0};
   TickType_t* loopTmrStrtTmPtr{&loopTmrStrtTm};
   TickType_t totalDelay {LoopDlyTtlTm};
  
   // Create the Left Hand Switch output update control task
   xReturned = xTaskCreatePinnedToCore(
      leftHandCtrlTsk,  // Callback function/task to be called
      "LeftHandControlTask",  // Name of the task
      1716,   // Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  // Pointer to the parameters for the function to work with
      mnCtrlTskConf.lsSwExecTskPrrtyCnfg, // Priority level given to the task
      &lftHndSwtchTskHndl, // Task handle
      mnCtrlTskConf.lsSwExecTskCore // Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
   );
   if(xReturned != pdPASS)
      Error_Handler();
  
   // Create the Right Hand Switch output update control task
   xReturned = xTaskCreatePinnedToCore(
      rghtHandCtrlTsk,  // Callback function/task to be called
      "RightHandControlTask",  // Name of the task
      1716,   // Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  // Pointer to the parameters for the function to work with
      mnCtrlTskConf.lsSwExecTskPrrtyCnfg, // Priority level given to the task
      &rghtHndSwtchTskHndl, // Task handle
      mnCtrlTskConf.lsSwExecTskCore // Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
   );
   if(xReturned != pdPASS)
      Error_Handler();
  
   // Create the Foot Switch output update control task
   xReturned = xTaskCreatePinnedToCore(
      footCtrlTsk,  // Callback function/task to be called
      "FootControlTask",  // Name of the task
      1716,   // Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  // Pointer to the parameters for the function to work with
      mnCtrlTskConf.lsSwExecTskPrrtyCnfg, // Priority level given to the task
      &ftSwtchTskHndl, // Task handle
      mnCtrlTskConf.lsSwExecTskCore // Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
   );
   if(xReturned != pdPASS)
      Error_Handler();
  
   // Create the Limb Safety Switch output update control task
   xReturned = xTaskCreatePinnedToCore(
      lmbSftyCtrlTsk,  // Callback function/task to be called
      "LimbSafetyControlTask",  // Name of the task
      1716,   // Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  // Pointer to the parameters for the function to work with
      mnCtrlTskConf.lsSwExecTskPrrtyCnfg, // Priority level given to the task
      &lmbSftySwtchTskHndl, // Task handle
      mnCtrlTskConf.lsSwExecTskCore // Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
   );
   if(xReturned != pdPASS)
      Error_Handler();
     
//=============================>> Underlying switches configuration parameters values BEGIN
   //----------------------------->> Hardware construction related parameter values BEGIN
   swtchInptHwCfg_t lftHndHwAttrbts{   // Left hand switch input hardware attributes
      .inptPin = GPIO_NUM_15,
      .typeNO = true,
      .pulledUp = true,
      .dbncTime = 0UL,
   };
   swtchInptHwCfg_t rghtHndHwAttrbts{  // Right hand switch input hardware attributes
      .inptPin = GPIO_NUM_2,
      .typeNO = true,
      .pulledUp = true,
      .dbncTime = 0UL,
   };
   swtchInptHwCfg_t ftHwAttrbts{ // Foot switch input hardware attributes
      .inptPin = GPIO_NUM_0,
      .typeNO = true,
      .pulledUp = true,
      .dbncTime = 0UL
   };
 
   uint8_t ds{33};   // Pin connected to ShiftRegGPIOXpander DS line
   uint8_t sh_cp{26};   // Pin connected to ShiftRegGPIOXpander SH_CP line
   uint8_t st_cp{25};   // Pin connected to ShiftRegGPIOXpander ST_CP line
   uint8_t srQty{1}; // Quantity of shift registers connected for the GPIOXpander
//------------------------------->> Hardware construction related parameters values END
    
//------------------------------------------>> Behavior related parameters values BEGIN
   //FFDR This settings are to be retrieved from a configuration setup in EEPROM BEGIN    
 
   swtchInptHwCfg_t enableLftHnd{
      .inptPin = 35,
      .typeNO = true,
      .pulledUp = false,
   };
   pinMode(enableLftHnd.inptPin, INPUT);
 
   swtchInptHwCfg_t enableRghtHnd{
      .inptPin = 34,
      .typeNO = true,
      .pulledUp = false,
   };
   pinMode(enableRghtHnd.inptPin, INPUT);
 
   swtchBhvrCfg_t lftHndBhvrSUp{ // Left hand switch behavior configuration properties
      .swtchStrtDlyTm = 100,
      .swtchIsEnbld = (digitalRead(enableLftHnd.inptPin) == HIGH)?true:false,   //FFDR This is the value to swap to have the object keep track of the left hand MPB or not
      .swtchVdTm = 3000,
   };
   swtchBhvrCfg_t rghtHndBhvrSUp{   // Right hand switch behavior configuration properties
      .swtchStrtDlyTm = 100,
      .swtchIsEnbld = (digitalRead(enableRghtHnd.inptPin) == HIGH)?true:false,   //FFDR This is the value to swap to have the object keep track of the left hand MPB or not
      .swtchVdTm = 3000,
   };
   swtchBhvrCfg_t ftBhvrSUp{  // Foot switch behavior configuration properties
      .swtchStrtDlyTm = 200,
      .swtchIsEnbld = false,
   };
   //FFDR This settings are to be retrieved from a configuration setup in EEPROM BEGIN
//-------------------------------------------->> Behavior related parameters values END
//===============================>> Underlying switches configuration parameters values END
 
//====================>> LimbsSftyLnFSwtch switch configuration parameters values BEGIN
//------------------------------------------>> Behavior related parameters values BEGIN
   lsSwtchSwCfg_t lsssSwtchWrkngPrm{
      .ltchRlsActvTm = 1500,
      .prdCyclActvTm = 4000,
   };
//-------------------------------------------->> Behavior related parameters values END
//=======================> LimbsSftyLnFSwtch switch configuration parameters values END
   
   ShiftRegGPIOXpander srgx (ds, sh_cp, st_cp, srQty, nullptr);
   srgxPtr = &srgx;    
   mxVldXpndrPnNm = srgxPtr->getMaxPin();

   LimbsSftyLnFSwtch stampSftySwtch (lftHndHwAttrbts, lftHndBhvrSUp, rghtHndHwAttrbts, rghtHndBhvrSUp, ftHwAttrbts, ftBhvrSUp, lsssSwtchWrkngPrm);
   
   stampSftySwtch.getLftHndSwtchPtr()->setTaskToNotify(lftHndSwtchTskHndl);
   stampSftySwtch.getRghtHndSwtchPtr()->setTaskToNotify(rghtHndSwtchTskHndl);
   stampSftySwtch.getFtSwtchPtr()->setTaskToNotify(ftSwtchTskHndl);
   stampSftySwtch.setTskToNtfyLsSwtchOtptsChng(lmbSftySwtchTskHndl);

   stampSftySwtch.setUndrlSwtchsPollDelay(UndrlyngMPBttnPollTm);
   stampSftySwtch.begin(LsSwtchPollTm);
  
   for(;;){
      *loopTmrStrtTmPtr = xTaskGetTickCount() / portTICK_RATE_MS; //! Although this is just a test execution, this is implemented to save resources by not executing this loop constantly

      vTaskDelayUntil(loopTmrStrtTmPtr, totalDelay);  //! Complementary code for the implementation to save resources by blocking this piece of code as is not needed to be executed constantly   
   }
}
  
void leftHandCtrlTsk(void *pvParameters){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   delay(10);  //FTPO Part of the WOKWI simulator suggestions for simulation needs
   swtchOtptHwCfg_t lftHndHwOtpts{  // Left hand switch hardware outputs configuration properties
      .isOnPin{
         .gpioOtptPin = 4,
         .gpioOtptActHgh = true,
      },
      .isVoidedPin{
         .gpioOtptPin = 5,
         .gpioOtptActHgh = true,
      },
      .isEnabledPin{
         .gpioOtptPin = 3,
         .gpioOtptActHgh = false,
      }   
   };
   uint32_t swtcSttsRcvd{0};
   MpbOtpts_t undrlyngSwtchStts;
  
   //------------------>> Left hand TmVdblMPBttn output pins initial values through ShiftRegGPIOXpander set BEGIN
   taskENTER_CRITICAL(&mux); 
   if((lftHndHwOtpts.isOnPin.gpioOtptPin != _InvalidPinNum) && (lftHndHwOtpts.isOnPin.gpioOtptPin <= mxVldXpndrPnNm)){
      srgxPtr->digitalWriteSrToAux(lftHndHwOtpts.isOnPin.gpioOtptPin, (lftHndHwOtpts.isOnPin.gpioOtptActHgh)?LOW:HIGH);
   }
   else if((lftHndHwOtpts.isOnPin.gpioOtptPin > mxVldXpndrPnNm)){
      Error_Handler();
   }
   if((lftHndHwOtpts.isEnabledPin.gpioOtptPin != _InvalidPinNum) && (lftHndHwOtpts.isEnabledPin.gpioOtptPin <= mxVldXpndrPnNm)){
      srgxPtr->digitalWriteSrToAux(lftHndHwOtpts.isEnabledPin.gpioOtptPin, (!lftHndHwOtpts.isEnabledPin.gpioOtptActHgh)?LOW:HIGH);
   }   
   else if((lftHndHwOtpts.isEnabledPin.gpioOtptPin > mxVldXpndrPnNm)){
      Error_Handler();
   }
   if((lftHndHwOtpts.isVoidedPin.gpioOtptPin != _InvalidPinNum) && (lftHndHwOtpts.isVoidedPin.gpioOtptPin  <= mxVldXpndrPnNm)){
      srgxPtr->digitalWriteSrToAux(lftHndHwOtpts.isVoidedPin.gpioOtptPin, (lftHndHwOtpts.isVoidedPin.gpioOtptActHgh)?LOW:HIGH);
   }
   else if((lftHndHwOtpts.isVoidedPin.gpioOtptPin > mxVldXpndrPnNm)){
      Error_Handler();
   }
   srgxPtr->moveAuxToMain(true);
   taskEXIT_CRITICAL(&mux); 
   //-------------------->> Left hand TmVdblMPBttn output pins initial values through ShiftRegGPIOXpander set END
 
   for(;;){
      xReturned = xTaskNotifyWait(
         0x00,	//uint32_t ulBitsToClearOnEntry
         0xFFFFFFFF,	//uint32_t ulBitsToClearOnExit,
         &swtcSttsRcvd,	// uint32_t *pulNotificationValue,
         portMAX_DELAY  //TickType_t xTicksToWait
      );
      if (xReturned != pdPASS)
         Error_Handler();
  
      undrlyngSwtchStts = otptsSttsUnpkg(swtcSttsRcvd);
  
      // Keep the _undrlLftHndMPB object outputs updated.		
      taskENTER_CRITICAL(&mux); 
      if(lftHndHwOtpts.isOnPin.gpioOtptPin != _InvalidPinNum){ // Keep the left hand switch isOn output updated
         if(srgxPtr->digitalReadSr(lftHndHwOtpts.isOnPin.gpioOtptPin) != ((undrlyngSwtchStts.isOn == lftHndHwOtpts.isOnPin.gpioOtptActHgh)?HIGH:LOW))
            srgxPtr->digitalWriteSrToAux(lftHndHwOtpts.isOnPin.gpioOtptPin, (undrlyngSwtchStts.isOn == lftHndHwOtpts.isOnPin.gpioOtptActHgh)?HIGH:LOW);
      }
      if(lftHndHwOtpts.isEnabledPin.gpioOtptPin != _InvalidPinNum){  // Keep the left hand switch isEnabled output updated
         if(srgxPtr->digitalReadSr(lftHndHwOtpts.isEnabledPin.gpioOtptPin) != ((undrlyngSwtchStts.isEnabled == lftHndHwOtpts.isEnabledPin.gpioOtptActHgh)?HIGH:LOW))
            srgxPtr->digitalWriteSrToAux(lftHndHwOtpts.isEnabledPin.gpioOtptPin, (undrlyngSwtchStts.isEnabled == lftHndHwOtpts.isEnabledPin.gpioOtptActHgh)?HIGH:LOW);
      }
      if(lftHndHwOtpts.isVoidedPin.gpioOtptPin != _InvalidPinNum){   // Keep the left hand switch isVoided output updated
         if(srgxPtr->digitalReadSr(lftHndHwOtpts.isVoidedPin.gpioOtptPin) != ((undrlyngSwtchStts.isVoided == lftHndHwOtpts.isVoidedPin.gpioOtptActHgh)?HIGH:LOW))
            srgxPtr->digitalWriteSrToAux(lftHndHwOtpts.isVoidedPin.gpioOtptPin, (undrlyngSwtchStts.isVoided == lftHndHwOtpts.isVoidedPin.gpioOtptActHgh)?HIGH:LOW);
      }
      srgxPtr->moveAuxToMain(true);
      taskEXIT_CRITICAL(&mux); 
   }
}
  
void rghtHandCtrlTsk(void *pvParameters){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   delay(10);  //FTPO Part of the WOKWI simulator suggestions for simulation needs
   swtchOtptHwCfg_t rghtHndHwOtpts{ // Right hand switch hardware outputs configuration properties
      .isOnPin{
         .gpioOtptPin = 1,
         .gpioOtptActHgh = true,
      },
      .isVoidedPin{
         .gpioOtptPin = 2,
         .gpioOtptActHgh = true,
      },
      .isEnabledPin{
         .gpioOtptPin = 0,
         .gpioOtptActHgh = false,
      }
   };
   uint32_t swtcSttsRcvd{0};
   MpbOtpts_t undrlyngSwtchStts;
  
   //----------------->> Right hand TmVdblMPBttn output initial values through ShiftRegGPIOXpander set BEGIN 
   taskENTER_CRITICAL(&mux); 
   if((rghtHndHwOtpts.isOnPin.gpioOtptPin != _InvalidPinNum) && (rghtHndHwOtpts.isOnPin.gpioOtptPin <= mxVldXpndrPnNm)){
      srgxPtr->digitalWriteSrToAux(rghtHndHwOtpts.isOnPin.gpioOtptPin, (rghtHndHwOtpts.isOnPin.gpioOtptActHgh)?LOW:HIGH);
   }
   else if((rghtHndHwOtpts.isOnPin.gpioOtptPin > mxVldXpndrPnNm)){
      Error_Handler();
   }
   if((rghtHndHwOtpts.isEnabledPin.gpioOtptPin != _InvalidPinNum) && (rghtHndHwOtpts.isEnabledPin.gpioOtptPin <= mxVldXpndrPnNm)){
      srgxPtr->digitalWriteSrToAux(rghtHndHwOtpts.isEnabledPin.gpioOtptPin, (!rghtHndHwOtpts.isEnabledPin.gpioOtptActHgh)?LOW:HIGH);
   }   
   else if((rghtHndHwOtpts.isEnabledPin.gpioOtptPin > mxVldXpndrPnNm)){
      Error_Handler();
   }
   if((rghtHndHwOtpts.isVoidedPin.gpioOtptPin != _InvalidPinNum) && (rghtHndHwOtpts.isVoidedPin.gpioOtptPin  <= mxVldXpndrPnNm)){
      srgxPtr->digitalWriteSrToAux(rghtHndHwOtpts.isVoidedPin.gpioOtptPin, (rghtHndHwOtpts.isVoidedPin.gpioOtptActHgh)?LOW:HIGH);
   }
   else if((rghtHndHwOtpts.isVoidedPin.gpioOtptPin > mxVldXpndrPnNm)){
      Error_Handler();
   }
   srgxPtr->moveAuxToMain(true);
   taskEXIT_CRITICAL(&mux); 
//------------------->> Right hand TmVdblMPBttn output initial values through ShiftRegGPIOXpander set END
 
   for(;;){
      xReturned = xTaskNotifyWait(
         0x00,	//uint32_t ulBitsToClearOnEntry
         0xFFFFFFFF,	//uint32_t ulBitsToClearOnExit,
         &swtcSttsRcvd,	// uint32_t *pulNotificationValue,
         portMAX_DELAY  //TickType_t xTicksToWait
      );
      if (xReturned != pdPASS)
         Error_Handler();
  
      undrlyngSwtchStts = otptsSttsUnpkg(swtcSttsRcvd);
  
      // Keep the _undrlLftHndMPB object outputs updated.		
      taskENTER_CRITICAL(&mux); 
      if(rghtHndHwOtpts.isOnPin.gpioOtptPin != _InvalidPinNum){ // Keep the left hand switch isOn output updated
         if(srgxPtr->digitalReadSr(rghtHndHwOtpts.isOnPin.gpioOtptPin) != ((undrlyngSwtchStts.isOn == rghtHndHwOtpts.isOnPin.gpioOtptActHgh)?HIGH:LOW))
            srgxPtr->digitalWriteSrToAux(rghtHndHwOtpts.isOnPin.gpioOtptPin, (undrlyngSwtchStts.isOn == rghtHndHwOtpts.isOnPin.gpioOtptActHgh)?HIGH:LOW);
      }
      if(rghtHndHwOtpts.isEnabledPin.gpioOtptPin != _InvalidPinNum){  // Keep the left hand switch isEnabled output updated
         if(srgxPtr->digitalReadSr(rghtHndHwOtpts.isEnabledPin.gpioOtptPin) != ((undrlyngSwtchStts.isEnabled == rghtHndHwOtpts.isEnabledPin.gpioOtptActHgh)?HIGH:LOW))
            srgxPtr->digitalWriteSrToAux(rghtHndHwOtpts.isEnabledPin.gpioOtptPin, (undrlyngSwtchStts.isEnabled == rghtHndHwOtpts.isEnabledPin.gpioOtptActHgh)?HIGH:LOW);
      }
      if(rghtHndHwOtpts.isVoidedPin.gpioOtptPin != _InvalidPinNum){   // Keep the left hand switch isVoided output updated
         if(srgxPtr->digitalReadSr(rghtHndHwOtpts.isVoidedPin.gpioOtptPin) != ((undrlyngSwtchStts.isVoided == rghtHndHwOtpts.isVoidedPin.gpioOtptActHgh)?HIGH:LOW))
            srgxPtr->digitalWriteSrToAux(rghtHndHwOtpts.isVoidedPin.gpioOtptPin, (undrlyngSwtchStts.isVoided == rghtHndHwOtpts.isVoidedPin.gpioOtptActHgh)?HIGH:LOW);
      }
      srgxPtr->moveAuxToMain(true);
      taskEXIT_CRITICAL(&mux); 
   }
}
  
void footCtrlTsk(void *pvParameters){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   delay(10);  //FTPO Part of the WOKWI simulator suggestions for simulation needs
   gpioPinOtptHwCfg_t ftSwtchIsEnbldOtpt{ // Foot switch hardware outputs configuration properties
    .gpioOtptPin = 6,
    .gpioOtptActHgh = true,
   };
   uint32_t swtcSttsRcvd{0};
   MpbOtpts_t undrlyngSwtchStts;
  
   //----------------->> Foot SnglSrvcVdblMPBttn output through ShiftRegGPIOXpander configuration BEGIN
   if((ftSwtchIsEnbldOtpt.gpioOtptPin != _InvalidPinNum) && (ftSwtchIsEnbldOtpt.gpioOtptPin <= mxVldXpndrPnNm)){
      srgxPtr->digitalWriteSr(ftSwtchIsEnbldOtpt.gpioOtptPin, (ftSwtchIsEnbldOtpt.gpioOtptActHgh)?LOW:HIGH);
   } 
   else if((ftSwtchIsEnbldOtpt.gpioOtptPin > mxVldXpndrPnNm)){
      Error_Handler();
   }
   //------------------->> Foot SnglSrvcVdblMPBttn output through ShiftRegGPIOXpander configuration END
 
   for(;;){
      xReturned = xTaskNotifyWait(
         0x00,	//uint32_t ulBitsToClearOnEntry
         0xFFFFFFFF,	//uint32_t ulBitsToClearOnExit,
         &swtcSttsRcvd,	// uint32_t *pulNotificationValue,
         portMAX_DELAY  //TickType_t xTicksToWait
      );
      if (xReturned != pdPASS)
         Error_Handler();

      undrlyngSwtchStts = otptsSttsUnpkg(swtcSttsRcvd);
 
      taskENTER_CRITICAL(&mux); 
      // Keep the _undrlFtdMPB object outputs updated.	      
      if(ftSwtchIsEnbldOtpt.gpioOtptPin != _InvalidPinNum){
         if(srgxPtr->digitalReadSr(ftSwtchIsEnbldOtpt.gpioOtptPin) != ((undrlyngSwtchStts.isEnabled == ftSwtchIsEnbldOtpt.gpioOtptActHgh)?HIGH:LOW))
            srgxPtr->digitalWriteSr(ftSwtchIsEnbldOtpt.gpioOtptPin, (undrlyngSwtchStts.isEnabled == ftSwtchIsEnbldOtpt.gpioOtptActHgh)?HIGH:LOW);
      }
      taskEXIT_CRITICAL(&mux); 
   }
}
  
void lmbSftyCtrlTsk(void *pvParameters){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   delay(10);  //FTPO Part of the WOKWI simulator suggestions for simulation needs
   gpioPinOtptHwCfg_t prdCyclIsOnOtpt{   // Production cycle hardware outputs configuration properties
      .gpioOtptPin = 7,
      .gpioOtptActHgh = true,
   };
   gpioPinOtptHwCfg_t ltchRlsIsOnOtpt{   // Latch Release hardware outputs configuration properties
      .gpioOtptPin = GPIO_NUM_27,
      .gpioOtptActHgh = true,
   };
   uint32_t swtcSttsRcvd{0};
   lsSwtchOtpts_t lsSwtchStts;
  
   //----------------------->> LimbsSftyLnFSwtch output pins configuration BEGIN
   //! The ltchRlsIsOnOtpt is the ONLY FUNDAMENTAL pin that is REQUIRED to be a valid available addressable pin number. 
   //! All the of the rest of the output pins are for information provision, and might be assigned to a _InvalidPinNum to be ignored.
   if((prdCyclIsOnOtpt.gpioOtptPin != _InvalidPinNum) && (prdCyclIsOnOtpt.gpioOtptPin <= mxVldXpndrPnNm)){
      srgxPtr->digitalWriteSr(prdCyclIsOnOtpt.gpioOtptPin, (prdCyclIsOnOtpt.gpioOtptActHgh)?LOW:HIGH);
   }
   else if((prdCyclIsOnOtpt.gpioOtptPin > mxVldXpndrPnNm)){
      Error_Handler();
   }
   if((ltchRlsIsOnOtpt.gpioOtptPin != _InvalidPinNum) && (ltchRlsIsOnOtpt.gpioOtptPin <= _maxValidPinNum)){
      pinMode(ltchRlsIsOnOtpt.gpioOtptPin, INPUT);
      digitalWrite(ltchRlsIsOnOtpt.gpioOtptPin, (ltchRlsIsOnOtpt.gpioOtptActHgh)?LOW:HIGH);
   }
   else{
      Error_Handler();
   }
   pinMode(ltchRlsIsOnOtpt.gpioOtptPin, OUTPUT); // Setting the main activation output pin to OUTPUT mode
   //------------------------->> LimbsSftyLnFSwtch output pins configuration END
    for(;;){
      xReturned = xTaskNotifyWait(
         0x00,	//uint32_t ulBitsToClearOnEntry
         0xFFFFFFFF,	//uint32_t ulBitsToClearOnExit,
         &swtcSttsRcvd,	// uint32_t *pulNotificationValue,
         portMAX_DELAY  //TickType_t xTicksToWait
      );
      if (xReturned != pdPASS)
         Error_Handler();
  
      lsSwtchStts = lssOtptsSttsUnpkg(swtcSttsRcvd);
  
      if(digitalRead(ltchRlsIsOnOtpt.gpioOtptPin) != ((lsSwtchStts.ltchRlsIsOn == ltchRlsIsOnOtpt.gpioOtptActHgh)?HIGH:LOW))
         digitalWrite(ltchRlsIsOnOtpt.gpioOtptPin, (lsSwtchStts.ltchRlsIsOn == ltchRlsIsOnOtpt.gpioOtptActHgh)?HIGH:LOW);
  
      if(prdCyclIsOnOtpt.gpioOtptPin != _InvalidPinNum){
         if(srgxPtr->digitalReadSr(prdCyclIsOnOtpt.gpioOtptPin) != ((lsSwtchStts.prdCyclIsOn == prdCyclIsOnOtpt.gpioOtptActHgh)?HIGH:LOW))
            srgxPtr->digitalWriteSr(prdCyclIsOnOtpt.gpioOtptPin, (lsSwtchStts.prdCyclIsOn == prdCyclIsOnOtpt.gpioOtptActHgh)?HIGH:LOW);
      }
   }
}
  //===============================>> User Tasks Implementations END
  
  //===============================>> User Functions Implementations BEGIN
  /**
   * @brief Error Handling function
   * 
   * Placeholder for a Error Handling function, in case of an error the execution
   * will be trapped in this endless loop
   */
  void Error_Handler(){
    for(;;)
    {    
    }
    
    return;
  }
  //===============================>> User Functions Implementations END