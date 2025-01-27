/**
  ******************************************************************************
  * @file	: LimbsSftyLnFSwtch_Example_01.cpp
  * @brief  : Implements of a Limbs Safety switch using a LimbsSftyLnFSwtch class object
  * 
  * Example for the LimbsSftySw_ESP32 library LimbsSftyLnFSwtch class.
  * 
  * This example builds a Limbs Safety switch for a Launch and Forget device by
  * using the most basic resources provided by the LimbsSftyLnFSwtch class. 
  * The example creates ONE Task in the standard Arduino setup(), and makes the 
  * LoopTask to delete itself.
  * The task created as the Main Control Task will then proceed with the execution:
  * - Underlying switches' input pins configuration parameters setting
  * - Underlying switches' output pins configuration parameters setting
  * - Underlying switches' behavior configuration parameters setting
  * - LimbsSftyLnFSwtch object outputs' configuration parameters setting
  * - LimbsSftyLnFSwtch object behavior configuration parameters setting
  * - All previously configuration parameters set application by pin configurations
  * - LimbsSftyLnFSwtch object instantiation and timer activation using .begin() method
  * - Endless loop execution updating inputs and outputs
  *
  * Framework: Arduino
  * Platform: ESP32
  * 
  * @author	: Gabriel D. Goldman
  *
  * @date First release: 28/11/2024 
  *       Last update:   26/01/2025 11:20 GMT+0200
  ******************************************************************************
  * @attention	This library gives no guarantees whatsoever about it's compliance
  * to any expectations but as those from it's own designers. Use under your own 
  * responsability and risk.
  * 
  * Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
  ******************************************************************************
  */
#include <Arduino.h>
#include <LimbsSafetySw_ESP32.h>

//==============================================>> General use definitions BEGIN
//* This definitios are placed here for easier testing of important implementation parameters, checking the change of behavior when playing around with different values, etc.
#define LoopDlyTtlTm 25
#define MainCtrlTskPrrtyLvl 5
#define UndrlyngMPBttnPollTm 25
#define LsSwtchPollTm 50
//================================================>> General use definitions END

//======================================>> General use function prototypes BEGIN
void Error_Handler();
//========================================>> General use function prototypes END

//======================================>> Task Callback function prototypes BEGIN
void mainCtrlTsk(void *pvParameters);
//========================================>> Task Callback function prototypes END

