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

LimbsSftySnglShtSw::LimbsSftySnglShtSw(swtchInptHwCfg_t lftHndInpCfg, swtchInptHwCfg_t rghtHndInpCfg, swtchInptHwCfg_t ftInpCfg)
:_lftHndInpCfg{lftHndInpCfg}, _rghtHndInpCfg{rghtHndInpCfg}, _ftInpCfg{ftInpCfg}
{
   // Build DbncdMPBttn objects and pointers
    TmVdblMPBttn _undrlLftHndHTVMPB(_lftHndInpCfg.inptPin, _lftHndSwCfg.swtchVdTm, _lftHndInpCfg.pulledUp, _lftHndInpCfg.typeNO, _lftHndInpCfg.dbncTime, _lftHndSwCfg.swtchStrtDlyTm, true);
   _undrlLftHndHTVMPBPtr = &_undrlLftHndHTVMPB;   
   TmVdblMPBttn _undrlRghtHndHTVMPB (_rghtHndInpCfg.inptPin, _rghtHndSwCfg.swtchVdTm, _rghtHndInpCfg.pulledUp, _rghtHndInpCfg.typeNO, _rghtHndInpCfg.dbncTime, _rghtHndSwCfg.swtchStrtDlyTm, true);
   _undrlRghtHndHTVMPBPtr = &_undrlRghtHndHTVMPB;
   SnglSrvcVdblMPBttn _undrlFtSSVMPB (_ftInpCfg.inptPin, _ftInpCfg.pulledUp, _ftInpCfg.typeNO, _ftInpCfg.dbncTime, _ftSwCfg.swtchStrtDlyTm);
   _undrlFtSSVMPBPtr = &_undrlFtSSVMPB;

   // Configuration of the isEnabled state of both hands switches. Note: TmVdblMPBttn objects are instantiated with _isEnabled = true property value
   if(!_lftHndSwCfg.swtchIsEnbld)
      _undrlLftHndHTVMPBPtr->disable();
   if(!_rghtHndSwCfg.swtchIsEnbld)
      _undrlRghtHndHTVMPBPtr->disable();
      
}

LimbsSftySnglShtSw::~LimbsSftySnglShtSw()
{
}

