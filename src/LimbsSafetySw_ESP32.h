/**
  ******************************************************************************
  * @file	: LimbsSafetySw_ESP32.h
  * @brief	: Header file for the LimbsSafetySw_ESP32 library classes
  *
  * @details The library implements a class that models a Limbs Safety Switch for
  * industrial production machines or dangerous devices. The LimbsSftySnglShtSw
  * cumpliments the following properties:
  * 
  * - Its input signals are analog to those produced by an MPB, including but not limited to:
  *   - MPBs
  *   - Digital output pressure sensors
  *   - Proximity sensors
  *   - Infrared U-Slot Photoelectric Sensor
  * - Its main output signal will be limited to:
  *   - Release|OK/Latch|Not-OK
  * - Its secondary outputs signals will be limited to:
  *   - Enabled|Disabled
  *   - Valid|Voided
  *   - Pressed|Non-pressed
  *   - Both Hands OK (Ready for foot switch)
  *
  * @author	: Gabriel D. Goldman
  * @version v1.0.0
  * @date	: Created on: 11/11/2024
  * 		   : Last updated: 25/12/2024
  * @copyright GPL-3.0 license
  *
  ******************************************************************************
  * @attention	This library was developed as part of the refactoring process for
  * an industrial machines safety enforcement and productivity control
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
#ifndef _LIMBSSAFETYSW_ESP32_H_
#define _LIMBSSAFETYSW_ESP32_H_

#include <Arduino.h>
#include <stdint.h>
#include <ButtonToSwitch_ESP32.h>

//==============================================>> BEGIN User defined constants
#ifndef _InvalidPinNum
   #define _InvalidPinNum GPIO_NUM_NC  //Not Connected pin number = -1, so a signed numeric type must be used!
#endif
#ifndef _maxValidPinNum
   #define _maxValidPinNum GPIO_NUM_MAX-1
#endif
#define _stdTVMPBttnVoidTime 10000UL
#define _stdTVMPBttnDelayTime 0UL
#define _stdSSVMPBttnDelayTime 0UL
#define _dfltPollDelay 20UL
//=================================================>> END User defined constants

//===================================================>> BEGIN User defined types
/**
 * @struct gpioPinOtptHwCfg_t
 * @brief GPIO Generic Pin for Output Mode Configuration data structure
 * 
 * @param gpioOtptPin Pin identified by pin number, to be used in OUTPUT mode
 * @param gpioOtptActHgh Boolean indicating if the device attached to the pin is activated by a high level signal (true) or a low level signal (false)
 * 
 * Resource to keep the class design flexible to be used with different hardware
 * components, this structure holds the output GPIO pin identification and a
 * flag indicating if the voltage level needed to activate the device connected 
 * to the output pin. Polarity connection of leds, CC or CA RGB leds, low or high
 * level activated relays are examples of where the gpioPinOtptHwCfg_t is a
 * convenient replacement to simple pin configuration as Output Mode
 */
struct gpioPinOtptHwCfg_t{
   int8_t gpioOtptPin = _InvalidPinNum;
   bool gpioOtptActHgh = true;
};

/**
 * @struct limbSftyFwConf_t
 * @brief Limbs Safety Firmware Configuration parameters
 * 
 * Holds the parameters for the creation and configuration of the Execution Core
 * (for multicore MCUs) and Task Execution Priority level for the switch status
 * update code execution.
 * 
 * @attention Software construction related!! The information must be provided by
 * the software developers as it is related to the general develpment parameters.
 * The core selection for the Task Execution might be provided by the hardware
 * development team or automatically determined by O.S. as the core executing 
 * the applications.
 */
struct limbSftyFwConf_t{
  uint8_t lsSwExecTskCore;
  uint8_t lsSwExecTskPrrtyCnfg; // Holds the priority level for the task running the switch code
};

/**
 * @struct limbSftySwConf_t
 * @brief Limbs Safety Software Configuration parameters
 * 
 * Holds the required set of parameters needed for the configuration of each
 * of the three DbncdMPBttn subclass switches needed for input (left hand's 
 * TmVdblMPBttn, right hand's TmVdblMPBttn, foot's SnglSrvcVdblMPBttn)
 * 
 * Each parameter has default values assigned for a standard LimbsSftySnglShtSw
 * configuration. The provided default values are expected to be only used if
 * no explicit values are provided by the object instantiating software (from
 * previous) executions configured values.
 * 
 * @param swtchStrtDlyTm Corresponds to the DbncdMPBttn subclasses strtDelay constructor parameter
 * @param swtchIsEnbld Corresponds to the DbncdMPBttn subclasses _isEnabled attribute
 * @param swtchVdTm Corresponds to the TmVdblMPBttn class voidTime constructor parameter, and will be used as an acivation time limit for the foot controled SnglSrvcMPBttn
 */
