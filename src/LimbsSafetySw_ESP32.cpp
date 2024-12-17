/**
  ******************************************************************************
  * @file	: LimbSafetySw_ESP32.cpp
  * @brief	: Source file for the LimbSafetySw_ESP32 library classes
  *
  * @details The library implements classes that model Limbs Safety Switch for
  * industrial production machines.
  *
  * @author	: Gabriel D. Goldman
  * @version v1.0.0
  * @date	: Created on: 06/11/2023
  * 		: Last modification: 08/11/2024
  * @copyright GPL-3.0 license
  *
  ******************************************************************************
  * @attention	This library was developed as part of the refactoring process for
  * an industrial machines security enforcement and productivity control
  * (hardware & firmware update). As such every class included complies **AT LEAST**
  * with the provision of the attributes and methods to make the hardware & firmware
  * replacement transparent to the controlled machines. Generic use attribute and
  * methods were added to extend the usability to other projects and application
  * environments, but no fitness nor completeness of those are given but for the
  * intended refactoring project.
  * 
  * @warning **Use of this library is under your own responsibility**
  ******************************************************************************
  */

#include "LimbsSafetySw_ESP32.h"

//=========================================================================> Class methods delimiter
TaskHandle_t LimbsSftySnglShtSw::lssTskToNtfyHndl = NULL;

LimbsSftySnglShtSw::LimbsSftySnglShtSw()
{
}

LimbsSftySnglShtSw::LimbsSftySnglShtSw(swtchInptHwCfg_t lftHndInpPrm, swtchInptHwCfg_t rghtHndInpPrm, swtchInptHwCfg_t ftInpPrm,
                                       int8_t sftySwActvOtpPin,
                                       swtchOtptHwCfg_t lftHndOtptPrm, swtchOtptHwCfg_t rghtHndOtptPrm, swtchOtptHwCfg_t ftOtptPrm,
                                       int8_t hndsSwtchsOkPin)
