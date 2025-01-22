/**
  ******************************************************************************
  * @file	: LimbsSftyLnFSwtch_Example_01rc01.cpp
  * @brief  : Implementation of a Limbs Safety switch using a LimbsSftyLnFSwtch class object
  * 
  * Example for the LimbsSftySw_ESP32 library LimbsSftyLnFSwtch class
  * 
  * This example creates ONE Task in the standard Arduino LoopTask, and makes the LoopTask to delete itself.
  * The task created as the Main Control Task will operate locally, including:
  * - Input pins configuration parameters
  * - Output pins configuration parameters
  * - Behavior configuration parameters
  * - Underlying DbncdMPBttn sublclasses objects' Attribute Flags related outputs update
  * - LimbsSftyLnFSwtch object Attribute Flags related outputs update
  *
  * Framework: Arduino
  * Platform: ESP32
  * 
  * @author	: Gabriel D. Goldman
  *
  * @date First release: 28/11/2024 
  *       Last update:   16/01/2025 13:55 GMT+0300
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
   .lsSwExecTskPrrtyCnfg = 3,
};

void setup() { 
  //Create the Main control task for setup and execution of the main code
   xReturned = xTaskCreatePinnedToCore(
      mainCtrlTsk,  //Callback function/task to be called
      "MainControlTask",  //Name of the task
      2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  //Pointer to the parameters for the function to work with
      // configTIMER_TASK_PRIORITY,  //Priority level given to the task
      mnCtrlTskConf.lsSwExecTskPrrtyCnfg,
      &mainCtrlTskHndl, //Task handle
      // xPortGetCoreID() //Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
      mnCtrlTskConf.lsSwExecTskCore
   );
   if(xReturned != pdPASS)
      Error_Handler();
}

void loop() {
   vTaskDelete(NULL); // Delete this task -the ESP-Arduino LoopTask()- and remove it from the execution list
}  

//===============================>> User Tasks Implementations BEGIN
void mainCtrlTsk(void *pvParameters){
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
      .swtchIsEnbld = true,
      .swtchVdTm = 5000,
   };
   swtchBhvrCfg_t rghtHndBhvrSUp{   // Right hand switch behavior configuration properties
      .swtchStrtDlyTm = 100,
      .swtchIsEnbld = true,
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
   
   LimbsSftyLnFSwtch stampSftySwtch (lftHndHwAttrbts, lftHndBhvrSUp, rghtHndHwAttrbts, rghtHndBhvrSUp, ftHwAttrbts, ftBhvrSUp, lsssSwtchWrkngPrm);

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

   //The rest of the possible connected pins are configured as OUTPUTS, setting its' starting values at the INACTIVE value
   if((prdCyclIsOnOtpt.gpioOtptPin != _InvalidPinNum) && (prdCyclIsOnOtpt.gpioOtptPin <= _maxValidPinNum)){
      digitalWrite(prdCyclIsOnOtpt.gpioOtptPin, (prdCyclIsOnOtpt.gpioOtptActHgh)?LOW:HIGH);
      pinMode(prdCyclIsOnOtpt.gpioOtptPin, OUTPUT);
   }
   else if((prdCyclIsOnOtpt.gpioOtptPin > _maxValidPinNum)){
      Error_Handler();
   }

   pinMode(ltchRlsIsOnOtpt.gpioOtptPin, OUTPUT); // Setting the main activation output pin to OUTPUT mode
   //------------------>> LimbsSftyLnFSwtch output pins configuration END

   stampSftySwtch.begin(50);

   for(;;){
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