struct limbSftySwCfg_t{
  unsigned long int swtchStrtDlyTm = 0;
  bool swtchIsEnbld = true;
  unsigned long int swtchVdTm = _stdTVMPBttnVoidTime;
};

/**
 * @struct swtchInptHwCfg_t
 * @brief Switch Input Hardware Configuration data structure
 * 
 * Holds the hardware characteristics of each of the switches used.
 * As the attributes held in the structure are all hardware related they must be
 * set or modified only when hardware construction or modification happens. The
 * actual parameters must be saved in non volatile memory or by physical means
 * (jumpers, dip switches, etc.) and are non user nor supervisor modifiable. 
 * Only designer level operators might modify them.
 * 
 * The DbncdMPBttn subclasses objects default configuration values are:
 * - Type of switch: NO
 * - Switch connection: Pulled-up
 * - Debounce Time = 20 milliseconds
 * 
 * @note GPIO_NUM_NC = -1 is used to signal not connected to S/W 
 * @note GPIO_NUM_MAX is used to contain the maximum number of a GPIO pin (GPIO_NUM_x < GPIO_NUM_MAX)
 * 
 * @attention Hardware construction related!! The information must be provided by
 * the hardware developers
*/
struct swtchInptHwCfg_t{
   int8_t inptPin;
   bool typeNO = true;
   bool pulledUp = true;
   unsigned long int dbncTime = 20;
};

/**
 * @struct swtchOtptHwCfg_t
 * @brief Switch Output Hardware Configuration data structure
 * 
 * Holds the pins assigned to output the state of the isOn, isVoided and 
 * isEnabled attribute flag for the TmVdblMPBttn class hands switches.
 * 
 * @attention Hardware construction related!! The information must be provided by
 * the hardware developers
 */
struct swtchOtptHwCfg_t{
   gpioPinOtptHwCfg_t isOnPin;  
   gpioPinOtptHwCfg_t isVoidedPin;  
   gpioPinOtptHwCfg_t isEnabledPin;  
};
//===================================================>> END User defined types

//======================================>> BEGIN General use function prototypes
//========================================>> END General use function prototypes

//===========================>> BEGIN General use Static variables and constants
static const uint8_t _exePrty = 1;   //Execution priority of the updating Task
static const int app_cpu = xPortGetCoreID();
static BaseType_t rc;
//=============================>> END General use Static variables and constants

//=================================================>> BEGIN Classes declarations
/**
 * @class LimbsSftySnglShtSw
 * @brief Models a Limbs Safety Single Shot Switch (LimbsSftySnglShtSw)
 * 
 * This industry grade machinery oriented switch enforces conditioned activation
 * of several switches to ensure limbs security. This enforced conditions 
 * include sequenced pressing order and timings as main parameters.
 * The most basic configuration requires each hand pressing simultaneously two
 * switches to enable a foot switch. The enabled foot switch activates the 
 * machinery action.  
 * Operators' or production managers' needs and criteria often end with regular
 * security switches tampered, played down, rigged or other actions that degrade
 * the security provided by regular switches, far below the real needs and the 
 * standards.
 * The switches modeled by this class ensures the required enforced security
 * practices, while letting some aspects to be configured for specific 
 * production tasks on the machine, or to adapt the switch to machines with 
 * different security needs.
 * 
 * This class relies on ButtonToSwitch_ESP32 class objects:
 * - A TmVdblMPBttn object for each hand switch
 * - A SnglSrvcVdblMPBttn object for the foot switch
 */
class LimbsSftySnglShtSw{
private:
  static TaskHandle_t lssTskToNtfyHndl;
  const unsigned long int _minVoidTime{1000};
  enum fdaLsSwtchStts {
		stOffNotBHP,   /*State: Switch off, NOT both hands pressed*/
		stOffBHPNotFP, /*State: Switch off, both hands pressed, NOT foot press*/
		stStrtRlsStrtCycl,   /*State: Both hands and foot pressed, start Production Cycle*/
		stEndRls,  /*State: end latch released phase */
      stEndCycl,   /*State: End production cycle, restart FDA*/
      stEmrgncyExcpHndl   /*State: Handle received Emergency Exception signal*/
	};

protected:
   swtchInptHwCfg_t _lftHndInpCfg;
   swtchInptHwCfg_t _rghtHndInpCfg;
   swtchInptHwCfg_t _ftInpCfg;