:_lftHndInpCfg{lftHndInpPrm}, _rghtHndInpCfg{rghtHndInpPrm}, _ftInpCfg{ftInpPrm},
_sftySwActvOtpPin{sftySwActvOtpPin}, 
_lftHndOtptCfg{lftHndOtptPrm}, _rghtHndOtptCfg{rghtHndOtptPrm}, _ftOtptCfg{ftOtptPrm},
_hndsSwtchsOkPin{hndsSwtchsOkPin}
{
   // Build DbncdMPBttn objects and pointers
    TmVdblMPBttn _undrlLftHndHTVMPB(_lftHndInpCfg.inptPin, _lftHndSwCfg.hndSwtchVdTm /*_stdTVMPBttnVoidTime*/, _lftHndInpCfg.pulledUp, _lftHndInpCfg.typeNO, _lftHndInpCfg.dbncTime, _lftHndSwCfg.hndSwtchDlyTm/*_stdTVMPBttnDelayTime*/, true);
   _undrlLftHndHTVMPBPtr = &_undrlLftHndHTVMPB;   
   TmVdblMPBttn _undrlRghtHndHTVMPB (_rghtHndInpCfg.inptPin, _rghtHndSwCfg.hndSwtchVdTm/*_stdTVMPBttnVoidTime*/, _rghtHndInpCfg.pulledUp, _rghtHndInpCfg.typeNO, _rghtHndInpCfg.dbncTime, _rghtHndSwCfg.hndSwtchDlyTm/*_stdTVMPBttnDelayTime*/, true);
   _undrlRghtHndHTVMPBPtr = &_undrlRghtHndHTVMPB;
   SnglSrvcVdblMPBttn _undrlFtSSVMPB (_ftInpCfg.inptPin, _ftInpCfg.pulledUp, _ftInpCfg.typeNO, _ftInpCfg.dbncTime, _ftSwCfg.hndSwtchDlyTm /*_stdSSVMPBttnDelayTime*/);
   _undrlFtSSVMPBPtr = &_undrlFtSSVMPB;

   // Configuration of the isEnabled state of both hands switches. Note: TmVdblMPBttn are instantiated with _isEnabled = true property value
   if(!_lftHndSwCfg.hndSwtchEnbld)
      _undrlLftHndHTVMPBPtr->disable();
   if(!_rghtHndSwCfg.hndSwtchEnbld)
      _undrlRghtHndHTVMPBPtr->disable();
      
   // Set the output pins to the required states
   // Altough is expected after a Turn on/Reset that the MCU starts with all
   // it's GPIO pins configured as Open Drain Input, but for security reasons
   // the main activation switch output is first set to INPUT "Deactivated"
   // It must be expressly set to OUTPUT just after configuration and before use
   pinMode(_sftySwActvOtpPin.gpioOtptPin, INPUT);
   digitalWrite(_sftySwActvOtpPin.gpioOtptPin, (_sftySwActvOtpPin.gpioOtptActHgh)?LOW:HIGH);

   //The rest of te possible connected pins are configured as OUTPUTS, setting it's starting values as LOW/Reset
   if(_lftHndOtptCfg.isOnOtptPin != _InvalidPinNum){
      digitalWrite(_lftHndOtptCfg.isOnOtptPin, (_lftHndOtptCfg.isOnOtptActHgh)?LOW:HIGH);  //Deactivate pin
      pinMode(_lftHndOtptCfg.isOnOtptPin, OUTPUT);
   }
   if(_lftHndOtptCfg.isVddOtptPin != _InvalidPinNum){
      digitalWrite(_lftHndOtptCfg.isVddOtptPin, (_lftHndOtptCfg.isVddOtptActHgh)?LOW:HIGH); //Deactivate pin
      pinMode(_lftHndOtptCfg.isVddOtptPin, OUTPUT);
   }
   if(_lftHndOtptCfg.isEnbldOtptPin != _InvalidPinNum){
      digitalWrite(_lftHndOtptCfg.isEnbldOtptPin,(_lftHndOtptCfg.isEnabledOtptActHgh)?LOW:HIGH);  //Deactivate pin
      pinMode(_lftHndOtptCfg.isEnbldOtptPin, OUTPUT);
   }
   
   if(_rghtHndOtptCfg.isOnOtptPin != _InvalidPinNum){
      digitalWrite(_rghtHndOtptCfg.isOnOtptPin,(_rghtHndOtptCfg.isOnOtptActHgh)?LOW:HIGH); //Deactivate pin
      pinMode(_rghtHndOtptCfg.isOnOtptPin, OUTPUT);
   }
   if(_rghtHndOtptCfg.isVddOtptPin != _InvalidPinNum){
      digitalWrite(_rghtHndOtptCfg.isVddOtptPin, (_rghtHndOtptCfg.isVddOtptActHgh)?LOW:HIGH);   //Deactivate pin
      pinMode(_rghtHndOtptCfg.isVddOtptPin, OUTPUT);
   }
   if(_rghtHndOtptCfg.isEnbldOtptPin != _InvalidPinNum){
      digitalWrite(_rghtHndOtptCfg.isEnbldOtptPin, (_rghtHndOtptCfg.isEnabledOtptActHgh)?LOW:HIGH); //Deactivate pin
      pinMode(_rghtHndOtptCfg.isEnbldOtptPin, OUTPUT);
   }
   if(_hndsSwtchsOkPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_hndsSwtchsOkPin.gpioOtptPin, (_hndsSwtchsOkPin.gpioOtptActHgh)?LOW:HIGH);   //Deactivate pin
      pinMode(_hndsSwtchsOkPin.gpioOtptPin, OUTPUT);
   }
   
   pinMode(_sftySwActvOtpPin.gpioOtptPin, OUTPUT); // Setting the main activation output pin to OUTPUT mode
}

LimbsSftySnglShtSw::~LimbsSftySnglShtSw()
{
}

bool LimbsSftySnglShtSw::begin(unsigned long int updtPeriod){
   bool result {false};
	BaseType_t tmrModResult {pdFAIL};

	if (updtPeriod > 0){
   // Set the logical underlying MPBttns to start updating it's inputs readings & output states
      result = _undrlLftHndHTVMPBPtr->begin(20);
      if(result){
         result = _undrlRghtHndHTVMPBPtr->begin(20);
         if(result){
            result = _undrlFtSSVMPBPtr->begin(20);
            if(result){
               if (!_lsssSwtchPollTmrHndl){        
                  _lsssSwtchPollTmrHndl = xTimerCreate(
                     _swtchPollTmrName.c_str(),  //Timer name
                     pdMS_TO_TICKS(updtPeriod),  //Timer period in ticks
                     pdTRUE,     //Auto-reload true
                     this,       //TimerID: data passed to the callback function to work
                     lsssSwtchPollCb	  //Callback function
                  );
                  if (_lsssSwtchPollTmrHndl != NULL){
                     tmrModResult = xTimerStart(_lsssSwtchPollTmrHndl, portMAX_DELAY);
                     if (tmrModResult == pdPASS)
                        result = true;
                  }
               }
            }
         }
      }
	}

	return result;
}

