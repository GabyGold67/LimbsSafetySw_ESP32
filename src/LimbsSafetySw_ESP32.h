/**
  ******************************************************************************
  * @file	: LimbsSafetySw_ESP32.h
  * @brief	: Header file for the LimbsSafetySw_ESP32 library classes
  *
  * @details The library implements classes that model a Limbs Safety Switch for
  * industrial production machines. The LimbsSftySnglShtSw cumpliments the following 
  * properties:
  * - Its input signal is analog to those produced by an MPB, including but not limited to:
  *   - MPBs
  *   - Digital output pressure sensors
  *   - Proximity sensors
  *   - Infrared U-Slot Photoelectric Sensor
  * - Its main output signal will be limited to:
  *   - On/Pressed/OK/High
  *   - Off/Not Pressed/Not-OK/Low
  * - Its secondary outputs signals will be limited to:
  *   - Enabled|Disabled
  *   - Valid|Voided
  *   - Pressed|Non-pressed
  *   - Hands OK (Ready for foot switch)
  *   - lssIsOn
  *   - timerExceptionFlag
  *
  * @author	: Gabriel D. Goldman
  * @version v1.0.0
  * @date	: Created on: 11/11/2024
  * 		   : Last updated: 11/12/2024
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
   #define _maxValidPinNum GPIO_NUM_MAX-1
#endif
#define _stdTVMPBttnVoidTime 10000UL
#define _stdTVMPBttnDelayTime 0UL
#define _stdSSVMPBttnDelayTime 0UL
//=================================================>> END User defined constants

//===================================================>> BEGIN User defined types

/**
 * @struct gpioPinOtptHwCfg_t
 * @brief GPIO Pin Output Hardware Configuration data structure
 * 
 * Resource to keep the class design flexible to be used with different hardware
 * develpments, this structure holds the output GPIO pin identification and a
 * flag indicating if the device connected to the output pin is activated by a
 * LOW level voltage or a HIGH level voltage.
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
 * and Task Execution Priority level for the switch output responses.
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
 * Holds the default values for a standard LimbsSftySnglShtSw configuration.
 * The provided default values might be modified using the S&G methods provided.
 */
struct limbSftySwCfg_t{
  unsigned long int hndSwtchDlyTm = 0;
  bool hndSwtchEnbld = true;
  unsigned long int hndSwtchVdTm = _stdTVMPBttnVoidTime;
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
   int8_t isOnOtptPin = _InvalidPinNum;
   bool isOnOtptActHgh = true;
   int8_t isVddOtptPin = _InvalidPinNum;
   bool isVddOtptActHgh = true;
   int8_t isEnbldOtptPin = _InvalidPinNum;
   bool isEnabledOtptActHgh = true;
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
		stOffNotBHP,   /* State: Switch off, NOT both hands pressed signals isOn state  */
		stBHPNotFP,    /* State: Switch off, both hands pressed signals isOn state*/
		stOn,
		stOnTaSP,
      stOnToSR,
      stFailExcepHndl
	};

protected:
   TmVdblMPBttn* _undrlLftHndHTVMPBPtr{nullptr};
   TmVdblMPBttn* _undrlRghtHndHTVMPBPtr{nullptr};
   SnglSrvcVdblMPBttn* _undrlFtSSVMPBPtr;   
   swtchInptHwCfg_t _lftHndInpCfg;
   swtchInptHwCfg_t _rghtHndInpCfg;
   swtchInptHwCfg_t _ftInpCfg;
   gpioPinOtptHwCfg_t _sftySwActvOtpPin{};
   swtchOtptHwCfg_t _lftHndOtptCfg;
   swtchOtptHwCfg_t _rghtHndOtptCfg;
   swtchOtptHwCfg_t _ftOtptCfg;
   gpioPinOtptHwCfg_t _hndsSwtchsOkPin{};
   limbSftySwCfg_t _ftSwCfg{};
   limbSftySwCfg_t _lftHndSwCfg{};
   limbSftySwCfg_t _rghtHndSwCfg{};

   bool _bothHandsSwOk{false};
   fdaLsSwtchStts _lssFdaState {stOffNotBHP};
   TimerHandle_t _lsssSwtchPollTmrHndl {NULL};   //FreeRTOS returns NULL if creation fails (not nullptr)
   bool _sttChng{true};
   String _swtchPollTmrName{"lmbSftySwtch-01"};

	static void lsssSwtchPollCb(TimerHandle_t lssTmrCbArg);

   void _clrSttChng();
   bool _cnfgHndSwtch(bool isLeft, limbSftySwCfg_t newCfg);
	void _setSttChng();
   void _updBothHndsSwState();
   void _updFtSwState();
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
   * @param sftySwActvOtpPin (Optional) GPIO pin assigned to attach a device (indicator or other) to be activated when the switch signals ON. 
   * @param lftHndOtptPrm A swtchOtptHwCfg_t structure containing the hardware implemented characteristics for the left hand output indicators for isOn, isEnabled and isVoided attributes flags
   * @param rghtHndOtptPrm A swtchOtptHwCfg_t structure containing the hardware implemented characteristics for the right hand output indicators for isOn, isEnabled and isVoided attributes flags
   * @param ftOtptPrm A swtchOtptHwCfg_t structure containing the hardware implemented characteristics for the foot switch output indicators for isOn attribute flag
   * @param hndsSwtchsOkPin (Optional) GPIO pin assigned to attach a device (indicator or other) activated when the hands safety protocol is cumplimented
   */
  LimbsSftySnglShtSw(swtchInptHwCfg_t lftHndInpPrm,
                    swtchInptHwCfg_t rghtHndInpPrm,
                    swtchInptHwCfg_t ftInpPrm,
                    int8_t sftySwActvOtpPin,
                    swtchOtptHwCfg_t lftHndOtptPrm,
                    swtchOtptHwCfg_t rghtHndOtptPrm,
                    swtchOtptHwCfg_t ftOtptPrm,
                    int8_t hndsSwtchsOkPin = _InvalidPinNum
                    );
   ~LimbsSftySnglShtSw();
   bool begin(unsigned long int updtPeriod);
   void clrStatus();
   bool getBothHndsSwOk();
   // bool cnfgFtSwtch();
   // bool cnfgLftHndSwtch();
   // bool cnfgRghtHndSwtch();
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
#endif   //_LIMBSSAFETYSW_ESP32_H_
