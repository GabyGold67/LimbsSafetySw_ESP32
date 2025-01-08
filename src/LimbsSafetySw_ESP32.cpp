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
  *       Last update:   08/01/2025 11:30 (GMT+0300 DST)
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
LimbsSftyLnFSwtch::LimbsSftyLnFSwtch()
{
}

LimbsSftyLnFSwtch::LimbsSftyLnFSwtch(swtchInptHwCfg_t lftHndInpCfg, swtchInptHwCfg_t rghtHndInpCfg, swtchInptHwCfg_t ftInpCfg, lsSwtchSwCfg_t lsSwtchWrkngCnfg)
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

LimbsSftyLnFSwtch::~LimbsSftyLnFSwtch(){
   _undrlFtMPBPtr->~SnglSrvcVdblMPBttn();
   _undrlRghtHndMPBPtr->~TmVdblMPBttn();
   _undrlLftHndMPBPtr->~TmVdblMPBttn();
}

bool LimbsSftyLnFSwtch::begin(unsigned long int pollDelayMs){
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

void LimbsSftyLnFSwtch::clrStatus(){
   _ltchRlsIsOn = false;
   _prdCyclIsOn = false;
   _prdCyclTmrStrt = 0;
   _undrlFtMPBPtr->disable(); // Disable FtSwitch

   return;
}

void LimbsSftyLnFSwtch::_clrSttChng(){
   _sttChng = false;

   return;
}

void LimbsSftyLnFSwtch::cnfgFtSwtch(const limbSftySwCfg_t &newCfg){
   _undrlFtMPBPtr->setStrtDelay(newCfg.swtchStrtDlyTm);

   return;
}

bool LimbsSftyLnFSwtch::_cnfgHndSwtch(const bool &isLeft, const limbSftySwCfg_t &newCfg){
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

bool LimbsSftyLnFSwtch::cnfgLftHndSwtch(const limbSftySwCfg_t &newCfg){

   return _cnfgHndSwtch(true, newCfg);
}

bool LimbsSftyLnFSwtch::cnfgRghtHndSwtch(const limbSftySwCfg_t &newCfg){

   return _cnfgHndSwtch(false, newCfg);
}

fncVdPtrPrmPtrType LimbsSftyLnFSwtch::getFnWhnTrnOffLtchRlsPtr(){

   return _fnWhnTrnOffLtchRls;
}

fncVdPtrPrmPtrType LimbsSftyLnFSwtch::getFnWhnTrnOffPrdCyclPtr(){

   return _fnWhnTrnOffPrdCycl;
}

fncVdPtrPrmPtrType LimbsSftyLnFSwtch::getFnWhnTrnOnLtchRlsPtr(){

   return _fnWhnTrnOnLtchRls;
}

fncVdPtrPrmPtrType LimbsSftyLnFSwtch::getFnWhnTrnOnPrdCyclPtr(){

   return _fnWhnTrnOnPrdCycl;
}

SnglSrvcVdblMPBttn* LimbsSftyLnFSwtch::getFtSwtchPtr(){
   
   return _undrlFtMPBPtr;
}

TmVdblMPBttn* LimbsSftyLnFSwtch::getLftHndSwtchPtr(){

   return _undrlLftHndMPBPtr;
}

const bool LimbsSftyLnFSwtch::getlsSwtchOtptsChng() const{

	return _lsSwtchOtptsChng;
}

uint32_t LimbsSftyLnFSwtch::getLsSwtchOtptsSttsPkgd(){

   return _lsSwtchOtptsSttsPkgd();
}

const bool LimbsSftyLnFSwtch::getLtchRlsIsOn() const{

   return _ltchRlsIsOn;
}

unsigned long int LimbsSftyLnFSwtch::getLtchRlsTtlTm(){
   
   return _ltchRlsTtlTm;
}

const bool LimbsSftyLnFSwtch::getPrdCyclIsOn() const{

   return _prdCyclIsOn;
}

unsigned long int LimbsSftyLnFSwtch::getPrdCyclTtlTm(){
   
   return _prdCyclTtlTm;
}

TmVdblMPBttn* LimbsSftyLnFSwtch::getRghtHndSwtchPtr(){

   return _undrlRghtHndMPBPtr;
}

const TaskHandle_t LimbsSftyLnFSwtch::getLssTskToNtfyOtptsChng() const{
   
   return _lssTskToNtfyOtptsChng;
}

const TaskHandle_t LimbsSftyLnFSwtch::getTskToNtfyTrnOffLtchRls() const{

   return _tskToNtfyTrnOffLtchRls;
}

const TaskHandle_t LimbsSftyLnFSwtch::getTskToNtfyTrnOffPrdCycl() const{
   
   return _tskToNtfyTrnOffPrdCycl;
}

const TaskHandle_t LimbsSftyLnFSwtch::getTskToNtfyTrnOnLtchRls() const{
   
   return _tskToNtfyTrnOnLtchRls;
}

const TaskHandle_t LimbsSftyLnFSwtch::getTskToNtfyTrnOnPrdCycl() const{
   
   return _tskToNtfyTrnOnPrdCycl;
}

uint32_t LimbsSftyLnFSwtch::_lsSwtchOtptsSttsPkgd(uint32_t prevVal){
	if(_ftSwtchStts.isEnabled)
		prevVal |= ((uint32_t)1) << IsFtSwtchEnbldBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsFtSwtchEnbldBitPos);

	if(_ltchRlsIsOn)
		prevVal |= ((uint32_t)1) << IsOnLtchRlsBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsOnLtchRlsBitPos);

	if(_prdCyclIsOn)
		prevVal |= ((uint32_t)1) << IsOnPrdCyclBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsOnPrdCyclBitPos);

   // Attribute flags from the underlying MPBttns, included for practicity, might be replaced by other values in future development iterations.
	if(_lftHndSwtchStts.isEnabled)
		prevVal |= ((uint32_t)1) << IsEnbldLftHndBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsEnbldLftHndBitPos);

   if(_lftHndSwtchStts.isOn)
		prevVal |= ((uint32_t)1) << IsOnLftHndBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsOnLftHndBitPos);
      
	if(_rghtHndSwtchStts.isEnabled)
		prevVal |= ((uint32_t)1) << IsEnbldRghtHndBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsEnbldRghtHndBitPos);

	if(_rghtHndSwtchStts.isOn)
		prevVal |= ((uint32_t)1) << IsOnRghtHndBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsOnRghtHndBitPos);

   return prevVal;
}

