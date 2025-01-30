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
  *       Last update:   30/01/2025 14:00 (GMT+0200)
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


//=======================================> Static variables initialization BEGIN
bool LimbsSftyLnFSwtch::_ltchRlsPndng = false;
//=========================================> Static variables initialization END

//=========================================================================> Class methods delimiter
LimbsSftyLnFSwtch::LimbsSftyLnFSwtch()
{
}

LimbsSftyLnFSwtch::LimbsSftyLnFSwtch(swtchInptHwCfg_t lftHndInpCfg, swtchBhvrCfg_t lftHndBhvrCfg, swtchInptHwCfg_t rghtHndInpCfg, swtchBhvrCfg_t rghtHndBhvrCfg, swtchInptHwCfg_t ftInpCfg, swtchBhvrCfg_t ftBhvrCfg, lsSwtchSwCfg_t lsSwtchWrkngCnfg)
:_lftHndInpCfg{lftHndInpCfg}, _lftHndBhvrCfg{lftHndBhvrCfg}, _rghtHndInpCfg{rghtHndInpCfg}, _rghtHndBhvrCfg{rghtHndBhvrCfg}, _ftInpCfg{ftInpCfg}, _ftBhvrCfg{ftBhvrCfg}
{
   // Build underlying DbncdMPBttn objects and pointers
   _undrlLftHndMPBPtr = new TmVdblMPBttn(_lftHndInpCfg.inptPin, _lftHndBhvrCfg.swtchVdTm, _lftHndInpCfg.pulledUp, _lftHndInpCfg.typeNO, _lftHndInpCfg.dbncTime, _lftHndBhvrCfg.swtchStrtDlyTm, true);
   _undrlRghtHndMPBPtr = new TmVdblMPBttn (_rghtHndInpCfg.inptPin, _rghtHndBhvrCfg.swtchVdTm, _rghtHndInpCfg.pulledUp, _rghtHndInpCfg.typeNO, _rghtHndInpCfg.dbncTime, _rghtHndBhvrCfg.swtchStrtDlyTm, true);
   _undrlFtMPBPtr = new SnglSrvcVdblMPBttn(_ftInpCfg.inptPin, _ftInpCfg.pulledUp, _ftInpCfg.typeNO, _ftInpCfg.dbncTime, _ftBhvrCfg.swtchStrtDlyTm);
   
   // Configure underlying DbncdMPBttn objects and pointers  
   // Left Hand TmVdblMPBttn   
   if(!_lftHndBhvrCfg.swtchIsEnbld) // TmVdblMPBttn objects are instantiated with _isEnabled = true property value
      _undrlLftHndMPBPtr->setBeginDisabled(true);
   // Right Hand TmVdblMPBttn   
   if(!_rghtHndBhvrCfg.swtchIsEnbld)   // TmVdblMPBttn objects are instantiated with _isEnabled = true property value
      _undrlRghtHndMPBPtr->setBeginDisabled(true);
   // Foot SnglSrvcVdblMPBttn   
   _undrlRghtHndMPBPtr->setBeginDisabled(true);
   _undrlFtMPBPtr-> setFnWhnTrnOnPtr(_setLtchRlsPndng); 

   // Configure LimbsSftyLnFSwtch attributes
   _ltchRlsTtlTm = lsSwtchWrkngCnfg.ltchRlsActvTm;
   _prdCyclTtlTm = lsSwtchWrkngCnfg.prdCyclActvTm;      
}

LimbsSftyLnFSwtch::~LimbsSftyLnFSwtch(){
   _undrlFtMPBPtr->~SnglSrvcVdblMPBttn();
   _undrlRghtHndMPBPtr->~TmVdblMPBttn();
   _undrlLftHndMPBPtr->~TmVdblMPBttn();
}