bool LimbsSftySnglShtSw::begin(unsigned long int updtPeriod){
   bool result {false};
	BaseType_t tmrModResult {pdFAIL};

	if (updtPeriod > 0){      
      result = _undrlLftHndHTVMPBPtr->begin(_undrlSwtchsPollDelay);   // Set the underlying left hand MPBttns to start updating it's input readings & output states
      if(result){
         result = _undrlRghtHndHTVMPBPtr->begin(_undrlSwtchsPollDelay);  // Set the underlying right hand MPBttns to start updating it's input readings & output states
         if(result){
            result = _undrlFtSSVMPBPtr->begin(_undrlSwtchsPollDelay); // Set the underlying foot MPBttns to start updating it's input readings & output states
            if(result){
               if (!_lsSwtchPollTmrHndl){        
                  _lsSwtchPollTmrHndl = xTimerCreate(
                     _swtchPollTmrName.c_str(),  //Timer name
                     pdMS_TO_TICKS(updtPeriod),  //Timer period in ticks
                     pdTRUE,     //Auto-reload true
                     this,       //TimerID: the data passed as parametert to the callback function is this same object
                     lsSwtchPollCb	  //Callback function
                  );
                  if (_lsSwtchPollTmrHndl != NULL){
                     tmrModResult = xTimerStart(_lsSwtchPollTmrHndl, portMAX_DELAY);
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
   _bthHndsSwArOn = false;
   _prdCyclTmrStrt = 0;

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
   hndSwtchToCnf->setStrtDelay(newCfg.swtchStrtDlyTm);
   if(newCfg.swtchIsEnbld)
      hndSwtchToCnf->enable();
   else
      hndSwtchToCnf->disable();
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

const bool LimbsSftySnglShtSw::getOutputsChange() const
{

	return _outputsChange;
}

void LimbsSftySnglShtSw::lsSwtchPollCb(TimerHandle_t lssTmrCbArg){
   LimbsSftySnglShtSw* lsssSwtchObj = (LimbsSftySnglShtSw*)pvTimerGetTimerID(lssTmrCbArg);

	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
   // Underlying switches status recovery
   lsssSwtchObj->_updUndrlSwState();
   
   // Flags, Triggers and timers calculation & update
 	lsssSwtchObj->_updCurTime();
	// State machine update
 	lsssSwtchObj->_updFdaState();
 	taskEXIT_CRITICAL(&mux);

	//Outputs update, function and tasks executions based on outputs changed generated by the State Machine
	if (lsssSwtchObj->getOutputsChange()){
		/*if(mpbObj->getTaskToNotify() != NULL){
			xTaskNotify(
				mpbObj->getTaskToNotify(),	//TaskHandle_t of the task receiving notification
				static_cast<unsigned long>(mpbObj->getOtptsSttsPkgd()),
				eSetValueWithOverwrite
			);
			mpbObj->setOutputsChange(false);
		}
      */
	}     

	return;
}

bool LimbsSftySnglShtSw::setLtchRlsTm(const unsigned long int &newVal){
   bool result{false};

   if (_ltchRlsTtlTm != newVal){
      if((newVal > 0) && (newVal <= _prdCyclTtlTm)){
         _ltchRlsTtlTm = newVal;
         result = true;
      }
   }
   else{
      result = true;
   }

   return result;
}

void LimbsSftySnglShtSw::setOutputsChange(bool newOutputsChange){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	/*
	if(_outputsChange != newOutputsChange)
   	_outputsChange = newOutputsChange;
	*/
	if(newOutputsChange)
		++_outputsChangeCnt;
	else
		if(_outputsChangeCnt)
			--_outputsChangeCnt;

	/*if(_outputsChangeCnt)
		_outputsChange = true;
	else
		_outputsChange = false;

	if((_taskToNotifyHndl != NULL) && newOutputsChange)
		_outputsChngTskTrggr = true;*/
	taskEXIT_CRITICAL(&mux);

   return;
}

bool LimbsSftySnglShtSw::setPrdCyclTm(const unsigned long int &newVal){
   bool result{false};

   if(_prdCyclTtlTm != newVal){
      if((newVal > 0) && (newVal >= _ltchRlsTtlTm)){
         _prdCyclTtlTm = newVal;
         result = true;
      }
   }
   else{
      result = true;
   }
   
   return result;
}

void LimbsSftySnglShtSw::_setSttChng(){
   _sttChng = true;

   return;
}

void LimbsSftySnglShtSw::_updBothHndsSwState(){
   _bthHndsSwArOn = getBothHndsSwOk();

   return;
}

void LimbsSftySnglShtSw::_updCurTime(){
   _curTime = xTaskGetTickCount() / portTICK_RATE_MS;

   return;
}

void LimbsSftySnglShtSw::_updFdaState(){
/*
## States description

-------------------------------------------------------->>
Brief: stOffNotBHP
Meaning: State off, not both hands pressed
In: 
   - Clean and reset data and flags
   - Default: Clear **State Change** flag
Do:
   - Check if (both hands' isOn condition)
      - New State: stOffBHPNotFP
      - Set **State Change** flag
Out:    
   - set _bthHndsSwArOn = true
   - setOutputsChange(true); // This modifies the flag and the OtpsChngCnt
   - Enable FtMPB
-------------------------------------------------------->>
Brief: stOffBHPNotFP
Meaning: State off, both hands pressed, no Foot press
In: 
   - Default: Clear **State Change** flag
Do:
   - If ((both hands' isOn condition) == false)
      - Disable FtMPB
      - set _bthHndsSwArOn = false
      - setOutputsChange(true); // This modifies the flag and the OtpsChngCnt
      - New State: stOffNotBHP
      - Set **State Change** flag
   - Else if (flag FtOn = true):
      //- Save lftHndSwtch isEnabled value (to restore afterwards, actually not needed, just remember to reset value from config)
      //- Save rghtHndSwtch isEnabled value (to restore afterwards, actually not needed, just remember to reset value from config)
      - Set lftHndSwtch isOnDisabled = false
         - setOutputsChange(true); // This modifies the flag and the OtpsChngCnt
      - Set rghtHndSwtch isOnDisabled = false
         - setOutputsChange(true); // This modifies the flag and the OtpsChngCnt
      - Set lftHndSwtch isEnabled = false
         - setOutputsChange(true); // This modifies the flag and the OtpsChngCnt
      - Set rghtHndSwtch isEnabled = false
         - setOutputsChange(true); // This modifies the flag and the OtpsChngCnt
   - New State: stStrtRlsStrtCycl
      - Set **State Change** flag
Out:    
   - N/A
-------------------------------------------------------->>
Brief: stStrtRlsStrtCycl
Meaning: State Start latch release, Start production Cycle
In:
   - Set **Start production cycle** start time
      - _prdCyclTmrStrt = now
   - Set _ltchRlsIsOn = true
      - setOutputsChange(true); // This modifies the flag and the OtpsChngCnt
   - Set _prdCyclIsOn = true
      - setOutputsChange(true); // This modifies the flag and the OtpsChngCnt
   - Default: Clear **State Change** flag
Do:
   - if (Timer == _ltchRlsTtlTm)
      - Reset _ltchRlsIsOn
      - Set OtpsChng flag and increment OtpsChngCnt
      - New State: stEndRls
      - Set **State Change** flag
Out:    
   - N/A
-------------------------------------------------------->>
Brief: stEndRls
Meaning: State End release time (release time reached)
In:

Do:

Out:    

-------------------------------------------------------->>

Brief: stEndCycl
Meaning: State End production cycle (production cycle time reached)
In:

Do:

Out:    

-------------------------------------------------------->>

Brief: stEmrgncyExcpHndl
Meaning: State Emergency Exception signal activated
In:

Do:

Out:    
*/
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	switch(_lsSwtchFdaState){
		case stOffNotBHP:
/*In: 
   - Clean and reset data and flags
   - Default: Clear **State Change** flag
Do:
   - Check if (both hands' isOn condition)
      - New State: stOffBHPNotFP
      - Set **State Change** flag
Out:    
   - set _bthHndsSwArOn = true
   - setOutputsChange(true); // This modifies the flag and the OtpsChngCnt
   - Enable FtMPB
*/
			//In: >>---------------------------------->>
			if(_sttChng){
				clrStatus();
				_clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(getBothHndsSwOk()){            
            _lsSwtchFdaState = stOffBHPNotFP;
				_setSttChng();	//Set flag to execute exiting OUT code
			}
			//Out: >>---------------------------------->>
			if(_sttChng){
            _bthHndsSwArOn = true;
            setOutputsChange(true);
            _undrlFtSSVMPBPtr->enable(); // Enable FtSwitch
         }	// Execute this code only ONCE, when exiting this state
			break;

		case stOffBHPNotFP:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!_bthHndsSwArOn){
            _lsSwtchFdaState = stOffNotBHP;
            _setSttChng();
         }
         else{
            // Check the foot switch release signal ok flag
            if(_ltchRlsIsOn){
               _lsSwtchFdaState = stStrtRlsStrtCycl;
               _setSttChng();
            }
            // If no External Accomplished signal expected, ensure the signal is set to false

         }
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stStrtRlsStrtCycl:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			//_mpbFdaState = stOffNotVPP;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stEndRls:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			//_mpbFdaState = stOffNotVPP;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stEndCycl:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			//_mpbFdaState = stOffNotVPP;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stEmrgncyExcpHndl:
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

void LimbsSftySnglShtSw::_updUndrlSwState(){   
   _lftHndSwtchStts = otptsSttsUnpkg(_undrlLftHndHTVMPBPtr->getOtptsSttsPkgd());
   _rghtHndSwtchStts = otptsSttsUnpkg(_undrlRghtHndHTVMPBPtr->getOtptsSttsPkgd());
   _ftSwtchStts = otptsSttsUnpkg(_undrlFtSSVMPBPtr->getOtptsSttsPkgd());

   return;
}



//=========================================================================> Class methods delimiter

LimbsSftySnglShtSwHI::LimbsSftySnglShtSwHI(){
}

LimbsSftySnglShtSwHI::LimbsSftySnglShtSwHI(gpioPinOtptHwCfg_t ltchRlsActvOtpPin, gpioPinOtptHwCfg_t prdCyclActvOtpPin,
                                        swtchOtptHwCfg_t lftHndOtptcfg, swtchOtptHwCfg_t rghtHndOtptCfg, swtchOtptHwCfg_t ftOtptCfg)
:_ltchRlsActvOtpPin{ltchRlsActvOtpPin}, _prdCyclActvOtpPin{prdCyclActvOtpPin},
_lftHndOtptCfg{lftHndOtptcfg}, _rghtHndOtptCfg{rghtHndOtptCfg}, _ftOtptCfg{ftOtptCfg}                                        
{
      /*Set the output pins to the required states
   Altough is expected after a Turn on/Reset that the MCU starts with all
   it's GPIO pins configured as Open Drain Input, but for security reasons
   the main activation switch output is first set to INPUT "Deactivated"
   It must be expressly set to OUTPUT just after configuration and before use
   */
   pinMode(_ltchRlsActvOtpPin.gpioOtptPin, INPUT);
   digitalWrite(_ltchRlsActvOtpPin.gpioOtptPin, (_ltchRlsActvOtpPin.gpioOtptActHgh)?LOW:HIGH);

   pinMode(_prdCyclActvOtpPin.gpioOtptPin, INPUT);
   digitalWrite(_prdCyclActvOtpPin.gpioOtptPin, (_prdCyclActvOtpPin.gpioOtptActHgh)?LOW:HIGH);

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

   pinMode(_ltchRlsActvOtpPin.gpioOtptPin, OUTPUT); // Setting the main activation output pin to OUTPUT mode
   pinMode(_prdCyclActvOtpPin.gpioOtptPin, OUTPUT); // Setting the main activation output pin to OUTPUT mode

}

LimbsSftySnglShtSwHI::~LimbsSftySnglShtSwHI(){
}

//=========================================================================> Class methods delimiter