void LimbsSftyLnFSwtch::lsSwtchPollCb(TimerHandle_t lssTmrCbArg){
   LimbsSftyLnFSwtch* lsssSwtchObj = (LimbsSftyLnFSwtch*)pvTimerGetTimerID(lssTmrCbArg);
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
   // Underlying switches status recovery
   lsssSwtchObj->_getUndrlSwtchStts();
   //------------
   // Flags, Triggers and timers calculation & update
 	lsssSwtchObj->_updCurTimeMs();
   //------------
	// State machine update
 	lsssSwtchObj->_updFdaState();
 	taskEXIT_CRITICAL(&mux);

	//Outputs update, function and tasks executions based on outputs changed generated by the State Machine
      //---------------->> Tasks related actions
      //---------------->> Generic Task for output changes related actions
	if (lsssSwtchObj->getlsSwtchOtptsChng()){
		if(lsssSwtchObj->getLssTskToNtfyOtptsChng() != NULL){
			xReturned = xTaskNotify(
				lsssSwtchObj->getLssTskToNtfyOtptsChng(),	//TaskHandle_t of the task receiving notification
				static_cast<uint32_t>(lsssSwtchObj->getLsSwtchOtptsSttsPkgd()),
				eSetValueWithOverwrite
			);
			lsssSwtchObj->setlsSwtchOtptsChng(false);
		}
	}     

	return;
}

void LimbsSftyLnFSwtch::setFnWhnTrnOffLtchRlsPtr(fncVdPtrPrmPtrType newFnWhnTrnOff){
   if(_fnWhnTrnOffLtchRls != newFnWhnTrnOff)
      _fnWhnTrnOffLtchRls = newFnWhnTrnOff;

   return;
}