   limbSftySwCfg_t _lftHndSwCfg{};
   limbSftySwCfg_t _rghtHndSwCfg{};
   limbSftySwCfg_t _ftSwCfg{};

   SnglSrvcVdblMPBttn* _undrlFtSSVMPBPtr{nullptr};   
   TmVdblMPBttn* _undrlLftHndHTVMPBPtr{nullptr};
   TmVdblMPBttn* _undrlRghtHndHTVMPBPtr{nullptr};

   MpbOtpts_t _lftHndSwtchStts{0};
   MpbOtpts_t _rghtHndSwtchStts{0};
   MpbOtpts_t _ftSwtchStts{0};

   unsigned long int _undrlSwtchsPollDelay{_dfltPollDelay};

   unsigned long int _curTime{0};
   unsigned long int _prdCyclTmrStrt{0};
   unsigned long int _prdCyclTtlTm{0};
   unsigned long int _ltchRlsTtlTm{0};

   bool _bthHndsSwArOn{false};
   bool _ltchRlsIsOn{false};
   bool _prdCyclIsOn{false};

   fdaLsSwtchStts _lsSwtchFdaState {stOffNotBHP};
   TimerHandle_t _lsSwtchPollTmrHndl {NULL};   //FreeRTOS returns NULL if creation fails (not nullptr)
   bool _outputsChange{false};
   uint32_t _outputsChangeCnt{0};
   bool _sttChng{true};
   String _swtchPollTmrName{"lsSwtch-01"};

	static void lsSwtchPollCb(TimerHandle_t lssTmrCbArg);

   void _clrSttChng();
   bool _cnfgHndSwtch(const bool &isLeft, const limbSftySwCfg_t &newCfg);
	void _setSttChng();
   void _updBothHndsSwState();
   void _updCurTime();
   void _updUndrlSwState();
   void _updFdaState();
   bool _updOutputs();

public:
  /**
   * @brief Class default constructor
   * 
   */
  LimbsSftySnglShtSw();
  /**
   * @brief Class constructor
   * 
   * The class models a Limbs Safety Single Shot switch, a switch that gives an
   * activation signal to a machine or device when some independent switches
   * activated in a designated pattern ensures no risk for the operator limbs is
   * completed.
   * The constructor must instantiate the DbncdMPBttn subclasses objects that 
   * compose the Limbs Safety Single Shot switch, i.e. the left hand 
   * TmVdblMPBttn, the right hand TmVdblMPBttn and the foot SnglSrvcVdblMPBttn
   * 
   * @param lftHndInpPrm A swtchInptHwCfg_t structure containing the hardware implemented characteristics for the left hand controlled TmVdblMPBttn
   * @param rghtHndInpPrm A swtchInptHwCfg_t structure containing the hardware implemented characteristics for the right hand controlled TmVdblMPBttn
   * @param ftInpPrm A swtchInptHwCfg_t structure containing the hardware implemented characteristics for the foot controlled SnglSrvcVdblMPBttn
   * @param ltchRlsActvOtpPin (Optional) GPIO pin assigned to attach a device (indicator or other) to be activated when the switch signals ON. 
   * @param lftHndOtptPrm A swtchOtptHwCfg_t structure containing the hardware implemented characteristics for the left hand output indicators for isOn, isEnabled and isVoided attributes flags
   * @param rghtHndOtptPrm A swtchOtptHwCfg_t structure containing the hardware implemented characteristics for the right hand output indicators for isOn, isEnabled and isVoided attributes flags
   * @param ftOtptPrm A swtchOtptHwCfg_t structure containing the hardware implemented characteristics for the foot switch output indicators for isOn attribute flag
   * @param hndsSwtchsOkPin (Optional) GPIO pin assigned to attach a device (indicator or other) activated when the hands safety protocol is cumplimented
   */
  LimbsSftySnglShtSw(swtchInptHwCfg_t lftHndInpCfg,
                    swtchInptHwCfg_t rghtHndInpCfg,
                    swtchInptHwCfg_t ftInpCfg
                    );
   ~LimbsSftySnglShtSw();
   bool begin(unsigned long int updtPeriod);
   void clrStatus();
   bool getBothHndsSwOk();
   const bool getOutputsChange() const;
   bool cnfgFtSwtch(const limbSftySwCfg_t &newCfg);
   bool cnfgLftHndSwtch(const limbSftySwCfg_t &newCfg);
   bool cnfgRghtHndSwtch(const limbSftySwCfg_t &newCfg);
   bool setLtchRlsTm(const unsigned long int &newVal);
   /**
	 * @brief Sets the value of the attribute flag indicating if a change took place in any of the output attribute flags (IsOn included).
	 *
	 * The usual path for the **outputsChange** flag is to be set by any method changing an output attribute flag, the callback function signaled to take care of the hardware actions because of this changes clears back **outputsChange** after taking care of them. In the unusual case the developer wants to "intercept" this sequence, this method is provided to set (true) or clear (false) outputsChange value.
    *
    * @param newOutputChange The new value to set the **outputsChange** flag to.
    */
	void setOutputsChange(bool newOutputsChange);
   bool setPrdCyclTm(const unsigned long int &newVal);
   bool setUndrlSwtchPollDelay(DbncdMPBttn* undrlSwtch, const unsigned long int &newVal);
   // bool end();
   // fncPtrType getFnWhnTrnOff();
	// fncPtrType getFnWhnTrnOn();
   // bool getIsOn();
	// const TaskHandle_t getTaskToNotify() const;
   // bool pause();
   // void resetFda();
   // bool resume();
	// void setFnWhnTrnOffPtr(void(*newFnWhnTrnOff)());
	// void setFnWhnTrnOnPtr(void (*newFnWhnTrnOn)());
	// void setTaskToNotify(const TaskHandle_t &newTaskHandle);    
};
//=============================================================>