void LimbsSftySnglShtSw::clrStatus(){

   return;
}

void LimbsSftySnglShtSw::_clrSttChng(){
   _sttChng = false;

   return;
}

bool LimbsSftySnglShtSw::getBothHndsSwOk(){

   return ((_undrlLftHndHTVMPBPtr->getIsOn()) && (_undrlRghtHndHTVMPBPtr->getIsOn()));
}

void LimbsSftySnglShtSw::lsssSwtchPollCb(TimerHandle_t lssTmrCbArg){
   LimbsSftySnglShtSw* lsssSwtchObj = (LimbsSftySnglShtSw*)pvTimerGetTimerID(lssTmrCbArg);
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
/**	if(mpbObj->getIsEnabled()){
		// Input/Output signals update
		mpbObj->updIsPressed();
		// Flags/Triggers calculation & update
		mpbObj->updValidPressesStatus();
		mpbObj->updValidUnlatchStatus();
		mpbObj->updWrnngOn();
		mpbObj->updPilotOn();
	}
 	// State machine state update
 	mpbObj->updFdaState();
 	taskEXIT_CRITICAL(&mux);

	//Outputs update, function and tasks executions based on outputs changed generated by the State Machine
	if (mpbObj->getOutputsChange()){
		if(mpbObj->getTaskToNotify() != NULL){
			xTaskNotify(
				mpbObj->getTaskToNotify(),	//TaskHandle_t of the task receiving notification
				static_cast<unsigned long>(mpbObj->getOtptsSttsPkgd()),
				eSetValueWithOverwrite
			);
			mpbObj->setOutputsChange(false);
		}
	}
 */    

	return;
//}
}

void LimbsSftySnglShtSw::_setSttChng(){
   _sttChng = true;

   return;
}

void LimbsSftySnglShtSw::_updBothHndsSwState(){
   _bothHandsSwOk = getBothHndsSwOk();

   return;
}

void LimbsSftySnglShtSw::_updFdaState(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	switch(_lssFdaState){
		case stOffNotBHP:
			//In: >>---------------------------------->>
			if(_sttChng){
				clrStatus();
				_clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(getBothHndsSwOk()){
				// Set "both hands pressed" attribute flag = true
            // Set period enabled timer = 0
            // Enable FtSwitch
            // If no External Accomplished signal expected, ensure the siganl is set to false
            _lssFdaState = stBHPNotFP;
				_setSttChng();	//Set flag to execute exiting OUT code
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stBHPNotFP:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!getBothHndsSwOk()){
            _lssFdaState = stOffNotBHP;
            _setSttChng();
         }
         else{

         }
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOn:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			//_mpbFdaState = stOffNotVPP;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnTaSP:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			//_mpbFdaState = stOffNotVPP;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnToSR:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			//_mpbFdaState = stOffNotVPP;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stFailExcepHndl:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			//_mpbFdaState = stOffNotVPP;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

	default:
		break;
	}
	taskEXIT_CRITICAL(&mux);

	return;

}

bool LimbsSftySnglShtSw::_updOutputs(){
   /*
    if(!_underlMPB->getIsEnabled()){ // The switch is DISABLED, handle the outputs accordingly
        // Set disabledPin ON
        updIsEnabled(false);
        //If the switch is disabled there should be no voided indication
        updIsVoided(false);
        // if the switch should stay on when disabled
        updIsOn(_underlMPB->getIsOnDisabled());
        // if(_onIfDisabled){      
        //     updIsOn(true);  // LoadPin is forced ON
        // }
        // else{
        //     updIsOn(false); // LoadPin is forced OFF
        // }
    }
    else{
        //The switch is ENABLED, update outputs accordingly
        //Update disabled pin
        updIsEnabled(true);   // Set disabledPin OFF
        //Update voided status/pins
        updIsVoided(_underlMPB->getIsVoided());
        //Update On status/pins
        updIsOn(_underlMPB->getIsOn());
    }
    */ 
    return true;
}

/*bool LimbsSftySnglShtSw::updIsEnabled(const bool &enabledValue){
    if(_disabledPin > 0){
        if(enabledValue != (!digitalRead(_disabledPin)))
            digitalWrite(_disabledPin, !enabledValue);
    }

    return enabledValue;

}*/

/*bool LimbsSftySnglShtSw::updIsOn(const bool &onValue){

   return true;
}
*/


//=========================================================================> Class methods delimiter