void LimbsSftyLnFSwtch::setFnWhnTrnOffPrdCyclPtr(fncVdPtrPrmPtrType newFnWhnTrnOff){
   if(_fnWhnTrnOffPrdCycl != newFnWhnTrnOff)
      _fnWhnTrnOffPrdCycl = newFnWhnTrnOff;

   return;
}

void LimbsSftyLnFSwtch::setFnWhnTrnOnLtchRlsPtr(fncVdPtrPrmPtrType newFnWhnTrnOn){
   if(_fnWhnTrnOnLtchRls != newFnWhnTrnOn)
      _fnWhnTrnOnLtchRls = newFnWhnTrnOn;

   return;
}

void LimbsSftyLnFSwtch::setFnWhnTrnOnPrdCyclPtr(fncVdPtrPrmPtrType newFnWhnTrnOn){
   if(_fnWhnTrnOnPrdCycl != newFnWhnTrnOn)
      _fnWhnTrnOnPrdCycl = newFnWhnTrnOn;

   return;
}

bool LimbsSftyLnFSwtch::setLtchRlsTtlTm(const unsigned long int &newVal){
   bool result{true};

   if (_ltchRlsTtlTm != newVal){
      if((newVal > 0) && (newVal <= _prdCyclTtlTm))
         _ltchRlsTtlTm = newVal;
      else
         result = false;
   }

   return result;
}

