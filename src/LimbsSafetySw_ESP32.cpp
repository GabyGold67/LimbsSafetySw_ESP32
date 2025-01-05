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
  * @date First release: 11/11/2024 
  *       Last update:   05/01/2025 13:55 GMT+0300
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
TaskHandle_t LimbsSftySnglShtSw::lssTskToNtfyOtptsChng = NULL;

LimbsSftySnglShtSw::LimbsSftySnglShtSw()
{
}

LimbsSftySnglShtSw::LimbsSftySnglShtSw(swtchInptHwCfg_t lftHndInpCfg, swtchInptHwCfg_t rghtHndInpCfg, swtchInptHwCfg_t ftInpCfg, lsSwtchSwCfg_t lsSwtchWrkngCnfg)
:_lftHndInpCfg{lftHndInpCfg}, _rghtHndInpCfg{rghtHndInpCfg}, _ftInpCfg{ftInpCfg}
{
   // Build DbncdMPBttn objects and pointers
   _undrlLftHndMPBPtr = new TmVdblMPBttn(_lftHndInpCfg.inptPin, _lftHndSwCfg.swtchVdTm, _lftHndInpCfg.pulledUp, _lftHndInpCfg.typeNO, _lftHndInpCfg.dbncTime, _lftHndSwCfg.swtchStrtDlyTm, true);

   _undrlRghtHndMPBPtr = new TmVdblMPBttn (_rghtHndInpCfg.inptPin, _rghtHndSwCfg.swtchVdTm, _rghtHndInpCfg.pulledUp, _rghtHndInpCfg.typeNO, _rghtHndInpCfg.dbncTime, _rghtHndSwCfg.swtchStrtDlyTm, true);

   _undrlFtMPBPtr = new SnglSrvcVdblMPBttn(_ftInpCfg.inptPin, _ftInpCfg.pulledUp, _ftInpCfg.typeNO, _ftInpCfg.dbncTime, _ftSwCfg.swtchStrtDlyTm);
   
   // Configuration of the isEnabled state of both hands switches. Note: TmVdblMPBttn objects are instantiated with _isEnabled = true property value
   if(!_lftHndSwCfg.swtchIsEnbld)
      _undrlLftHndMPBPtr->disable();
   if(!_rghtHndSwCfg.swtchIsEnbld)
      _undrlRghtHndMPBPtr->disable();

   _ltchRlsTtlTm = lsSwtchWrkngCnfg.ltchRlsActvTm;
   _prdCyclTtlTm = lsSwtchWrkngCnfg.prdCyclActvTm;      
}

LimbsSftySnglShtSw::~LimbsSftySnglShtSw(){
   _undrlFtMPBPtr->~SnglSrvcVdblMPBttn();
   _undrlRghtHndMPBPtr->~TmVdblMPBttn();
   _undrlLftHndMPBPtr->~TmVdblMPBttn();
}