void LimbsSftyLnFSwtch::_ackBthHndsOnMssd(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   
      //---------------->> Tasks related actions
      if(getTskToNtfyBthHndsOnMssd() != NULL){
         xReturned = xTaskNotify(
            getTskToNtfyBthHndsOnMssd(),	// TaskHandle_t of the task receiving notification
            static_cast<uint32_t>(0x00),
            eSetValueWithOverwrite	// In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }
		//---------------->> Functions related actions
		if(_fnWhnBthHndsOnMssd != nullptr){
			_fnWhnBthHndsOnMssd(_fnWhnBthHndsOnMssdArg);
		}
	   //---------------->> Flags related actions

   return;
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
                     _swtchPollTmrName.c_str(),  // Timer name
                     pdMS_TO_TICKS(pollDelayMs),  // Timer period in ticks
                     pdTRUE,     // Auto-reload true
                     this,       // TimerID: the data passed as parameter to the callback function is this same object
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

   //todo Check why this must be forced!! BEGIN -----------------> Not right, go directly to the _isEnabled flag value!!!
   _undrlLftHndMPBPtr->setIsOnDisabled(true);
   if(_lftHndBhvrCfg.swtchIsEnbld){
      _undrlLftHndMPBPtr->enable();
   }
   else{
      _undrlLftHndMPBPtr->disable();
   }
   _undrlRghtHndMPBPtr->setIsOnDisabled(true);
   if(_rghtHndBhvrCfg.swtchIsEnbld){
      _undrlRghtHndMPBPtr->enable();
   }
   else{
      _undrlRghtHndMPBPtr->disable();
   }
   //todo Check why this must be forced!! END

   return;
}

void LimbsSftyLnFSwtch::_clrSttChng(){
   _sttChng = false;

   return;
}

void LimbsSftyLnFSwtch::cnfgFtSwtch(const swtchBhvrCfg_t &newCfg){
   _undrlFtMPBPtr->setStrtDelay(newCfg.swtchStrtDlyTm);
   _ftBhvrCfg.swtchStrtDlyTm = newCfg.swtchStrtDlyTm;

   return;
}

bool LimbsSftyLnFSwtch::_cnfgHndSwtch(const bool &isLeft, const swtchBhvrCfg_t &newCfg){
   bool result{false};

   if((isLeft?_undrlLftHndMPBPtr:_undrlRghtHndMPBPtr)->getStrtDelay() != newCfg.swtchStrtDlyTm){
      (isLeft?_undrlLftHndMPBPtr:_undrlRghtHndMPBPtr)->setStrtDelay(newCfg.swtchStrtDlyTm);
      (isLeft?_lftHndBhvrCfg:_rghtHndBhvrCfg).swtchStrtDlyTm = newCfg.swtchStrtDlyTm;
   }
   
   if((isLeft?_undrlLftHndMPBPtr:_undrlRghtHndMPBPtr)->getIsEnabled() != newCfg.swtchIsEnbld){
      if(newCfg.swtchIsEnbld)
         (isLeft?_undrlLftHndMPBPtr:_undrlRghtHndMPBPtr)->enable();
      else
         (isLeft?_undrlLftHndMPBPtr:_undrlRghtHndMPBPtr)->disable();
      
      (isLeft?_lftHndBhvrCfg:_rghtHndBhvrCfg).swtchIsEnbld = newCfg.swtchIsEnbld;
   }
   
   if((isLeft?_undrlLftHndMPBPtr:_undrlRghtHndMPBPtr)->getVoidTime() != newCfg.swtchVdTm){
      result = (isLeft?_undrlLftHndMPBPtr:_undrlRghtHndMPBPtr)->setVoidTime(newCfg.swtchVdTm);
      if(result)
         (isLeft?_lftHndBhvrCfg:_rghtHndBhvrCfg).swtchVdTm = newCfg.swtchVdTm;
   }

   return result;
}

bool LimbsSftyLnFSwtch::cnfgLftHndSwtch(const swtchBhvrCfg_t &newCfg){

   return _cnfgHndSwtch(true, newCfg);
}

bool LimbsSftyLnFSwtch::cnfgRghtHndSwtch(const swtchBhvrCfg_t &newCfg){

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

const bool LimbsSftyLnFSwtch::getLsSwtchOtptsChng() const{

	return _lsSwtchOtptsChng;
}

uint32_t LimbsSftyLnFSwtch::getLsSwtchOtptsSttsPkgd(){
   _getUndrlSwtchStts();
   
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

const TaskHandle_t LimbsSftyLnFSwtch::getTskToNtfyBthHndsOnMssd() const{
   
   return _tskToNtfyBthHndsOnMssd;
}

const TaskHandle_t LimbsSftyLnFSwtch::getTskToNtfyLsSwtchOtptsChng() const{
   
   return _tskToNtfyLsSwtchOtptsChng;
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

void LimbsSftyLnFSwtch::_getUndrlSwtchStts(){   
   _lftHndSwtchStts = otptsSttsUnpkg(_undrlLftHndMPBPtr->getOtptsSttsPkgd());
   _rghtHndSwtchStts = otptsSttsUnpkg(_undrlRghtHndMPBPtr->getOtptsSttsPkgd());
   _ftSwtchStts = otptsSttsUnpkg(_undrlFtMPBPtr->getOtptsSttsPkgd());

   return;
}

uint32_t LimbsSftyLnFSwtch::_lsSwtchOtptsSttsPkgd(uint32_t prevVal){
/*
+--+--+--+--+--+--+--+--++--+--+--+--+--+--+--+--++--+--+--+--+--+--+--+--++--+--+--+--+--+--+--+--+
|31|30|29|28|27|26|25|24||23|22|21|20|19|18|17|16||15|14|13|12|11|10|09|08||07|06|05|04|03|02|01|00|
                                                                     -- --  -- -- -- -- -- -- -- --
                                                                      |  |   |  |  |  |  |  |  |  |
                                                                      |  |   |  |  |  |  |  |  |  lftHndSwtchIsEnbld
                                                                      |  |   |  |  |  |  |  |  lftHndSwtchIsOn
                                                                      |  |   |  |  |  |  |  lftHndSwtchIsVdd
                                                                      |  |   |  |  |  |  rghtHndSwtchIsEnbld
                                                                      |  |   |  |  |  rghtHndSwtchIsOn
                                                                      |  |   |  |  rghtHndSwtchIsVdd
                                                                      |  |   |  ftSwtchIsEnbld
                                                                      |  |   ftSwtchIsOn
                                                                      |  LsSwtchLtchRlsIsOn
                                                                      LsSwtchPrdCyclIsOn
*/   
	if(_lftHndSwtchStts.isEnabled)
		prevVal |= ((uint32_t)1) << lftHndSwtchIsEnbldBP;
	else
		prevVal &= ~(((uint32_t)1) << lftHndSwtchIsEnbldBP);
   if(_lftHndSwtchStts.isOn)
		prevVal |= ((uint32_t)1) << lftHndSwtchIsOnBP;
	else
		prevVal &= ~(((uint32_t)1) << lftHndSwtchIsOnBP);
   if(_lftHndSwtchStts.isVoided)
		prevVal |= ((uint32_t)1) << lftHndSwtchIsVddBP;
	else
		prevVal &= ~(((uint32_t)1) << lftHndSwtchIsVddBP);

	if(_rghtHndSwtchStts.isEnabled)
		prevVal |= ((uint32_t)1) << rghtHndSwtchIsEnbldBP;
	else
		prevVal &= ~(((uint32_t)1) << rghtHndSwtchIsEnbldBP);
	if(_rghtHndSwtchStts.isOn)
		prevVal |= ((uint32_t)1) << rghtHndSwtchIsOnBP;
	else
		prevVal &= ~(((uint32_t)1) << rghtHndSwtchIsOnBP);
	if(_rghtHndSwtchStts.isVoided)
		prevVal |= ((uint32_t)1) << rghtHndSwtchIsVddBP;
	else
		prevVal &= ~(((uint32_t)1) << rghtHndSwtchIsVddBP);

	if(_ftSwtchStts.isEnabled)
		prevVal |= ((uint32_t)1) << ftSwtchIsEnbldBP;
	else
		prevVal &= ~(((uint32_t)1) << ftSwtchIsEnbldBP);
	if(_ftSwtchStts.isOn)   //! The ftSwtch is a SnglShtMPBttn object, the isOn flag makes no stable signal source!!
		prevVal |= ((uint32_t)1) << ftSwtchIsOnBP;
	else
		prevVal &= ~(((uint32_t)1) << ftSwtchIsOnBP);

	if(_ltchRlsIsOn)
		prevVal |= ((uint32_t)1) << LsSwtchLtchRlsIsOnBP;
	else
		prevVal &= ~(((uint32_t)1) << LsSwtchLtchRlsIsOnBP);
	if(_prdCyclIsOn)
		prevVal |= ((uint32_t)1) << LsSwtchPrdCyclIsOnBP;
	else
		prevVal &= ~(((uint32_t)1) << LsSwtchPrdCyclIsOnBP);

   return prevVal;
}

void LimbsSftyLnFSwtch::lsSwtchPollCb(TimerHandle_t lssTmrCbArg){
   LimbsSftyLnFSwtch* lsSwtchObj = (LimbsSftyLnFSwtch*)pvTimerGetTimerID(lssTmrCbArg);
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
   // Underlying switches status recovery
   lsSwtchObj->_getUndrlSwtchStts();
   //------------
   // Set the time base for Flags, Triggers and Timers calculation & update
 	lsSwtchObj->_updCurTimeMs();
   //------------
	// State machine update
 	lsSwtchObj->_updFdaState();
 	taskEXIT_CRITICAL(&mux);

	//Outputs update, function and tasks executions based on outputs changed generated by the State Machine
      //---------------->> Tasks related actions
      //---------------->> Generic Task for output changes related actions
	if (lsSwtchObj->getLsSwtchOtptsChng()){
		if(lsSwtchObj->getTskToNtfyLsSwtchOtptsChng() != NULL){
			xReturned = xTaskNotify(
				lsSwtchObj->getTskToNtfyLsSwtchOtptsChng(),	// TaskHandle_t of the task receiving notification
				static_cast<uint32_t>(lsSwtchObj->getLsSwtchOtptsSttsPkgd()),
				eSetValueWithOverwrite
			);
			lsSwtchObj->setLsSwtchOtptsChng(false);
		}
	}     

	return;
}

void LimbsSftyLnFSwtch::resetFda(){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	clrStatus();
	_setSttChng();
	_lsSwtchFdaState = stOffNotBHP;
	taskEXIT_CRITICAL(&mux);

   return;
}

void LimbsSftyLnFSwtch::_rstOtptsChngCnt(){
   _lsSwtchOtptsChngCnt = 0;

   return;
}

void LimbsSftyLnFSwtch::setFnWhnBthHndsOnMssd(fncVdPtrPrmPtrType &newFnWhnBthHndsOnMssd){
   if(_fnWhnBthHndsOnMssd != newFnWhnBthHndsOnMssd)
      _fnWhnBthHndsOnMssd = newFnWhnBthHndsOnMssd;

   return;
}

void LimbsSftyLnFSwtch::setFnWhnTrnOffLtchRlsPtr(fncVdPtrPrmPtrType &newFnWhnTrnOff){
   if(_fnWhnTrnOffLtchRls != newFnWhnTrnOff)
      _fnWhnTrnOffLtchRls = newFnWhnTrnOff;

   return;
}

void LimbsSftyLnFSwtch::setFnWhnTrnOffPrdCyclPtr(fncVdPtrPrmPtrType &newFnWhnTrnOff){
   if(_fnWhnTrnOffPrdCycl != newFnWhnTrnOff)
      _fnWhnTrnOffPrdCycl = newFnWhnTrnOff;

   return;
}

void LimbsSftyLnFSwtch::setFnWhnTrnOnLtchRlsPtr(fncVdPtrPrmPtrType &newFnWhnTrnOn){
   if(_fnWhnTrnOnLtchRls != newFnWhnTrnOn)
      _fnWhnTrnOnLtchRls = newFnWhnTrnOn;

   return;
}

void LimbsSftyLnFSwtch::setFnWhnTrnOnPrdCyclPtr(fncVdPtrPrmPtrType &newFnWhnTrnOn){
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

void LimbsSftyLnFSwtch::setLsSwtchOtptsChng(bool newLsSwtchOtptsChng){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(newLsSwtchOtptsChng)
		++_lsSwtchOtptsChngCnt;
	else
		if(_lsSwtchOtptsChngCnt)
			--_lsSwtchOtptsChngCnt;
	if(_lsSwtchOtptsChngCnt)
		_lsSwtchOtptsChng = true;
	else
		_lsSwtchOtptsChng = false;
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

void LimbsSftyLnFSwtch::setTrnOffLtchRlsArgPtr(void *&newVal){
   if(_fnWhnTrnOffLtchRlsArg != newVal)
      _fnWhnTrnOffLtchRlsArg = newVal;

   return;
}

void LimbsSftyLnFSwtch::setTrnOffPrdCyclArgPtr(void* &newVal){
   if(_fnWhnTrnOffPrdCyclArg != newVal)
      _fnWhnTrnOffPrdCyclArg = newVal;

   return;
}

void LimbsSftyLnFSwtch::setTrnOnLtchRlsArgPtr(void* &newVal){
   if(_fnWhnTrnOnLtchRlsArg != newVal)
      _fnWhnTrnOnLtchRlsArg = newVal;

   return;
}
	
void LimbsSftyLnFSwtch::setTrnOnPrdCyclArgPtr(void* &newVal){
   if(_fnWhnTrnOnPrdCyclArg != newVal)
      _fnWhnTrnOnPrdCyclArg = newVal;

   return;
}

void LimbsSftyLnFSwtch::setTskToNtfyBthHndsOnMssd(const TaskHandle_t &newTaskHandle){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	eTaskState taskToNtfyStt{};

   taskENTER_CRITICAL(&mux);
   if(_tskToNtfyBthHndsOnMssd != newTaskHandle){
      if(_tskToNtfyBthHndsOnMssd != NULL){
         taskToNtfyStt = eTaskGetState(_tskToNtfyBthHndsOnMssd);
         if (taskToNtfyStt != eSuspended){
            if(taskToNtfyStt != eDeleted){
               vTaskSuspend(_tskToNtfyBthHndsOnMssd);
               _tskToNtfyBthHndsOnMssd = NULL;
            }
         }
      }
      if (newTaskHandle != NULL)
         _tskToNtfyBthHndsOnMssd = newTaskHandle;
   }
   taskEXIT_CRITICAL(&mux);

	return;
}

void LimbsSftyLnFSwtch::setTskToNtfyLsSwtchOtptsChng(const TaskHandle_t &newTaskHandle){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	eTaskState taskToNtfyStt{};

   taskENTER_CRITICAL(&mux);
   if(_tskToNtfyLsSwtchOtptsChng != newTaskHandle){
      if(_tskToNtfyLsSwtchOtptsChng != NULL){
         taskToNtfyStt = eTaskGetState(_tskToNtfyLsSwtchOtptsChng);
         if (taskToNtfyStt != eSuspended){
            if(taskToNtfyStt != eDeleted){
               vTaskSuspend(_tskToNtfyLsSwtchOtptsChng);
               _tskToNtfyLsSwtchOtptsChng = NULL;
            }
         }
      }
      if (newTaskHandle != NULL)
         _tskToNtfyLsSwtchOtptsChng = newTaskHandle;
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
            getTskToNtfyTrnOffLtchRls(),	// TaskHandle_t of the task receiving notification
            static_cast<uint32_t>(0x00),
            eSetValueWithOverwrite	// In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }
		//---------------->> Functions related actions
		if(_fnWhnTrnOffLtchRls != nullptr){
			_fnWhnTrnOffLtchRls(_fnWhnTrnOffLtchRlsArg);
		}
	   //---------------->> Flags related actions
		taskENTER_CRITICAL(&mux);
      _ltchRlsIsOn = false;
		setLsSwtchOtptsChng(true);
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
            getTskToNtfyTrnOffPrdCycl(),	// TaskHandle_t of the task receiving notification
            static_cast<uint32_t>(0x00),
            eSetValueWithOverwrite	// In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }
		//---------------->> Functions related actions
		if(_fnWhnTrnOffPrdCycl != nullptr){
			_fnWhnTrnOffPrdCycl(_fnWhnTrnOffPrdCyclArg);
		}
	   //---------------->> Flags related actions
		taskENTER_CRITICAL(&mux);
      _prdCyclIsOn = false;
		setLsSwtchOtptsChng(true);
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
            getTskToNtfyTrnOnLtchRls(),	// TaskHandle_t of the task receiving notification
            static_cast<uint32_t>(0x00),
            eSetValueWithOverwrite	// In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }
		//---------------->> Functions related actions
		if(_fnWhnTrnOnLtchRls != nullptr){
			_fnWhnTrnOnLtchRls(_fnWhnTrnOnLtchRlsArg);
		}
	   //---------------->> Flags related actions
		taskENTER_CRITICAL(&mux);
      _ltchRlsIsOn = true;
		setLsSwtchOtptsChng(true);
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
            getTskToNtfyTrnOnPrdCycl(),	// TaskHandle_t of the task receiving notification
            static_cast<uint32_t>(0x00),
            eSetValueWithOverwrite	// In this specific case using eSetBits is also a valid option
         );
         if (xReturned != pdPASS)
            errorFlag = pdTRUE;
      }
		//---------------->> Functions related actions
		if(_fnWhnTrnOnPrdCycl != nullptr){
			_fnWhnTrnOnPrdCycl(_fnWhnTrnOnPrdCyclArg);
		}
	   //---------------->> Flags related actions
		taskENTER_CRITICAL(&mux);
      _prdCyclIsOn = true;
		setLsSwtchOtptsChng(true);
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
         }	// Execute this code only ONCE, when exiting this state
			break;

		case stOffBHPNotFP:
			//In: >>---------------------------------->>
			if(_sttChng){
            _clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!(_lftHndSwtchStts.isOn && _rghtHndSwtchStts.isOn)){
            _undrlFtMPBPtr->disable(); // Disable FtSwitch
            _ackBthHndsOnMssd();
            _lsSwtchFdaState = stOffNotBHP;
            _setSttChng();
         }
         else{
            // Check the foot switch release signal ok flag
            if(_ltchRlsPndng){
               _ltchRlsPndng = false;
               _undrlLftHndMPBPtr->setIsOnDisabled(false);
               if(_lftHndBhvrCfg.swtchIsEnbld)
                  _undrlLftHndMPBPtr->disable();
               _undrlRghtHndMPBPtr->setIsOnDisabled(false);
               if(_rghtHndBhvrCfg.swtchIsEnbld)
                  _undrlRghtHndMPBPtr->disable();
               _undrlFtMPBPtr->disable();
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
         break;

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
            if(_lftHndBhvrCfg.swtchIsEnbld)
               _undrlLftHndMPBPtr->enable();
            _undrlRghtHndMPBPtr->setIsOnDisabled(true);
            if(_rghtHndBhvrCfg.swtchIsEnbld)
               _undrlRghtHndMPBPtr->enable();
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
         // Keep eternal loop
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

void LimbsSftyLnFSwtch::_setLtchRlsPndng(){
   _ltchRlsPndng = true;

   return;
}


//=========================================================================> Class methods delimiter

/**
 * @brief Unpackages a 32-bit value into a LimbsSftyLnFSwtch object status
 * 
 * The 32-bit encoded and packaged is used for inter-task object status communication, passed as a "notification value" in a xTaskNotify() execution.
 * For each bit value attribute flag represented see LimbsSftyLnFSwtch::getLsSwtchOtptsSttsPkgd()
 * 
 * @param pkgOtpts A 32-bit value holding a LimbsSftyLnFSwtch status encoded
 * @return A lsSwtchOtpts_t type element containing the information decoded
 */
lsSwtchOtpts_t lssOtptsSttsUnpkg(uint32_t pkgOtpts){
	lsSwtchOtpts_t lssCurSttsDcdd {0};

	// Attribute flags from the underlying DbncdMPBttn elements and it's status might be obtained from them by using the respective pointers and getters
	if(pkgOtpts & (((uint32_t)1) << lftHndSwtchIsEnbldBP))
		lssCurSttsDcdd.lftHndIsEnbld = true;
	else
		lssCurSttsDcdd.lftHndIsEnbld = false;
	if(pkgOtpts & (((uint32_t)1) << lftHndSwtchIsOnBP))
		lssCurSttsDcdd.lftHndIsOn = true;
	else
		lssCurSttsDcdd.lftHndIsOn = false;
	if(pkgOtpts & (((uint32_t)1) << lftHndSwtchIsVddBP))
		lssCurSttsDcdd.lftHndIsVdd = true;
	else
		lssCurSttsDcdd.lftHndIsVdd = false;

	if(pkgOtpts & (((uint32_t)1) << rghtHndSwtchIsEnbldBP))
		lssCurSttsDcdd.rghtHndIsEnbld = true;
	else
		lssCurSttsDcdd.rghtHndIsEnbld = false;
	if(pkgOtpts & (((uint32_t)1) << rghtHndSwtchIsOnBP))
		lssCurSttsDcdd.rghtHndIsOn = true;
	else
		lssCurSttsDcdd.rghtHndIsOn = false;

	if(pkgOtpts & (((uint32_t)1) << rghtHndSwtchIsVddBP))
		lssCurSttsDcdd.rghtHndIsVdd = true;
	else
		lssCurSttsDcdd.rghtHndIsVdd = false;

   if(pkgOtpts & (((uint32_t)1) << ftSwtchIsEnbldBP))
		lssCurSttsDcdd.ftSwIsEnbld = true;
	else
		lssCurSttsDcdd.ftSwIsEnbld = false;

   if(pkgOtpts & (((uint32_t)1) << ftSwtchIsOnBP))
		lssCurSttsDcdd.ftSwIsOn = true;
	else
		lssCurSttsDcdd.ftSwIsOn = false;

	if(pkgOtpts & (((uint32_t)1) << LsSwtchLtchRlsIsOnBP))
		lssCurSttsDcdd.ltchRlsIsOn = true;
	else
		lssCurSttsDcdd.ltchRlsIsOn = false;
	if(pkgOtpts & (((uint32_t)1) << LsSwtchPrdCyclIsOnBP))
		lssCurSttsDcdd.prdCyclIsOn = true;
	else
		lssCurSttsDcdd.prdCyclIsOn = false;
   
	return lssCurSttsDcdd;
}