void LimbsSftyLnFSwtch::setlsSwtchOtptsChng(bool newlsSwtchOtptsChng){
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

bool LimbsSftyLnFSwtch::setPrdCyclTtlTm(const unsigned long int &newVal){
   bool result{true};

   if(_prdCyclTtlTm != newVal){
      if((newVal > 0) && (newVal >= _ltchRlsTtlTm))
         _prdCyclTtlTm = newVal;
      else
         result = false;
   }
   
   return result;
}

void LimbsSftyLnFSwtch::_setSttChng(){
   _sttChng = true;

   return;
}

void LimbsSftyLnFSwtch::setLssTskToNtfyOtptsChng(const TaskHandle_t &newTaskHandle){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	eTaskState taskToNtfyStt{};

   taskENTER_CRITICAL(&mux);
   if(_lssTskToNtfyOtptsChng != newTaskHandle){
      if(_lssTskToNtfyOtptsChng != NULL){
         taskToNtfyStt = eTaskGetState(_lssTskToNtfyOtptsChng);
         if (taskToNtfyStt != eSuspended){
            if(taskToNtfyStt != eDeleted){
               vTaskSuspend(_lssTskToNtfyOtptsChng);
               _lssTskToNtfyOtptsChng = NULL;
            }
         }
      }
      if (newTaskHandle != NULL)
         _lssTskToNtfyOtptsChng = newTaskHandle;
   }
   taskEXIT_CRITICAL(&mux);

	return;
}

void LimbsSftyLnFSwtch::setTskToNtfyTrnOffLtchRls(const TaskHandle_t &newTaskHandle){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	eTaskState taskToNtfyStt{};

   taskENTER_CRITICAL(&mux);
   if(_tskToNtfyTrnOffLtchRls != newTaskHandle){
      if(_tskToNtfyTrnOffLtchRls != NULL){
         taskToNtfyStt = eTaskGetState(_tskToNtfyTrnOffLtchRls);
         if (taskToNtfyStt != eSuspended){
            if(taskToNtfyStt != eDeleted){
               vTaskSuspend(_tskToNtfyTrnOffLtchRls);
               _tskToNtfyTrnOffLtchRls = NULL;
            }
         }
      }
      if (newTaskHandle != NULL)
         _tskToNtfyTrnOffLtchRls = newTaskHandle;
   }
   taskEXIT_CRITICAL(&mux);

	return;
}

void LimbsSftyLnFSwtch::setTskToNtfyTrnOffPrdCycl(const TaskHandle_t &newTaskHandle){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	eTaskState taskToNtfyStt{};

   taskENTER_CRITICAL(&mux);
   if(_tskToNtfyTrnOffPrdCycl != newTaskHandle){
      if(_tskToNtfyTrnOffPrdCycl != NULL){
         taskToNtfyStt = eTaskGetState(_tskToNtfyTrnOffPrdCycl);
         if (taskToNtfyStt != eSuspended){
            if(taskToNtfyStt != eDeleted){
               vTaskSuspend(_tskToNtfyTrnOffPrdCycl);
               _tskToNtfyTrnOffPrdCycl = NULL;
            }
         }
      }
      if (newTaskHandle != NULL)
         _tskToNtfyTrnOffPrdCycl = newTaskHandle;
   }
   taskEXIT_CRITICAL(&mux);

	return;
}

void LimbsSftyLnFSwtch::setTskToNtfyTrnOnLtchRls(const TaskHandle_t &newTaskHandle){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	eTaskState taskToNtfyStt{};

   taskENTER_CRITICAL(&mux);
   if(_tskToNtfyTrnOnLtchRls != newTaskHandle){
      if(_tskToNtfyTrnOnLtchRls != NULL){
         taskToNtfyStt = eTaskGetState(_tskToNtfyTrnOnLtchRls);
         if (taskToNtfyStt != eSuspended){
            if(taskToNtfyStt != eDeleted){
               vTaskSuspend(_tskToNtfyTrnOnLtchRls);
               _tskToNtfyTrnOnLtchRls = NULL;
            }
         }
      }
      if (newTaskHandle != NULL)
         _tskToNtfyTrnOnLtchRls = newTaskHandle;
   }
   taskEXIT_CRITICAL(&mux);

	return;
}

void LimbsSftyLnFSwtch::setTskToNtfyTrnOnPrdCycl(const TaskHandle_t &newTaskHandle){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	eTaskState taskToNtfyStt{};

   taskENTER_CRITICAL(&mux);
   if(_tskToNtfyTrnOnPrdCycl != newTaskHandle){
      if(_tskToNtfyTrnOnPrdCycl != NULL){
         taskToNtfyStt = eTaskGetState(_tskToNtfyTrnOnPrdCycl);
         if (taskToNtfyStt != eSuspended){
            if(taskToNtfyStt != eDeleted){
               vTaskSuspend(_tskToNtfyTrnOnPrdCycl);
               _tskToNtfyTrnOnPrdCycl = NULL;
            }
         }
      }
      if (newTaskHandle != NULL)
         _tskToNtfyTrnOnPrdCycl = newTaskHandle;
   }
   taskEXIT_CRITICAL(&mux);

	return;
}

bool LimbsSftyLnFSwtch::setUndrlSwtchsPollDelay(const unsigned long int &newVal){
   bool result{false};
   
   if(_minPollDelay <= newVal){
      _undrlSwtchsPollDelay = newVal;
      result = true;
   }

   return result;
}

void LimbsSftyLnFSwtch::_turnOffLtchRls(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   
   if(_ltchRlsIsOn){
      //---------------->> Tasks related actions
      //---------------->> Specific Task for Latch Release turned off related actions
      if(getTskToNtfyTrnOffLtchRls() != NULL){
         xReturned = xTaskNotify(
            getTskToNtfyTrnOffLtchRls(),	//TaskHandle_t of the task receiving notification
            static_cast<uint32_t>(0x00),
            eSetValueWithOverwrite	//In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }
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

void LimbsSftyLnFSwtch::_turnOnLtchRls(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   
   if(!_ltchRlsIsOn){
      //---------------->> Tasks related actions
      //---------------->> Specific Task for Latch Release turned off related actions
      if(getTskToNtfyTrnOnLtchRls() != NULL){
         xReturned = xTaskNotify(
            getTskToNtfyTrnOnLtchRls(),	//TaskHandle_t of the task receiving notification
            static_cast<uint32_t>(0x00),
            eSetValueWithOverwrite	//In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }
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

void LimbsSftyLnFSwtch::_turnOffPrdCycl(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   
   if(_prdCyclIsOn){
      //---------------->> Tasks related actions
      //---------------->> Specific Task for Latch Release turned off related actions
      if(getTskToNtfyTrnOffPrdCycl() != NULL){
         xReturned = xTaskNotify(
            getTskToNtfyTrnOffPrdCycl(),	//TaskHandle_t of the task receiving notification
            static_cast<uint32_t>(0x00),
            eSetValueWithOverwrite	//In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }
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

void LimbsSftyLnFSwtch::_turnOnPrdCycl(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   
   if(!_prdCyclIsOn){
      //---------------->> Tasks related actions
      //---------------->> Specific Task for Latch Release turned off related actions
      if(getTskToNtfyTrnOnPrdCycl() != NULL){
         xReturned = xTaskNotify(
            getTskToNtfyTrnOnPrdCycl(),	//TaskHandle_t of the task receiving notification
            static_cast<uint32_t>(0x00),
            eSetValueWithOverwrite	//In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }
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

unsigned long int LimbsSftyLnFSwtch::_updCurTimeMs(){
   _curTimeMs = xTaskGetTickCount() / portTICK_RATE_MS;

   return _curTimeMs;
}

void LimbsSftyLnFSwtch::_updFdaState(){
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

void LimbsSftyLnFSwtch::_getUndrlSwtchStts(){   
   _lftHndSwtchStts = otptsSttsUnpkg(_undrlLftHndMPBPtr->getOtptsSttsPkgd());
   _rghtHndSwtchStts = otptsSttsUnpkg(_undrlRghtHndMPBPtr->getOtptsSttsPkgd());
   _ftSwtchStts = otptsSttsUnpkg(_undrlFtMPBPtr->getOtptsSttsPkgd());

   return;
}

//=========================================================================> Class methods delimiter

/**
 * @brief Unpackages a 32-bit value into a LimbsSftyLnFSwtch object status
 * 
 * The 32-bit encoded and packaged is used for inter-task object status comunication, passed as a "notification value" in a xTaskNotify() execution.
 * For each bit value attribute flag represented see LimbsSftyLnFSwtch::getLsSwtchOtptsSttsPkgd()
 * 
 * @param pkgOtpts A 32-bit value holding a LimbsSftyLnFSwtch status encoded
 * @return A lsSwtchOtpts_t type element containing the information decoded
 */
lsSwtchOtpts_t lssOtptsSttsUnpkg(uint32_t pkgOtpts){
	lsSwtchOtpts_t lssCurSttsDcdd {0};

	if(pkgOtpts & (((uint32_t)1) << IsFtSwtchEnbldBitPos))
		lssCurSttsDcdd.ftSwIsEnbld = true;
	else
		lssCurSttsDcdd.ftSwIsEnbld = false;

	if(pkgOtpts & (((uint32_t)1) << IsOnLtchRlsBitPos))
		lssCurSttsDcdd.ltchRlsIsOn = true;
	else
		lssCurSttsDcdd.ltchRlsIsOn = false;

	if(pkgOtpts & (((uint32_t)1) << IsOnPrdCyclBitPos))
		lssCurSttsDcdd.prdCyclIsOn = true;
	else
		lssCurSttsDcdd.prdCyclIsOn = false;

	// From here on the attribute flags are from the underlying DbncdMPBttn elements and it's status might be obtained from them by using the respective pointers and getters
	if(pkgOtpts & (((uint32_t)1) << IsEnbldLftHndBitPos))
		lssCurSttsDcdd.lftHndIsEnbld = true;
	else
		lssCurSttsDcdd.lftHndIsEnbld = false;

	if(pkgOtpts & (((uint32_t)1) << IsOnLftHndBitPos))
		lssCurSttsDcdd.lftHndIsOn = true;
	else
		lssCurSttsDcdd.lftHndIsOn = false;

	if(pkgOtpts & (((uint32_t)1) << IsEnbldRghtHndBitPos))
		lssCurSttsDcdd.rghtHndIsEnbld = true;
	else
		lssCurSttsDcdd.rghtHndIsEnbld = false;

	if(pkgOtpts & (((uint32_t)1) << IsOnRghtHndBitPos))
		lssCurSttsDcdd.rghtHndIsOn = true;
	else
		lssCurSttsDcdd.rghtHndIsOn = false;

	return lssCurSttsDcdd;
}