class LimbsSftySnglShtSwHI{
private:

protected:
   swtchOtptHwCfg_t _lftHndOtptCfg;
   swtchOtptHwCfg_t _rghtHndOtptCfg;
   swtchOtptHwCfg_t _ftOtptCfg;

   gpioPinOtptHwCfg_t _hndsSwtchsOkPin{};
   gpioPinOtptHwCfg_t _ltchRlsActvOtpPin{};
   gpioPinOtptHwCfg_t _prdCyclActvOtpPin{};

public:
  /**
   * @brief Class default constructor
   * 
   */
  LimbsSftySnglShtSwHI();
  /**
   * @brief Class constructor
   * 
   * The class models a Limbs Safety Single Shot switch, a switch that gives an
   * activation signal to a machine or device when some independent switches
   * activated in a designated pattern ensures no risk for the operator limbs is
   * completed.
   * The constructor must instantiate the DbncdMPBttn subclasses objects that 
   * compose the Limbs Safety Single Shot switch, i.e. the left hand 
   * TmVdblMPBttn, the right hand TmVdblMPBttn and the foot SnglSrvcVdblMPBttn
   * 
   * @param ltchRlsActvOtpPin (Optional) GPIO pin assigned to attach a device (indicator or other) to be activated when the switch signals ON. 
   * @param lftHndOtptPrm A swtchOtptHwCfg_t structure containing the hardware implemented characteristics for the left hand output indicators for isOn, isEnabled and isVoided attributes flags
   * @param rghtHndOtptPrm A swtchOtptHwCfg_t structure containing the hardware implemented characteristics for the right hand output indicators for isOn, isEnabled and isVoided attributes flags
   * @param ftOtptPrm A swtchOtptHwCfg_t structure containing the hardware implemented characteristics for the foot switch output indicators for isOn attribute flag
   * @param hndsSwtchsOkPin (Optional) GPIO pin assigned to attach a device (indicator or other) activated when the hands safety protocol is cumplimented
   */
  LimbsSftySnglShtSwHI(gpioPinOtptHwCfg_t ltchRlsActvOtpPin,
                    gpioPinOtptHwCfg_t prdCyclActvOtpPin,
                    swtchOtptHwCfg_t lftHndOtptcfg,
                    swtchOtptHwCfg_t rghtHndOtptCfg,
                    swtchOtptHwCfg_t ftOtptCfg
                    );
~LimbsSftySnglShtSwHI();
};
//===================================================>> END Classes declarations


#endif   //_LIMBSSAFETYSW_ESP32_H_
