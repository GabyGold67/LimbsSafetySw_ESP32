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
   _undrlLftHndHTVMPBPtr = new TmVdblMPBttn(_lftHndInpCfg.inptPin, _lftHndSwCfg.swtchVdTm, _lftHndInpCfg.pulledUp, _lftHndInpCfg.typeNO, _lftHndInpCfg.dbncTime, _lftHndSwCfg.swtchStrtDlyTm, true);

   _undrlRghtHndHTVMPBPtr = new TmVdblMPBttn (_rghtHndInpCfg.inptPin, _rghtHndSwCfg.swtchVdTm, _rghtHndInpCfg.pulledUp, _rghtHndInpCfg.typeNO, _rghtHndInpCfg.dbncTime, _rghtHndSwCfg.swtchStrtDlyTm, true);

   _undrlFtSSVMPBPtr = new SnglSrvcVdblMPBttn(_ftInpCfg.inptPin, _ftInpCfg.pulledUp, _ftInpCfg.typeNO, _ftInpCfg.dbncTime, _ftSwCfg.swtchStrtDlyTm);
   
   // Configuration of the isEnabled state of both hands switches. Note: TmVdblMPBttn objects are instantiated with _isEnabled = true property value
   if(!_lftHndSwCfg.swtchIsEnbld)
      _undrlLftHndHTVMPBPtr->disable();
   if(!_rghtHndSwCfg.swtchIsEnbld)
      _undrlRghtHndHTVMPBPtr->disable();
      
}

LimbsSftySnglShtSw::~LimbsSftySnglShtSw()
{
}

