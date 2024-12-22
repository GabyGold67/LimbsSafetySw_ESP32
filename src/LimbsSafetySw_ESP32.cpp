/**
  ******************************************************************************
  * @file	: LimbSafetySw_ESP32.cpp
  * @brief	: Source file for the LimbSafetySw_ESP32 library classes
  *
  * @details The library implements classes that models Limbs Safety Switch for
  * industrial production machines.
  *
  * @author	: Gabriel D. Goldman
  * @version v1.0.0
  * @date	: Created on: 06/11/2023
  * 		: Last modification: 18/12/2024
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

LimbsSftySnglShtSw::LimbsSftySnglShtSw(swtchInptHwCfg_t lftHndInpCfg, swtchInptHwCfg_t rghtHndInpCfg, swtchInptHwCfg_t ftInpCfg,
                                       int8_t sftySwActvOtpPin,
                                       swtchOtptHwCfg_t lftHndOtptCfg, swtchOtptHwCfg_t rghtHndOtptCfg, swtchOtptHwCfg_t ftOtptCfg,
                                       int8_t hndsSwtchsOkPin)
:_lftHndInpCfg{lftHndInpCfg}, _rghtHndInpCfg{rghtHndInpCfg}, _ftInpCfg{ftInpCfg},
_sftySwActvOtpPin{sftySwActvOtpPin}, 
_lftHndOtptCfg{lftHndOtptCfg}, _rghtHndOtptCfg{rghtHndOtptCfg}, _ftOtptCfg{ftOtptCfg},
_hndsSwtchsOkPin{hndsSwtchsOkPin}
{
   // Build DbncdMPBttn objects and pointers
    TmVdblMPBttn _undrlLftHndHTVMPB(_lftHndInpCfg.inptPin, _lftHndSwCfg.swtchVdTm, _lftHndInpCfg.pulledUp, _lftHndInpCfg.typeNO, _lftHndInpCfg.dbncTime, _lftHndSwCfg.swtchStrtDlyTm, true);
   _undrlLftHndHTVMPBPtr = &_undrlLftHndHTVMPB;   
   TmVdblMPBttn _undrlRghtHndHTVMPB (_rghtHndInpCfg.inptPin, _rghtHndSwCfg.swtchVdTm, _rghtHndInpCfg.pulledUp, _rghtHndInpCfg.typeNO, _rghtHndInpCfg.dbncTime, _rghtHndSwCfg.swtchStrtDlyTm, true);
   _undrlRghtHndHTVMPBPtr = &_undrlRghtHndHTVMPB;
   SnglSrvcVdblMPBttn _undrlFtSSVMPB (_ftInpCfg.inptPin, _ftInpCfg.pulledUp, _ftInpCfg.typeNO, _ftInpCfg.dbncTime, _ftSwCfg.swtchStrtDlyTm);
   _undrlFtSSVMPBPtr = &_undrlFtSSVMPB;

   // Configuration of the isEnabled state of both hands switches. Note: TmVdblMPBttn are instantiated with _isEnabled = true property value
   if(!_lftHndSwCfg.swtchIsEnbld)
      _undrlLftHndHTVMPBPtr->disable();
   if(!_rghtHndSwCfg.swtchIsEnbld)
      _undrlRghtHndHTVMPBPtr->disable();
      
   /*Set the output pins to the required states
   Altough is expected after a Turn on/Reset that the MCU starts with all
   it's GPIO pins configured as Open Drain Input, but for security reasons
   the main activation switch output is first set to INPUT "Deactivated"
   It must be expressly set to OUTPUT just after configuration and before use
   */
   pinMode(_sftySwActvOtpPin.gpioOtptPin, INPUT);
   digitalWrite(_sftySwActvOtpPin.gpioOtptPin, (_sftySwActvOtpPin.gpioOtptActHgh)?LOW:HIGH);

   //The rest of te possible connected pins are configured as OUTPUTS, setting it's starting values as LOW/Reset
   if(_lftHndOtptCfg.isOnPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_lftHndOtptCfg.isOnPin.gpioOtptPin, (_lftHndOtptCfg.isOnPin.gpioOtptActHgh)?LOW:HIGH);  //Deactivate pin
      pinMode(_lftHndOtptCfg.isOnPin.gpioOtptPin, OUTPUT);
   }
   if(_lftHndOtptCfg.isVoidedPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_lftHndOtptCfg.isVoidedPin.gpioOtptPin, (_lftHndOtptCfg.isVoidedPin.gpioOtptActHgh)?LOW:HIGH); //Deactivate pin
      pinMode(_lftHndOtptCfg.isVoidedPin.gpioOtptPin, OUTPUT);
   }
   if(_lftHndOtptCfg.isEnabledPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_lftHndOtptCfg.isEnabledPin.gpioOtptPin,(_lftHndOtptCfg.isEnabledPin.gpioOtptActHgh)?LOW:HIGH);  //Deactivate pin
      pinMode(_lftHndOtptCfg.isEnabledPin.gpioOtptPin, OUTPUT);
   }
   
   if(_rghtHndOtptCfg.isOnPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_rghtHndOtptCfg.isOnPin.gpioOtptPin,(_rghtHndOtptCfg.isOnPin.gpioOtptActHgh)?LOW:HIGH); //Deactivate pin
      pinMode(_rghtHndOtptCfg.isOnPin.gpioOtptPin, OUTPUT);
   }
   if(_rghtHndOtptCfg.isVoidedPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_rghtHndOtptCfg.isVoidedPin.gpioOtptPin, (_rghtHndOtptCfg.isVoidedPin.gpioOtptActHgh)?LOW:HIGH);   //Deactivate pin
      pinMode(_rghtHndOtptCfg.isVoidedPin.gpioOtptPin, OUTPUT);
   }
   if(_rghtHndOtptCfg.isEnabledPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_rghtHndOtptCfg.isEnabledPin.gpioOtptPin, (_rghtHndOtptCfg.isEnabledPin.gpioOtptActHgh)?LOW:HIGH); //Deactivate pin
      pinMode(_rghtHndOtptCfg.isEnabledPin.gpioOtptPin, OUTPUT);
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
      result = _undrlLftHndHTVMPBPtr->begin(20);   // Set the underlying left hand MPBttns to start updating it's input readings & output states
      if(result){
         result = _undrlRghtHndHTVMPBPtr->begin(20);  // Set the underlying right hand MPBttns to start updating it's input readings & output states
         if(result){
            result = _undrlFtSSVMPBPtr->begin(20); // Set the underlying foot MPBttns to start updating it's input readings & output states
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
   _bothHandsSwOk = false;
   _rlsMchnsmStrtTm = 0;
   _dvcActvtnStrtTm = 0;
   _dvcDeactvtnSgnl = false;

   return;
}

void LimbsSftySnglShtSw::_clrSttChng(){
   _sttChng = false;

   return;
}

bool LimbsSftySnglShtSw::_cnfgHndSwtch(const bool &isLeft, const limbSftySwCfg_t &newCfg){
   bool result{false};
   TmVdblMPBttn* hndSwtchToCnf {nullptr};

   if(isLeft)
      hndSwtchToCnf = _undrlLftHndHTVMPBPtr;
   else
      hndSwtchToCnf = _undrlRghtHndHTVMPBPtr;

   if(hndSwtchToCnf->getStrtDelay() != newCfg.swtchStrtDlyTm)
      hndSwtchToCnf->setStrtDelay(newCfg.swtchStrtDlyTm);
   
   if(hndSwtchToCnf->getIsEnabled() != newCfg.swtchIsEnbld){
      if(newCfg.swtchIsEnbld)
         hndSwtchToCnf->enable();
      else
         hndSwtchToCnf->disable();
   }

   if(hndSwtchToCnf->getVoidTime() != newCfg.swtchVdTm)
      hndSwtchToCnf->setVoidTime(newCfg.swtchVdTm);

   return result;
}

bool LimbsSftySnglShtSw::cnfgLftHndSwtch(const limbSftySwCfg_t &newCfg){

   return _cnfgHndSwtch(true, newCfg);
}

bool LimbsSftySnglShtSw::cnfgRghtHndSwtch(const limbSftySwCfg_t &newCfg){

   return _cnfgHndSwtch(false, newCfg);
}

bool LimbsSftySnglShtSw::getBothHndsSwOk(){

   return ((_undrlLftHndHTVMPBPtr->getIsOn()) && (_undrlRghtHndHTVMPBPtr->getIsOn()));
}

bool LimbsSftySnglShtSw::getActvCntrlChkpnt(){

   return _actvCntrlChkpntOk;
}

void LimbsSftySnglShtSw::lsssSwtchPollCb(TimerHandle_t lssTmrCbArg){
   LimbsSftySnglShtSw* lsssSwtchObj = (LimbsSftySnglShtSw*)pvTimerGetTimerID(lssTmrCbArg);
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
   // Input/Output signals update
	lsssSwtchObj->_updBothHndsSwState();   

   // Flags/Triggers calculation & update
/* mpbObj->updValidPressesStatus();
   mpbObj->updValidUnlatchStatus();
*/ 
	// State machine update
 	lsssSwtchObj->_updFdaState();
 	taskEXIT_CRITICAL(&mux);

	//Outputs update, function and tasks executions based on outputs changed generated by the State Machine
	/*if (mpbObj->getOutputsChange()){
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
}

void LimbsSftySnglShtSw::setActvCntrlChkpnt(const bool &newVal){
   if(_actvCntrlChkpntOk != newVal)
      _actvCntrlChkpntOk = newVal;

   return;
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
   /*
   ## States description
   
Brief: stOffNotBHP
Meaning: State off, not both hands pressed
In: 
   - Clean and reset data and flags
   - Default: Clear **State Change** flag
Do:
   - Check both hands isOn condition: If (_bothHandsSwOk)   
      - New State: stOffBHPNotFP
      - Set **State Change** flag
Out:    
   - Set BHOn flag (for output signaling purposes)
   - Set OtpsChng flag and increment OtpsChngCnt
   - Enable FtMPB
-------------------------------------------------------->>
Brief: stOffBHPNotFP
Meaning: State off, both hands pressed, not Foot press
In: 
   - Default: Clear **State Change** flag
Do:
   - If (_bothHandsSwOk == false)
      - Disable FtMPB
      - Disable BHOn flag
      - New State: stOffNotBHP
      - Set **State Change** flag
   - Else if (flag FtOn = true):
      - Save lftHndSwtch isEnabled value
      - Save rghtHndSwtch isEnabled value
      - Set lftHndSwtch isOnDisabled = false
      - Set rghtHndSwtch isOnDisabled = false
      - Set lftHndSwtch isEnabled = false
      - Set rghtHndSwtch isEnabled = false
      - New State: stActivated
      - Set **State Change** flag
Out:    
   - N/A
-------------------------------------------------------->>
Brief: stActivated
Meaning: State activate device
In:
   - Set Release Mechanism start time
   - Set Device Activation start time
   - Reset Deactivation flag
   - set Release flag
   - Set OtpsChng flag and increment OtpsChngCnt
   - Default: Clear **State Change** flag
Do:
   - if (Timer == ReleaseTime)
      - Reset Release flag
      - Set OtpsChng flag and increment OtpsChngCnt
      - New State: stActvtdTaSP
      - Set **State Change** flag
Out:    
   - N/A
-------------------------------------------------------->>
Brief: stActvtdTaSP
Meaning: State activated Time and signal pending
In:



   */
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
			if(_bothHandsSwOk){
            // Enable FtSwitch
            _undrlFtSSVMPBPtr->enable();
            _lssFdaState = stOffBHPNotFP;
				_setSttChng();	//Set flag to execute exiting OUT code
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffBHPNotFP:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!_bothHandsSwOk){
            _lssFdaState = stOffNotBHP;
            _setSttChng();
         }
         else{
            // Check the foot switch release signal ok flag
            if(_swtchRlsOk){
               _lssFdaState = stActivated;
               _setSttChng();
            }
            // If no External Accomplished signal expected, ensure the signal is set to false

         }
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stActivated:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			//_mpbFdaState = stOffNotVPP;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stActvtdTaSP:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			//_mpbFdaState = stOffNotVPP;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stActvtdoSR:
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

void setSwtchRlsStrt(){

   return;
}
void setSwtchRlsStp(){

   return;
}