//===========================>> Tasks Handles declarations BEGIN
TaskHandle_t mainCtrlTskHndl {NULL};
//===========================>> Tasks Handles declarations END

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
   TickType_t loopTmrStrtTm{0};
   TickType_t* loopTmrStrtTmPtr{&loopTmrStrtTm};
   TickType_t totalDelay {LoopDlyTtlTm};

   //=============================>> Underlying switches configuration parameters values BEGIN
   //----------------------------->> Hardware construction related parameter values BEGIN
   swtchInptHwCfg_t lftHndHwAttrbts{   // Left hand switch hardware attributes
      .inptPin = GPIO_NUM_4,
      .typeNO = true,
      .pulledUp = true,
      .dbncTime = 0UL,
   };
   swtchInptHwCfg_t rghtHndHwAttrbts{  // Right hand switch hardware attributes
      .inptPin = GPIO_NUM_2,
      .typeNO = true,
      .pulledUp = true,
      .dbncTime = 0UL,
   };
   swtchInptHwCfg_t ftHwAttrbts{ // Foot switch hardware attributes
      .inptPin = GPIO_NUM_5,
      .typeNO = true,
      .pulledUp = true,
      .dbncTime = 0UL
   };

   swtchOtptHwCfg_t lftHndHwOtpts{  // Left hand switch attribute flags outputs properties
      .isOnPin{
         .gpioOtptPin = GPIO_NUM_27,
         .gpioOtptActHgh = true,
      },
      .isVoidedPin{
         .gpioOtptPin = GPIO_NUM_14,
         .gpioOtptActHgh = true,
      },
      .isEnabledPin{
         .gpioOtptPin = GPIO_NUM_26,
         .gpioOtptActHgh = false,
      }   
   };
   swtchOtptHwCfg_t rghtHndHwOtpts{ // Right hand switch attribute flags outputs properties
      .isOnPin{
         .gpioOtptPin = GPIO_NUM_33,
         .gpioOtptActHgh = true,
      },
      .isVoidedPin{
         .gpioOtptPin = GPIO_NUM_25,
         .gpioOtptActHgh = true,
      },
      .isEnabledPin{
         .gpioOtptPin = GPIO_NUM_32,
         .gpioOtptActHgh = false,
      }
   };
   gpioPinOtptHwCfg_t ftSwtchIsEnbldOtpt{ // Foot switch attribute flags outputs properties
      .gpioOtptPin = GPIO_NUM_17,
      .gpioOtptActHgh = true,
   };
   //------------------------------->> Hardware construction related parameters values END
   //------------------------------------------>> Behavior related parameters values BEGIN
   swtchBhvrCfg_t lftHndBhvrSUp{ // Left hand switch behavior configuration properties
      .swtchStrtDlyTm = 100,
      .swtchIsEnbld = true,   //FTPO This is the value to swap to have the object keep track of the left hand MPB or not
      .swtchVdTm = 5000,
   };
   swtchBhvrCfg_t rghtHndBhvrSUp{   // Right hand switch behavior configuration properties
      .swtchStrtDlyTm = 100,
      .swtchIsEnbld = true,   //FTPO This is the value to swap to have the object keep track of the right hand MPB or not
      .swtchVdTm = 5000,
   };
   swtchBhvrCfg_t ftBhvrSUp{  // Foot switch behavior configuration properties
      .swtchStrtDlyTm = 200,
      .swtchIsEnbld = false,
   };
   //-------------------------------------------->> Behavior related parameters values END
   //===============================>> Underlying switches configuration parameters values END

   //=============================>> LimbsSftyLnFSwtch switch configuration parameters values BEGIN
   //----------------------------->> Hardware construction related parameters values BEGIN
   gpioPinOtptHwCfg_t prdCyclIsOnOtpt{   // Production cycle output pin hardware attributes
      .gpioOtptPin = GPIO_NUM_22,
      .gpioOtptActHgh = true,
      };
   gpioPinOtptHwCfg_t ltchRlsIsOnOtpt{   // Latch Release output pin hardware attributes
      .gpioOtptPin = GPIO_NUM_23,
      .gpioOtptActHgh = true,
      };
   //------------------------------->> Hardware construction related parameters values END
   //------------------------------------------>> Behavior related parameters values BEGIN
   lsSwtchSwCfg_t lsssSwtchWrkngPrm{
      .ltchRlsActvTm = 1500,
      .prdCyclActvTm = 6000,
   };
   //-------------------------------------------->> Behavior related parameters values END
   //===============================>> LimbsSftyLnFSwtch switch configuration parameters values END
   
   //===========================>> LimbsSftyLnFSwtch Outputs configuration BEGIN
   //------------------>> Left hand TmVdblMPBttn output pins configuration BEGIN
   if((lftHndHwOtpts.isOnPin.gpioOtptPin != _InvalidPinNum) && (lftHndHwOtpts.isOnPin.gpioOtptPin <= _maxValidPinNum)){
      pinMode(lftHndHwOtpts.isOnPin.gpioOtptPin, INPUT);
      digitalWrite(lftHndHwOtpts.isOnPin.gpioOtptPin, (lftHndHwOtpts.isOnPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(lftHndHwOtpts.isOnPin.gpioOtptPin, OUTPUT);
   }
   else if((lftHndHwOtpts.isOnPin.gpioOtptPin > _maxValidPinNum)){
      Error_Handler();
   }
   if((lftHndHwOtpts.isEnabledPin.gpioOtptPin != _InvalidPinNum) && (lftHndHwOtpts.isEnabledPin.gpioOtptPin <= _maxValidPinNum)){
      pinMode(lftHndHwOtpts.isEnabledPin.gpioOtptPin, INPUT);
      digitalWrite(lftHndHwOtpts.isEnabledPin.gpioOtptPin, (!lftHndHwOtpts.isEnabledPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(lftHndHwOtpts.isEnabledPin.gpioOtptPin, OUTPUT);
   }   
   else if((lftHndHwOtpts.isEnabledPin.gpioOtptPin > _maxValidPinNum)){
      Error_Handler();
   }
   if((lftHndHwOtpts.isVoidedPin.gpioOtptPin != _InvalidPinNum) && (lftHndHwOtpts.isVoidedPin.gpioOtptPin  <= _maxValidPinNum)){
      pinMode(lftHndHwOtpts.isVoidedPin.gpioOtptPin, INPUT);
      digitalWrite(lftHndHwOtpts.isVoidedPin.gpioOtptPin, (lftHndHwOtpts.isVoidedPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(lftHndHwOtpts.isVoidedPin.gpioOtptPin, OUTPUT);
   }
   else if((lftHndHwOtpts.isVoidedPin.gpioOtptPin > _maxValidPinNum)){
      Error_Handler();
   }
   //-------------------->> Left hand TmVdblMPBttn output pins configuration END

   //----------------->> Right hand TmVdblMPBttn output pins configuration BEGIN 
   if((rghtHndHwOtpts.isOnPin.gpioOtptPin != _InvalidPinNum) && (rghtHndHwOtpts.isOnPin.gpioOtptPin <= _maxValidPinNum)){
      pinMode(rghtHndHwOtpts.isOnPin.gpioOtptPin, INPUT);
      digitalWrite(rghtHndHwOtpts.isOnPin.gpioOtptPin, (rghtHndHwOtpts.isOnPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(rghtHndHwOtpts.isOnPin.gpioOtptPin, OUTPUT);
   }
   else if((rghtHndHwOtpts.isOnPin.gpioOtptPin > _maxValidPinNum)){
      Error_Handler();
   }
   if((rghtHndHwOtpts.isEnabledPin.gpioOtptPin != _InvalidPinNum) && (rghtHndHwOtpts.isEnabledPin.gpioOtptPin <= _maxValidPinNum)){
      pinMode(rghtHndHwOtpts.isEnabledPin.gpioOtptPin, INPUT);
      digitalWrite(rghtHndHwOtpts.isEnabledPin.gpioOtptPin, (!rghtHndHwOtpts.isEnabledPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(rghtHndHwOtpts.isEnabledPin.gpioOtptPin, OUTPUT);
   }   
   else if((rghtHndHwOtpts.isEnabledPin.gpioOtptPin > _maxValidPinNum)){
      Error_Handler();
   }
   if((rghtHndHwOtpts.isVoidedPin.gpioOtptPin != _InvalidPinNum) && (rghtHndHwOtpts.isVoidedPin.gpioOtptPin  <= _maxValidPinNum)){
      pinMode(rghtHndHwOtpts.isVoidedPin.gpioOtptPin, INPUT);
      digitalWrite(rghtHndHwOtpts.isVoidedPin.gpioOtptPin, (rghtHndHwOtpts.isVoidedPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(rghtHndHwOtpts.isVoidedPin.gpioOtptPin, OUTPUT);
   }
   else if((rghtHndHwOtpts.isVoidedPin.gpioOtptPin > _maxValidPinNum)){
      Error_Handler();
   }
   //------------------->> Right hand TmVdblMPBttn output pins configuration END

   //----------------->> Foot SnglSrvcVdblMPBttn output pins configuration BEGIN
   if((ftSwtchIsEnbldOtpt.gpioOtptPin != _InvalidPinNum) && (ftSwtchIsEnbldOtpt.gpioOtptPin <= _maxValidPinNum)){
      pinMode(ftSwtchIsEnbldOtpt.gpioOtptPin, INPUT);
      digitalWrite(ftSwtchIsEnbldOtpt.gpioOtptPin, (ftSwtchIsEnbldOtpt.gpioOtptActHgh)?LOW:HIGH);
      pinMode(ftSwtchIsEnbldOtpt.gpioOtptPin, OUTPUT);
   } 
   else if((ftSwtchIsEnbldOtpt.gpioOtptPin > _maxValidPinNum)){
      Error_Handler();
   }
   //------------------->> Foot SnglSrvcVdblMPBttn output pins configuration END

   //------------------>> LimbsSftyLnFSwtch output pins configuration BEGIN
   //! The ltchRlsIsOnOtpt is the ONLY FUNDAMENTAL pin that is REQUIRED to be a valid available addressable pin number. All the of the rest of the output pins are for information provision, and might be assigned to a _InvalidPinNum to be ignored.
   if((ltchRlsIsOnOtpt.gpioOtptPin != _InvalidPinNum) && (ltchRlsIsOnOtpt.gpioOtptPin <= _maxValidPinNum)){
      pinMode(ltchRlsIsOnOtpt.gpioOtptPin, INPUT);
      digitalWrite(ltchRlsIsOnOtpt.gpioOtptPin, (ltchRlsIsOnOtpt.gpioOtptActHgh)?LOW:HIGH);
   }
   else{
      Error_Handler();
   }

   // The rest of the possible connected pins are configured as OUTPUTS, setting its' starting values at the INACTIVE value
   if((prdCyclIsOnOtpt.gpioOtptPin != _InvalidPinNum) && (prdCyclIsOnOtpt.gpioOtptPin <= _maxValidPinNum)){
      digitalWrite(prdCyclIsOnOtpt.gpioOtptPin, (prdCyclIsOnOtpt.gpioOtptActHgh)?LOW:HIGH);
      pinMode(prdCyclIsOnOtpt.gpioOtptPin, OUTPUT);
   }
   else if((prdCyclIsOnOtpt.gpioOtptPin > _maxValidPinNum)){
      Error_Handler();
   }

   pinMode(ltchRlsIsOnOtpt.gpioOtptPin, OUTPUT); // Setting the main activation output pin to OUTPUT mode
   //------------------>> LimbsSftyLnFSwtch output pins configuration END

   LimbsSftyLnFSwtch stampSftySwtch (lftHndHwAttrbts, lftHndBhvrSUp, rghtHndHwAttrbts, rghtHndBhvrSUp, ftHwAttrbts, ftBhvrSUp, lsssSwtchWrkngPrm);

   stampSftySwtch.setUndrlSwtchsPollDelay(UndrlyngMPBttnPollTm);
   stampSftySwtch.begin(LsSwtchPollTm);

   for(;;){
      *loopTmrStrtTmPtr = xTaskGetTickCount() / portTICK_RATE_MS; //! Altough this is just a test execution, this is implemented to save resources by not executing this loop constantly

      // Keep the _undrlLftHndMPB object outputs updated.		
      if(stampSftySwtch.getLftHndSwtchPtr()->getOutputsChange()){
         while(stampSftySwtch.getLftHndSwtchPtr()->getOutputsChange()){
            if(lftHndHwOtpts.isOnPin.gpioOtptPin != _InvalidPinNum){ // Keep the left hand switch isOn output updated
               if(digitalRead(lftHndHwOtpts.isOnPin.gpioOtptPin) != ((stampSftySwtch.getLftHndSwtchPtr()->getIsOn() == lftHndHwOtpts.isOnPin.gpioOtptActHgh)?HIGH:LOW))
                  digitalWrite(lftHndHwOtpts.isOnPin.gpioOtptPin, (stampSftySwtch.getLftHndSwtchPtr()->getIsOn() == lftHndHwOtpts.isOnPin.gpioOtptActHgh)?HIGH:LOW);
            }
            if(lftHndHwOtpts.isEnabledPin.gpioOtptPin != _InvalidPinNum){  // Keep the left hand switch isEnabled output updated
               if(digitalRead(lftHndHwOtpts.isEnabledPin.gpioOtptPin) != ((stampSftySwtch.getLftHndSwtchPtr()->getIsEnabled() == lftHndHwOtpts.isEnabledPin.gpioOtptActHgh)?HIGH:LOW))
                  digitalWrite(lftHndHwOtpts.isEnabledPin.gpioOtptPin, (stampSftySwtch.getLftHndSwtchPtr()->getIsEnabled() == lftHndHwOtpts.isEnabledPin.gpioOtptActHgh)?HIGH:LOW);
            }
            if(lftHndHwOtpts.isVoidedPin.gpioOtptPin != _InvalidPinNum){   // Keep the left hand switch isVoided output updated
               if(digitalRead(lftHndHwOtpts.isVoidedPin.gpioOtptPin) != ((stampSftySwtch.getLftHndSwtchPtr()->getIsVoided() == lftHndHwOtpts.isVoidedPin.gpioOtptActHgh)?HIGH:LOW))
                  digitalWrite(lftHndHwOtpts.isVoidedPin.gpioOtptPin, (stampSftySwtch.getLftHndSwtchPtr()->getIsVoided() == lftHndHwOtpts.isVoidedPin.gpioOtptActHgh)?HIGH:LOW);
            }         
            stampSftySwtch.getLftHndSwtchPtr()->setOutputsChange(false);
         }      
      }

      // Keep the _undrlRghtHndMPB object outputs updated.
      if(stampSftySwtch.getRghtHndSwtchPtr()->getOutputsChange()){
         while(stampSftySwtch.getRghtHndSwtchPtr()->getOutputsChange()){
            if(rghtHndHwOtpts.isOnPin.gpioOtptPin != _InvalidPinNum){
               if(digitalRead(rghtHndHwOtpts.isOnPin.gpioOtptPin) != ((stampSftySwtch.getRghtHndSwtchPtr()->getIsOn() == rghtHndHwOtpts.isOnPin.gpioOtptActHgh)?HIGH:LOW))
                  digitalWrite(rghtHndHwOtpts.isOnPin.gpioOtptPin, (stampSftySwtch.getRghtHndSwtchPtr()->getIsOn() == rghtHndHwOtpts.isOnPin.gpioOtptActHgh)?HIGH:LOW);
            }
            if(rghtHndHwOtpts.isEnabledPin.gpioOtptPin != _InvalidPinNum){
               if(digitalRead(rghtHndHwOtpts.isEnabledPin.gpioOtptPin) != ((stampSftySwtch.getRghtHndSwtchPtr()->getIsEnabled() == rghtHndHwOtpts.isEnabledPin.gpioOtptActHgh)?HIGH:LOW))
                  digitalWrite(rghtHndHwOtpts.isEnabledPin.gpioOtptPin, (stampSftySwtch.getRghtHndSwtchPtr()->getIsEnabled() == rghtHndHwOtpts.isEnabledPin.gpioOtptActHgh)?HIGH:LOW);
            }
            if(rghtHndHwOtpts.isVoidedPin.gpioOtptPin != _InvalidPinNum){
               if(digitalRead(rghtHndHwOtpts.isVoidedPin.gpioOtptPin) != ((stampSftySwtch.getRghtHndSwtchPtr()->getIsVoided() == rghtHndHwOtpts.isVoidedPin.gpioOtptActHgh)?HIGH:LOW))
                  digitalWrite(rghtHndHwOtpts.isVoidedPin.gpioOtptPin, (stampSftySwtch.getRghtHndSwtchPtr()->getIsVoided() == rghtHndHwOtpts.isVoidedPin.gpioOtptActHgh)?HIGH:LOW);
            }
            stampSftySwtch.getRghtHndSwtchPtr()->setOutputsChange(false);
         }
      }

      // Keep the _undrlFtdMPB object outputs updated.	      
      if(stampSftySwtch.getFtSwtchPtr()->getOutputsChange()){
         while(stampSftySwtch.getFtSwtchPtr()->getOutputsChange()){
            if(ftSwtchIsEnbldOtpt.gpioOtptPin != _InvalidPinNum){
               if(digitalRead(ftSwtchIsEnbldOtpt.gpioOtptPin) != ((stampSftySwtch.getFtSwtchPtr()->getIsEnabled() == ftSwtchIsEnbldOtpt.gpioOtptActHgh)?HIGH:LOW))
                  digitalWrite(ftSwtchIsEnbldOtpt.gpioOtptPin, (stampSftySwtch.getFtSwtchPtr()->getIsEnabled() == ftSwtchIsEnbldOtpt.gpioOtptActHgh)?HIGH:LOW);
            }
            stampSftySwtch.getFtSwtchPtr()->setOutputsChange(false);         
         }
      }

      // Keep the LimbsSftyLnFSwtch object outputs updated.
      while(stampSftySwtch.getLsSwtchOtptsChng()){
         if(digitalRead(ltchRlsIsOnOtpt.gpioOtptPin) != ((stampSftySwtch.getLtchRlsIsOn() == ltchRlsIsOnOtpt.gpioOtptActHgh)?HIGH:LOW))
            digitalWrite(ltchRlsIsOnOtpt.gpioOtptPin, (stampSftySwtch.getLtchRlsIsOn() == ltchRlsIsOnOtpt.gpioOtptActHgh)?HIGH:LOW);

         if(prdCyclIsOnOtpt.gpioOtptPin != _InvalidPinNum){
            if(digitalRead(prdCyclIsOnOtpt.gpioOtptPin) != ((stampSftySwtch.getPrdCyclIsOn() == prdCyclIsOnOtpt.gpioOtptActHgh)?HIGH:LOW))
               digitalWrite(prdCyclIsOnOtpt.gpioOtptPin, (stampSftySwtch.getPrdCyclIsOn() == prdCyclIsOnOtpt.gpioOtptActHgh)?HIGH:LOW);
         }
         stampSftySwtch.setLsSwtchOtptsChng(false);
      }
   
      vTaskDelayUntil(loopTmrStrtTmPtr, totalDelay);  //! Complementary code for the implementation to save resources by blocking this piece of code as is not needed to be executed constantly   
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