bool LimbsSftySnglShtSw::begin(unsigned long int pollDelayMs){
   bool result {false};
	BaseType_t tmrModResult {pdFAIL};

	if (pollDelayMs > _undrlSwtchsPollDelay){      
      result = _undrlLftHndHTVMPBPtr->begin(_undrlSwtchsPollDelay);   // Set the underlying left hand MPBttns to start updating it's input readings & output states
      if(result){
         result = _undrlRghtHndHTVMPBPtr->begin(_undrlSwtchsPollDelay);  // Set the underlying right hand MPBttns to start updating it's input readings & output states
         if(result){
            result = _undrlFtSSVMPBPtr->begin(_undrlSwtchsPollDelay); // Set the underlying foot MPBttns to start updating it's input readings & output states
            if(result){
               if (!_lsSwtchPollTmrHndl){        
                  _lsSwtchPollTmrHndl = xTimerCreate(
                     _swtchPollTmrName.c_str(),  //Timer name
                     pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
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
   _ltchRlsIsOn = false;
   _prdCyclIsOn = false;
   _prdCyclTmrStrt = 0;

   return;
}

void LimbsSftySnglShtSw::_clrSttChng(){
   _sttChng = false;

   return;
}

void LimbsSftySnglShtSw::cnfgFtSwtch(const limbSftySwCfg_t &newCfg){
   _undrlFtSSVMPBPtr->setStrtDelay(newCfg.swtchStrtDlyTm);

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
   result = hndSwtchToCnf->setVoidTime(newCfg.swtchVdTm);

   return result;
}

bool LimbsSftySnglShtSw::cnfgLftHndSwtch(const limbSftySwCfg_t &newCfg){

   return _cnfgHndSwtch(true, newCfg);
}

bool LimbsSftySnglShtSw::cnfgRghtHndSwtch(const limbSftySwCfg_t &newCfg){

   return _cnfgHndSwtch(false, newCfg);
}

bool LimbsSftySnglShtSw::getBothHndsSwOk(){
   _bthHndsSwArOn = ((_undrlLftHndHTVMPBPtr->getIsOn()) && (_undrlRghtHndHTVMPBPtr->getIsOn()));

   return _bthHndsSwArOn;
}

const bool LimbsSftySnglShtSw::getLtchRlsIsOn() const{

   return _ltchRlsIsOn;
}

const bool LimbsSftySnglShtSw::getOutputsChange() const{

	return _outputsChange;
}

LimbsSftySnglShtSwHI* LimbsSftySnglShtSw::getPnOtHWUpdtrPtr(){

   return _pnOtHWUpdtrPtr;
}

const bool LimbsSftySnglShtSw::getPrdCyclIsOn() const{
   return _prdCyclIsOn;
}

void LimbsSftySnglShtSw::lsSwtchPollCb(TimerHandle_t lssTmrCbArg){
   LimbsSftySnglShtSw* lsssSwtchObj = (LimbsSftySnglShtSw*)pvTimerGetTimerID(lssTmrCbArg);
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
   // Underlying switches status recovery
   lsssSwtchObj->_updUndrlSwState();
   //------------
   // Flags, Triggers and timers calculation & update
 	lsssSwtchObj->_updCurTimeMs();
   //------------
	// State machine update
 	lsssSwtchObj->_updFdaState();
 	taskEXIT_CRITICAL(&mux);

	//Outputs update, function and tasks executions based on outputs changed generated by the State Machine
	if (lsssSwtchObj->getOutputsChange()){
      if(lsssSwtchObj->getPnOtHWUpdtrPtr() != nullptr){

      }
		/*if(mpbObj->getTaskToNotify() != NULL){
			xTaskNotify(
				mpbObj->getTaskToNotify(),	//TaskHandle_t of the task receiving notification
				static_cast<unsigned long>(mpbObj->getOtptsSttsPkgd()),
				eSetValueWithOverwrite
			);
			mpbObj->setOutputsChange(false);
		}*/
	}     

	return;
}

bool LimbsSftySnglShtSw::setLtchRlsTm(const unsigned long int &newVal){
   bool result{true};

   if (_ltchRlsTtlTm != newVal){
      if((newVal > 0) && (newVal <= _prdCyclTtlTm)){
         _ltchRlsTtlTm = newVal;
      }
      else{
         result = false;
      }
   }

   return result;
}

void LimbsSftySnglShtSw::setOutputsChange(bool newOutputsChange){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(newOutputsChange)
		++_outputsChangeCnt;
	else
		if(_outputsChangeCnt)
			--_outputsChangeCnt;
	if(_outputsChangeCnt)
		_outputsChange = true;
	else
		_outputsChange = false;

	/*if((_taskToNotifyHndl != NULL) && newOutputsChange)
		_outputsChngTskTrggr = true;*/
	taskEXIT_CRITICAL(&mux);

   return;
}

void LimbsSftySnglShtSw::setPnOtHWUpdtr(LimbsSftySnglShtSwHI* &newVal){
   _pnOtHWUpdtrPtr = newVal;

   return;
}

bool LimbsSftySnglShtSw::setPrdCyclTtlTm(const unsigned long int &newVal){
   bool result{true};

   if(_prdCyclTtlTm != newVal){
      if((newVal > 0) && (newVal >= _ltchRlsTtlTm)){
         _prdCyclTtlTm = newVal;
      }
      else{
         result = false;
      }
   }
   
   return result;
}

bool LimbsSftySnglShtSw::setUndrlSwtchPollDelay(const unsigned long int &newVal){
   bool result{false};
   
   if(_minPollDelay <= newVal){
      _undrlSwtchsPollDelay = newVal;
      result = true;
   }

   return result;
}

void LimbsSftySnglShtSw::_setSttChng(){
   _sttChng = true;

   return;
}

void LimbsSftySnglShtSw::_turnOffLtchRls(){
   _ltchRlsIsOn = false;

   return;
}

void LimbsSftySnglShtSw::_turnOnLtchRls(){
   _ltchRlsIsOn = true;

   return;
}

void LimbsSftySnglShtSw::_turnOffPrdCycl(){
   _prdCyclIsOn = false;

   return;

}

void LimbsSftySnglShtSw::_turnOnPrdCycl(){
   _prdCyclIsOn = true;

   return;
}

void LimbsSftySnglShtSw::_updBothHndsSwState(){
   getBothHndsSwOk();

   return;
}

void LimbsSftySnglShtSw::_updCurTimeMs(){
   _curTimeMs = xTaskGetTickCount() / portTICK_RATE_MS;

   return;
}

void LimbsSftySnglShtSw::_updFdaState(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	switch(_lsSwtchFdaState){
		case stOffNotBHP:
			//In: >>---------------------------------->>
			if(_sttChng){
				clrStatus();
				_clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>         
			if(getBothHndsSwOk()){            
            setOutputsChange(true);
            _lsSwtchFdaState = stOffBHPNotFP;
				_setSttChng();	//Set flag to execute exiting OUT code
			}
			//Out: >>---------------------------------->>
			if(_sttChng){
            _undrlFtSSVMPBPtr->enable(); // Enable FtSwitch
         }	// Execute this code only ONCE, when exiting this state
			break;

		case stOffBHPNotFP:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!(_lftHndSwtchStts.isOn)&&(_rghtHndSwtchStts.isOn)){
            _undrlFtSSVMPBPtr->disable(); // Disable FtSwitch
            _bthHndsSwArOn = false;
            setOutputsChange(true);
            _lsSwtchFdaState = stOffNotBHP;
            _setSttChng();
         }
         else{
            // Check the foot switch release signal ok flag
            if(_ftSwtchStts.isOn){
               _undrlLftHndHTVMPBPtr->setIsOnDisabled(false);
               _undrlLftHndHTVMPBPtr->disable();
               _undrlRghtHndHTVMPBPtr->setIsOnDisabled(false);
               _undrlRghtHndHTVMPBPtr->disable();
               _undrlFtSSVMPBPtr->disable();
               _bthHndsSwArOn = false;
               setOutputsChange(true);
               _lsSwtchFdaState = stStrtRlsStrtCycl;
               _setSttChng();
            }
         }
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stStrtRlsStrtCycl:
			//In: >>---------------------------------->>
			if(_sttChng){
            _prdCyclTmrStrt= _curTimeMs;
            _clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_turnOnLtchRls();
         setOutputsChange(true);
         _turnOnPrdCycl();
         setOutputsChange(true);         
         _lsSwtchFdaState = stEndRls;
			_setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
//!			break;	// This state makes no conditional next state setting, and it's next state is next in line, let it cascade

		case stEndRls:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
         if((_curTimeMs - _prdCyclTmrStrt) >= _ltchRlsTtlTm){
            _turnOffLtchRls();
            setOutputsChange(true);
            //todo: Execute flagging function (to be short) for release closure
            //todo: Release parallel independent task for release closure
            _lsSwtchFdaState = stEndCycl;
			   _setSttChng();
         }
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stEndCycl:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
         if((_curTimeMs - _prdCyclTmrStrt) >= _prdCyclTtlTm){
            _turnOffPrdCycl();
            setOutputsChange(true);        
            // Restore modified isOnDisabled, isEnabled for the underlying switches
            _undrlLftHndHTVMPBPtr->setIsOnDisabled(true);
            if(_lftHndSwCfg.swtchIsEnbld)
               _undrlLftHndHTVMPBPtr->enable();
            setOutputsChange(true);        
            _undrlRghtHndHTVMPBPtr->setIsOnDisabled(true);
            if(!_rghtHndSwCfg.swtchIsEnbld)
               _undrlRghtHndHTVMPBPtr->enable();
            setOutputsChange(true);        
            //todo: Execute flagging function (to be short) for cycle closure
            //todo: Release parallel independent task for cycle closure
            _lsSwtchFdaState = stOffNotBHP;
            _setSttChng();
         }
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stEmrgncyExcpHndl:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
         // Keep ethernal loop
			//_setSttChng();
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

    return true;
}

void LimbsSftySnglShtSw::_updUndrlSwState(){   
   _lftHndSwtchStts = otptsSttsUnpkg(_undrlLftHndHTVMPBPtr->getOtptsSttsPkgd());
   _rghtHndSwtchStts = otptsSttsUnpkg(_undrlRghtHndHTVMPBPtr->getOtptsSttsPkgd());
   _ftSwtchStts = otptsSttsUnpkg(_undrlFtSSVMPBPtr->getOtptsSttsPkgd());

   return;
}

//=========================================================================> Class methods delimiter

LimbsSftySnglShtSwHI::LimbsSftySnglShtSwHI(){
}

LimbsSftySnglShtSwHI::LimbsSftySnglShtSwHI(gpioPinOtptHwCfg_t hndsSwtchsOkPin, gpioPinOtptHwCfg_t ltchRlsActvOtpPin, gpioPinOtptHwCfg_t prdCyclActvOtpPin,
                                          swtchOtptHwCfg_t lftHndOtptcfg, swtchOtptHwCfg_t rghtHndOtptCfg, swtchOtptHwCfg_t ftOtptCfg)
:_hndsSwtchsOkPin{hndsSwtchsOkPin}, _ltchRlsActvOtpPin{ltchRlsActvOtpPin}, _prdCyclActvOtpPin{prdCyclActvOtpPin},
_lftHndOtptCfg{lftHndOtptcfg}, _rghtHndOtptCfg{rghtHndOtptCfg}, _ftOtptCfg{ftOtptCfg}                                        
{
      /*Set the output pins to the required states
   Altough is expected after a Turn on/Reset that the MCU starts with all
   it's GPIO pins configured as Open Drain Input, but for security reasons
   the main activation switch output is first set to INPUT "Deactivated"
   It must be expressly set to OUTPUT just after configuration and before use
   */
   //! The _ltchRlsActvOtpPin is the ONLY FUNDAMENTAL pin that is REQUIRED to be a valid available pin number. All the rest are for information provision
   pinMode(_ltchRlsActvOtpPin.gpioOtptPin, INPUT);
   digitalWrite(_ltchRlsActvOtpPin.gpioOtptPin, (_ltchRlsActvOtpPin.gpioOtptActHgh)?LOW:HIGH);

   //The rest of te possible connected pins are configured as OUTPUTS, setting it's starting values as LOW/Reset
   if(_prdCyclActvOtpPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_prdCyclActvOtpPin.gpioOtptPin, (_prdCyclActvOtpPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(_prdCyclActvOtpPin.gpioOtptPin, OUTPUT);
   }

   if(_hndsSwtchsOkPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_hndsSwtchsOkPin.gpioOtptPin, (_hndsSwtchsOkPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(_hndsSwtchsOkPin.gpioOtptPin, OUTPUT);
   }

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

}

LimbsSftySnglShtSwHI::~LimbsSftySnglShtSwHI(){
}

void LimbsSftySnglShtSwHI::updOutputs(MpbOtpts_t lftHndSwtchStts,
               MpbOtpts_t rghtHndSwtchStts,
               MpbOtpts_t ftSwtchStts,
               lsSwtchOtpts_t lsSwtchStts){

/*struct lsSwtchOtpts_t{
   bool bthHndsSwArOn;
   bool ltchRlsIsOn;
   bool prdCyclIsOn;
};
*/
   digitalWrite(_ltchRlsActvOtpPin.gpioOtptPin, (_ltchRlsActvOtpPin.gpioOtptActHgh)?LOW:HIGH);

   //The rest of te possible connected pins are configured as OUTPUTS, setting it's starting values as LOW/Reset
   if(_prdCyclActvOtpPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_prdCyclActvOtpPin.gpioOtptPin, (_prdCyclActvOtpPin.gpioOtptActHgh)?LOW:HIGH);
      // digitalWrite(_prdCyclActvOtpPin.gpioOtptPin, (_prdCyclActvOtpPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(_prdCyclActvOtpPin.gpioOtptPin, OUTPUT);
   }

   if(_hndsSwtchsOkPin.gpioOtptPin != _InvalidPinNum){
      digitalWrite(_hndsSwtchsOkPin.gpioOtptPin, (_hndsSwtchsOkPin.gpioOtptActHgh)?LOW:HIGH);
      pinMode(_hndsSwtchsOkPin.gpioOtptPin, OUTPUT);
   }

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
   return;

}

//=========================================================================> Class methods delimiter