bool LimbsSftySnglShtSw::begin(unsigned long int pollDelayMs){
   bool result {false};
	BaseType_t tmrModResult {pdFAIL};

	if (pollDelayMs >= _undrlSwtchsPollDelay){      
      result = _undrlLftHndMPBPtr->begin(_undrlSwtchsPollDelay);   // Set the underlying left hand MPBttns to start updating it's input readings & output states
      if(result){
         result = _undrlRghtHndMPBPtr->begin(_undrlSwtchsPollDelay);  // Set the underlying right hand MPBttns to start updating it's input readings & output states
         if(result){
            result = _undrlFtMPBPtr->begin(_undrlSwtchsPollDelay); // Set the underlying foot MPBttns to start updating it's input readings & output states
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
   // _bthHndsSwArOn = false;
   _ltchRlsIsOn = false;
   _prdCyclIsOn = false;
   _prdCyclTmrStrt = 0;
   _undrlFtMPBPtr->disable(); // Disable FtSwitch

   return;
}

void LimbsSftySnglShtSw::_clrSttChng(){
   _sttChng = false;

   return;
}

void LimbsSftySnglShtSw::cnfgFtSwtch(const limbSftySwCfg_t &newCfg){
   _undrlFtMPBPtr->setStrtDelay(newCfg.swtchStrtDlyTm);

   return;
}

bool LimbsSftySnglShtSw::_cnfgHndSwtch(const bool &isLeft, const limbSftySwCfg_t &newCfg){
   bool result{false};
   TmVdblMPBttn* hndSwtchToCnf {nullptr};

   if(isLeft)
      hndSwtchToCnf = _undrlLftHndMPBPtr;
   else
      hndSwtchToCnf = _undrlRghtHndMPBPtr;
   hndSwtchToCnf->setStrtDelay(newCfg.swtchStrtDlyTm);
   if(newCfg.swtchIsEnbld)
      hndSwtchToCnf->enable();
   else
      hndSwtchToCnf->disable();
   result = hndSwtchToCnf->setVoidTime(newCfg.swtchVdTm);

   return result;
}

uint32_t LimbsSftySnglShtSw::_lsSwtchOtptsSttsPkgd(uint32_t prevVal){
	if(_lftHndSwtchStts.isOn)
		prevVal |= ((uint32_t)1) << IsOnLftHndPos;
	else
		prevVal &= ~(((uint32_t)1) << IsOnLftHndPos);
      
	if(_rghtHndSwtchStts.isOn)
		prevVal |= ((uint32_t)1) << IsOnRghtHndPos;
	else
		prevVal &= ~(((uint32_t)1) << IsOnRghtHndPos);

	if(_ftSwtchStts.isEnabled)
		prevVal |= ((uint32_t)1) << IsFtSwtchEnbld;
	else
		prevVal &= ~(((uint32_t)1) << IsFtSwtchEnbld);

	if(_ltchRlsIsOn)
		prevVal |= ((uint32_t)1) << IsOnLtchRls;
	else
		prevVal &= ~(((uint32_t)1) << IsOnLtchRls);

	if(_prdCyclIsOn)
		prevVal |= ((uint32_t)1) << IsOnPrdCycl;
	else
		prevVal &= ~(((uint32_t)1) << IsOnPrdCycl);

   return prevVal;
}

bool LimbsSftySnglShtSw::cnfgLftHndSwtch(const limbSftySwCfg_t &newCfg){

   return _cnfgHndSwtch(true, newCfg);
}

bool LimbsSftySnglShtSw::cnfgRghtHndSwtch(const limbSftySwCfg_t &newCfg){

   return _cnfgHndSwtch(false, newCfg);
}

// bool LimbsSftySnglShtSw::getBothHndsSwOk(){
//    _bthHndsSwArOn = ((_undrlLftHndMPBPtr->getIsOn()) && (_undrlRghtHndMPBPtr->getIsOn()));

//    return _bthHndsSwArOn;
// }

fncVdPtrPrmPtrType LimbsSftySnglShtSw::getFnWhnTrnOffLtchRlsPtr(){

   return _fnWhnTrnOffLtchRls;
}

fncVdPtrPrmPtrType LimbsSftySnglShtSw::getFnWhnTrnOffPrdCyclPtr(){

   return _fnWhnTrnOffPrdCycl;
}

fncVdPtrPrmPtrType LimbsSftySnglShtSw::getFnWhnTrnOnLtchRlsPtr(){

   return _fnWhnTrnOnLtchRls;
}

fncVdPtrPrmPtrType LimbsSftySnglShtSw::getFnWhnTrnOnPrdCyclPtr(){

   return _fnWhnTrnOnPrdCycl;
}

SnglSrvcVdblMPBttn* LimbsSftySnglShtSw::getFtSwtchPtr(){
   
   return _undrlFtMPBPtr;
}

TmVdblMPBttn* LimbsSftySnglShtSw::getLftHndSwtchPtr(){

   return _undrlLftHndMPBPtr;
}

const bool LimbsSftySnglShtSw::getlsSwtchOtptsChng() const{

	return _lsSwtchOtptsChng;
}

uint32_t LimbsSftySnglShtSw::getlsSwtchOtptsSttsPkgd(){

   return _lsSwtchOtptsSttsPkgd();
}

const bool LimbsSftySnglShtSw::getLtchRlsIsOn() const{

   return _ltchRlsIsOn;
}

unsigned long int LimbsSftySnglShtSw::getLtchRlsTtlTm(){
   
   return _ltchRlsTtlTm;
}

const bool LimbsSftySnglShtSw::getPrdCyclIsOn() const{

   return _prdCyclIsOn;
}

unsigned long int LimbsSftySnglShtSw::getPrdCyclTtlTm(){
   
   return _prdCyclTtlTm;
}

TmVdblMPBttn* LimbsSftySnglShtSw::getRghtHndSwtchPtr(){

   return _undrlRghtHndMPBPtr;
}

const TaskHandle_t LimbsSftySnglShtSw::getLssTskToNtfyOtptsChng() const{
   
   return lssTskToNtfyOtptsChng;
}

const TaskHandle_t LimbsSftySnglShtSw::getTskToNtfyTrnOffLtchRls() const
{

   return tskToNtfyTrnOffLtchRls;
}

const TaskHandle_t LimbsSftySnglShtSw::getTskToNtfyTrnOffPrdCycl() const{
   
   return tskToNtfyTrnOffPrdCycl;
}

const TaskHandle_t LimbsSftySnglShtSw::getTskToNtfyTrnOnLtchRls() const{
   
   return tskToNtfyTrnOnLtchRls;
}

const TaskHandle_t LimbsSftySnglShtSw::getTskToNtfyTrnOnPrdCycl() const{
   
   return tskToNtfyTrnOnPrdCycl;
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
	if (lsssSwtchObj->getlsSwtchOtptsChng()){
		if(lsssSwtchObj->getLssTskToNtfyOtptsChng() != NULL){
			xTaskNotify(
				lsssSwtchObj->getLssTskToNtfyOtptsChng(),	//TaskHandle_t of the task receiving notification
				static_cast<unsigned long>(lsssSwtchObj->getlsSwtchOtptsSttsPkgd()),
				eSetValueWithOverwrite
			);
			lsssSwtchObj->setlsSwtchOtptsChng(false);
		}
	}     

	return;
}

void LimbsSftySnglShtSw::setFnWhnTrnOffLtchRlsPtr(fncVdPtrPrmPtrType newFnWhnTrnOff){
   _fnWhnTrnOffLtchRls = newFnWhnTrnOff;

   return;
}

void LimbsSftySnglShtSw::setFnWhnTrnOffPrdCyclPtr(fncVdPtrPrmPtrType newFnWhnTrnOff){
   _fnWhnTrnOffPrdCycl = newFnWhnTrnOff;

   return;
}

void LimbsSftySnglShtSw::setFnWhnTrnOnLtchRlsPtr(fncVdPtrPrmPtrType newFnWhnTrnOn){
   _fnWhnTrnOnLtchRls = newFnWhnTrnOn;

   return;
}

void LimbsSftySnglShtSw::setFnWhnTrnOnPrdCyclPtr(fncVdPtrPrmPtrType newFnWhnTrnOn){
   _fnWhnTrnOnPrdCycl = newFnWhnTrnOn;

   return;
}

bool LimbsSftySnglShtSw::setLtchRlsTtlTm(const unsigned long int &newVal){
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

void LimbsSftySnglShtSw::setlsSwtchOtptsChng(bool newlsSwtchOtptsChng){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(newlsSwtchOtptsChng)
		++_lsSwtchOtptsChngCnt;
	else
		if(_lsSwtchOtptsChngCnt)
			--_lsSwtchOtptsChngCnt;
	if(_lsSwtchOtptsChngCnt)
		_lsSwtchOtptsChng = true;
	else
		_lsSwtchOtptsChng = false;

	/*if((_taskToNotifyHndl != NULL) && newOutputsChange)
		_outputsChngTskTrggr = true;*/
	taskEXIT_CRITICAL(&mux);

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

void LimbsSftySnglShtSw::_setSttChng(){
   _sttChng = true;

   return;
}

void LimbsSftySnglShtSw::setLssTskToNtfyOtptsChng(const TaskHandle_t &newTaskHandle){
   if(lssTskToNtfyOtptsChng != newTaskHandle)
      lssTskToNtfyOtptsChng = newTaskHandle;

   return;
}

void LimbsSftySnglShtSw::setTskToNtfyTrnOffLtchRls(const TaskHandle_t &newTaskHandle)
{
   if(tskToNtfyTrnOffLtchRls != newTaskHandle)
      tskToNtfyTrnOffLtchRls = newTaskHandle;

   return;
}

void LimbsSftySnglShtSw::setTskToNtfyTrnOffPrdCycl(const TaskHandle_t &newTaskHandle){
   if(tskToNtfyTrnOffPrdCycl != newTaskHandle)
      tskToNtfyTrnOffPrdCycl = newTaskHandle;

   return;
}

void LimbsSftySnglShtSw::setTskToNtfyTrnOnLtchRls(const TaskHandle_t &newTaskHandle){
   if(tskToNtfyTrnOnLtchRls != newTaskHandle)
      tskToNtfyTrnOnLtchRls = newTaskHandle;

   return;
}

void LimbsSftySnglShtSw::setTskToNtfyTrnOnPrdCycl(const TaskHandle_t &newTaskHandle){
   if(tskToNtfyTrnOnPrdCycl != newTaskHandle)
      tskToNtfyTrnOnPrdCycl = newTaskHandle;

   return;
}

bool LimbsSftySnglShtSw::setUndrlSwtchsPollDelay(const unsigned long int &newVal){
   bool result{false};
   
   if(_minPollDelay <= newVal){
      _undrlSwtchsPollDelay = newVal;
      result = true;
   }

   return result;
}

void LimbsSftySnglShtSw::_turnOffLtchRls(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   
   if(_ltchRlsIsOn){
      //---------------->> Tasks related actions
      /*If the solution was built to unblock a specific task when the Latch release is turned on, this is a model of code to execute, refactor it as needed
      if(getTskToNtfyXXX() != NULL){
         xReturned = xTaskNotify(
            getTskToNtfyXXX(),	//TaskHandle_t of the task receiving notification
            static_cast<unsigned long>(get32_bitsMsgXXX()),
            eSetValueWithOverwrite	//In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }*/
		//---------------->> Functions related actions
		if(_fnWhnTrnOffLtchRls != nullptr){
			_fnWhnTrnOffLtchRls(nullptr);
		}
	   //---------------->> Flags related actions
		taskENTER_CRITICAL(&mux);
      _ltchRlsIsOn = false;
		setlsSwtchOtptsChng(true);
		taskEXIT_CRITICAL(&mux);
	} 

   return;
}

void LimbsSftySnglShtSw::_turnOnLtchRls(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   
   if(!_ltchRlsIsOn){
      //---------------->> Tasks related actions
      /*If the solution was built to unblock a specific task when the Latch release is turned on, this is a model of code to execute, refactor it as needed
      if(getTskToNtfyXXX() != NULL){
         xReturned = xTaskNotify(
            getTskToNtfyXXX(),	//TaskHandle_t of the task receiving notification
            static_cast<unsigned long>(get32_bitsMsgXXX()),
            eSetValueWithOverwrite	//In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }*/
		//---------------->> Functions related actions
		if(_fnWhnTrnOnLtchRls != nullptr){
			_fnWhnTrnOnLtchRls(nullptr);
		}
	   //---------------->> Flags related actions
		taskENTER_CRITICAL(&mux);
      _ltchRlsIsOn = true;
		setlsSwtchOtptsChng(true);
		taskEXIT_CRITICAL(&mux);
	} 

   return;
}

void LimbsSftySnglShtSw::_turnOffPrdCycl(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   
   if(_prdCyclIsOn){
      //---------------->> Tasks related actions
      /*If the solution was built to unblock a specific task when the Latch release is turned on, this is a model of code to execute, refactor it as needed
      if(getTskToNtfyXXX() != NULL){
         xReturned = xTaskNotify(
            getTskToNtfyXXX(),	//TaskHandle_t of the task receiving notification
            static_cast<unsigned long>(get32_bitsMsgXXX()),
            eSetValueWithOverwrite	//In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }*/
		//---------------->> Functions related actions
		if(_fnWhnTrnOffPrdCycl != nullptr){
			_fnWhnTrnOffPrdCycl(nullptr);
		}
	   //---------------->> Flags related actions
		taskENTER_CRITICAL(&mux);
      _prdCyclIsOn = false;
		setlsSwtchOtptsChng(true);
		taskEXIT_CRITICAL(&mux);
	}

   return;
}

void LimbsSftySnglShtSw::_turnOnPrdCycl(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   
   if(!_prdCyclIsOn){
      //---------------->> Tasks related actions
      /*If the solution was built to unblock a specific task when the Latch release is turned on, this is a model of code to execute, refactor it as needed
      if(getTskToNtfyXXX() != NULL){
         xReturned = xTaskNotify(
            getTskToNtfyXXX(),	//TaskHandle_t of the task receiving notification
            static_cast<unsigned long>(get32_bitsMsgXXX()),
            eSetValueWithOverwrite	//In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }*/
		//---------------->> Functions related actions
		if(_fnWhnTrnOnPrdCycl != nullptr){
			_fnWhnTrnOnPrdCycl(nullptr);
		}
	   //---------------->> Flags related actions
		taskENTER_CRITICAL(&mux);
      _prdCyclIsOn = true;
		setlsSwtchOtptsChng(true);
		taskEXIT_CRITICAL(&mux);
	}

   return;
}

// void LimbsSftySnglShtSw::_updBothHndsSwState(){
//    getBothHndsSwOk();

//    return;
// }

unsigned long int LimbsSftySnglShtSw::_updCurTimeMs(){
   _curTimeMs = xTaskGetTickCount() / portTICK_RATE_MS;

   return _curTimeMs;
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
			if(_lftHndSwtchStts.isOn && _rghtHndSwtchStts.isOn){
            _lsSwtchFdaState = stOffBHPNotFP;
				_setSttChng();	//Set flag to execute exiting OUT code
			}
			//Out: >>---------------------------------->>
			if(_sttChng){
            _undrlFtMPBPtr->enable(); // Enable FtSwitch
            setlsSwtchOtptsChng(true);
         }	// Execute this code only ONCE, when exiting this state
			break;

		case stOffBHPNotFP:
			//In: >>---------------------------------->>
			if(_sttChng){_clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!(_lftHndSwtchStts.isOn && _rghtHndSwtchStts.isOn)){
            _undrlFtMPBPtr->disable(); // Disable FtSwitch
            setlsSwtchOtptsChng(true);
            _lsSwtchFdaState = stOffNotBHP;
            _setSttChng();
         }
         else{
            // Check the foot switch release signal ok flag
            if(_ftSwtchStts.isOn){
               _undrlLftHndMPBPtr->setIsOnDisabled(false);
               _undrlLftHndMPBPtr->disable();
               setlsSwtchOtptsChng(true);
               _undrlRghtHndMPBPtr->setIsOnDisabled(false);
               _undrlRghtHndMPBPtr->disable();
               setlsSwtchOtptsChng(true);
               _undrlFtMPBPtr->disable();
               setlsSwtchOtptsChng(true);
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
         _turnOnPrdCycl();
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
            // Restore modified isOnDisabled, isEnabled for the underlying switches
            _undrlLftHndMPBPtr->setIsOnDisabled(true);
            if(_lftHndSwCfg.swtchIsEnbld){
               _undrlLftHndMPBPtr->enable();
               setlsSwtchOtptsChng(true);        
            }
            _undrlRghtHndMPBPtr->setIsOnDisabled(true);
            if(_rghtHndSwCfg.swtchIsEnbld){
               _undrlRghtHndMPBPtr->enable();
               setlsSwtchOtptsChng(true);        
            }
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
   _lftHndSwtchStts = otptsSttsUnpkg(_undrlLftHndMPBPtr->getOtptsSttsPkgd());
   _rghtHndSwtchStts = otptsSttsUnpkg(_undrlRghtHndMPBPtr->getOtptsSttsPkgd());
   _ftSwtchStts = otptsSttsUnpkg(_undrlFtMPBPtr->getOtptsSttsPkgd());

   return;
}

//=========================================================================> Class methods delimiter
