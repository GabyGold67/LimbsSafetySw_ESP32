/**
  ******************************************************************************
  * @file   LimbsSafetySw_ESP32.h
  * @brief  Header file for the LimbsSafetySw_ESP32 library
  *
  * @details The library implements Limbs Safety Switches for industrial production machines and dangerous devices. The switches generated are ISO 13849-1 (2023) compliant (configuration dependant).
  * 
  * The switches generated includes the following properties:
  * - It accepts any input signals analog to those produced by an MPB, including but not limited to:
  *   - MPBs
  *   - Digital output pressure sensors
  *   - Proximity sensors
  *   - Infrared U-Slot Photoelectric Sensor
  * - Its main output signal is limited to:
  *   - Latch release Activated|Deactivated
  * - Its secondary outputs signals include:
  *   - Hands switches status
  *      - isOn state
  *      - isVoided state
  *      - isEnabled state
  *   - Foot switch isEnabled state indicator
  *   - Production Cycle Active|Inactive
  *
  * @author	: Gabriel D. Goldman
  * @version v1.0.0
  * @date First release: 11/11/2024 
  *       Last update:   30/01/2025 19:00 (GMT+0200)
  * 
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
#ifndef _HwMinDbncTime
   #define _HwMinDbncTime 20   //Documented minimum wait time for a MPB signal to stabilize
#endif
#define _stdTVMPBttnVoidTime 10000UL
#define _stdTVMPBttnDelayTime 0UL
#define _stdSSVMPBttnDelayTime 0UL
#define _minPollDelay 20UL

/*---------------- xTaskNotify() mechanism related constants BEGIN -------*/
const uint8_t lftHndSwtchIsEnbldBP{0x00};
const uint8_t lftHndSwtchIsOnBP{0x01};
const uint8_t lftHndSwtchIsVddBP{0x02};
const uint8_t rghtHndSwtchIsEnbldBP{0x03};
const uint8_t rghtHndSwtchIsOnBP{0x04};
const uint8_t rghtHndSwtchIsVddBP{0x05};
const uint8_t ftSwtchIsEnbldBP{0x06};
const uint8_t ftSwtchIsOnBP{0x07};
const uint8_t LsSwtchLtchRlsIsOnBP{0x08};
const uint8_t LsSwtchPrdCyclIsOnBP{0x09};
//=================================================>> END User defined constants

// Definition workaround to let a function/method return value to be a function pointer to a function that receives no arguments and returns no values: void (funcName*)()
typedef void (*fncPtrType)();
typedef fncPtrType (*ptrToTrnFnc)();

/* Definition workaround to let a function/method return value to be a function pointer to a function that receives a void* arguments and returns no values: void (funcName*)(void*)

Use sample (by Gemini):
void myFunction(void* data) {
   Cast the pointer to the appropiate data type before using it.
   Do whatever with the data in the function, 
}

Function that returns a function pointer to the function previously defined
ptrToTrnFncVdPtr getFunctionPointer() {
    fncVdPtrPrmPtrType ptr = myFunction;
    return ptr;
}
*/
typedef void (*fncVdPtrPrmPtrType)(void*);
typedef fncVdPtrPrmPtrType (*ptrToTrnFncVdPtr)(void*);

//===================================================>> BEGIN User defined types
/**
 * @struct gpioPinOtptHwCfg_t
 * @brief GPIO Generic Pin for Output Configuration data structure
 * 
 * Resource to keep the class design flexible to be used with different hardware components, this structure holds the output GPIO pin identification and a flag indicating if the voltage level needed to activate the device connected to the output pin. Polarity connection of leds, CC or CA RGB leds or led arrays, low or high level activated relays are examples of where the gpioPinOtptHwCfg_t is a convenient replacement to simple pin configuration as Output Mode.
 * 
 * @param gpioOtptPin Pin identified by pin number, to be used in OUTPUT mode. A value to indicate a non-connected pin is platform dependent
 * @param gpioOtptActHgh Boolean indicating if the device attached to the pin is activated by a high level signal (true) or a low level signal (false)
 */
struct gpioPinOtptHwCfg_t{
   int8_t gpioOtptPin = _InvalidPinNum;
   bool gpioOtptActHgh = true;
};

/**
 * @struct limbSftyFwConf_t
 * @brief Limbs Safety Firmware Configuration parameters
 * 
 * Holds the values needed for the configuration of parameters related with code execution.
 * 
 * @attention Software construction related!! The information is related to the general development parameters.
 * The core selection for the Task Execution might be provided by the hardware development team or automatically determined by O.S. as the core executing the applications.
 * 
 * @param lsSwExecTskCore Core where the code is expected to run
 * @param lsSwExecTskPrrtyCnfg Priority level for the tasks running the switch creation code, the different output updates code, including functions and other tasks related.
 */
struct limbSftyFwConf_t{
  BaseType_t lsSwExecTskCore = xPortGetCoreID();
  BaseType_t lsSwExecTskPrrtyCnfg = configTIMER_TASK_PRIORITY;
};

/**
 * @struct swtchBhvrCfg_t
 * @brief Limbs safety underlying switches behavior configuration parameters
 * 
 * Holds the required set of parameters needed for the configuration of each of the three DbncdMPBttn subclass switches needed for input (left hand's TmVdblMPBttn, right hand's TmVdblMPBttn, foot's SnglSrvcVdblMPBttn). 
 * Each parameter has default values assigned for a standard LimbsSftyLnFSwtch configuration.
 * 
 * @param swtchStrtDlyTm Corresponds to the DbncdMPBttn subclasses strtDelay class attribute.
 * @param swtchIsEnbld Corresponds to the DbncdMPBttn subclasses isEnabled attribute flag
 * @param swtchVdTm Corresponds to the TmVdblMPBttn class voidTime attribute, will be used as an activation time limit for the hands controlled TmVdblMPBttn
 * 
 * @note The provided default values are expected to be only used if no explicit values are provided by the object instantiating software. In a standard use case these values are expected to be saved from previous configured values, and be part of the configuration being set by supervisor level users.
 */
struct swtchBhvrCfg_t{
  unsigned long int swtchStrtDlyTm = _stdTVMPBttnDelayTime;
  bool swtchIsEnbld = true;
  unsigned long int swtchVdTm = _stdTVMPBttnVoidTime;
};

/**
 * @struct swtchInptHwCfg_t
 * @brief Switch Input Hardware Configuration parameters
 * 
 * Holds the hardware characteristics of each of the underlying composing switches used, data needed for those switches constructors.
 * As the attributes held in the structure are all hardware related they must be set or modified only when hardware construction or modification happens. The actual parameters must be saved in non volatile memory or by physical means (jumpers, dip switches, etc.). Only technical maintenance level operators might modify them.
 * 
 * @param inptPin GPIO pin number connected to the MPBttn
 * @param typeNO Type of switch, Normal Open (NO=true) or Normal Closed (NO=false). Default value: true
 * @param pulledUp Internal pull-up circuit configuration, default value true
 * @param dbncdTime Debounce process time required to receive a stable signal, default value 20 milliseconds
 * 
 * @note inptPin = GPIO_NUM_NC (-1) is used to indicate the pin is Not Connected (N/C). 
 * @note GPIO_NUM_MAX is used to indicate the maximum valid number for a GPIO pin (GPIO_NUM_x < GPIO_NUM_MAX)
 * @attention Hardware construction related!! The information must be provided by the hardware developers
 * @warning Not all the GPIO input pins of every MCU has input mode operation, nor all have a pull-up internal circuit available!
*/
struct swtchInptHwCfg_t{
   int8_t inptPin;
   bool typeNO = true;
   bool pulledUp = true;
   unsigned long int dbncTime = _HwMinDbncTime;
};

/**
 * @struct swtchOtptHwCfg_t
 * @brief Switch Output Hardware Configuration data structure
 * 
 * Holds the configuration parameters for the GPIO pins assigned to output the state of the isOn, isVoided and isEnabled attribute flag for the **underlying switches**. The configuration information is a gpioPinOtptHwCfg_t type value
 * 
 * @param isOnPin Holds the isOn flag attribute state output pin configuration properties
 * @param isVoidedPin Holds the isVoided flag attribute state output pin configuration properties
 * @param isEnabledPin Holds the isEnabled flag attribute state output pin configuration properties
 * 
 * @note The pins included in this structure are used for operator information related output. The use or lack of use of those pins are not related to the LimbsSftyLnFSwtch class objects behavior, as the attribute flags values that might be reflected by those pins are always computed, independently of being later used to activate hardware hints.
 * @note For the hands switches all three parameters are relevant, for the SnglSrvcVdblMPBttn foot switch only the isOnPin and the isEnabledPin parameters will be considered, being the values of the other parameter ignored.
 * 
 * @attention Hardware construction related!! The information must be provided by the hardware developers
 */
struct swtchOtptHwCfg_t{
   gpioPinOtptHwCfg_t isOnPin;  
   gpioPinOtptHwCfg_t isVoidedPin;  
   gpioPinOtptHwCfg_t isEnabledPin;  
};

/**
 * @struct lsSwtchOtpts_t
 * 
 * @brief Relevant Attribute Flags values data structure
 * 
 * Holds the values for the relevant attribute flags that define the Limbs Safety Switch outputs state.
 * 
 * @param lftHndIsEnbld Holds the value of the _isEnabled attribute flag for the _undrlLftHnd MPBttn
 * @param lftHndIsOn Holds the value of the _isOn attribute flag for the _undrlLftHnd MPBttn
 * @param lftHndIsVdd Holds the value of the _isVoided attribute flag for the _undrlLftHnd MPBttn
 * @param rghtHndIsEnbld Holds the value of the _isEnabled attribute flag for the _undrlRghtHnd MPBttn
 * @param rghtHndIsOn Holds the value of the _isOn attribute flag for the _undrlRghtHnd MPBttn
 * @param rghtHndIsVdd Holds the value of the _isVoided attribute flag for the _undrlRghtHnd MPBttn
 * @param ftSwIsEnbld Holds the computed value of the _undrlFtMPBPtr's _isEnabled attribute flag
 * @param ftSwIsOn Holds the computed value of the _undrlFtMPBPtr's _isOn attribute flag
 * @param ltchRlsIsOn Holds the value of the _ltchRlsIsOn attribute flag
 * @param prdCyclIsOn Holds the value of the _prdCyclIsOn attribute flag
 * 
 */
struct lsSwtchOtpts_t{
   // Underlying MPBttns AF values kept for practical use, might be changed in future development iterations as there are other resources to get these values
   bool lftHndIsEnbld;
   bool lftHndIsOn;
   bool lftHndIsVdd;
   bool rghtHndIsEnbld;
   bool rghtHndIsOn;
   bool rghtHndIsVdd;
   bool ftSwIsEnbld;
   bool ftSwIsOn;
   // LsSwtch AF specific values 
   bool ltchRlsIsOn;
   bool prdCyclIsOn;   
};

/**
 * @struct lsSwtchSwCfg_t
 * 
 * @brief Machine activation related attributes data structure
 * 
 * Holds the Cycle Machine working parameters that are relevant to the switch, the time to keep the latch release active, and the time to wait to consider a production cycle completed to close one production cycle and prepare the control for the next cycle.
 * 
 * @param ltchRlsActvTm Time -in milliseconds- to keep the latch release mechanism activated
 * @param prdCyclActvTm Time -in milliseconds- to wait before considering the Production Cycle completed
 * 
 * @note Both times are relative to the start of the latch release moment, and as such it's logical than the first parameter will be smaller or equal to the second.
 */
struct lsSwtchSwCfg_t{
   unsigned long int ltchRlsActvTm = 1500UL;
   unsigned long int prdCyclActvTm = 6000UL;  
};
//===================================================>> END User defined types

//======================================>> BEGIN General use function prototypes
lsSwtchOtpts_t lssOtptsSttsUnpkg(uint32_t pkgOtpts);
//========================================>> END General use function prototypes

//===========================>> BEGIN General use Static variables and constants
static BaseType_t xReturned; /*!<Static variable to keep returning result value from Tasks and Timers executions*/
static BaseType_t errorFlag {pdFALSE};
//=============================>> END General use Static variables and constants

//=================================================>> BEGIN Classes declarations
/**
 * @class LimbsSftyLnFSwtch
 * 
 * @brief Models a Limbs Safety Launch and Forget Switch (LimbsSftyLnFSwtch) for 
 * safely activation of **"launch and forget" cycle machines** and devices.
 * 
 * This industry grade machinery oriented switch enforces conditioned activation
 * of several switches to ensure limbs security. This enforced conditions 
 * include sequenced pressing order and timings as main parameters.
 * The most basic configuration requires each hand pressing simultaneously two
 * switches to enable a foot switch. The enabled foot switch activates the 
 * machine action.  
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
class LimbsSftyLnFSwtch{
private:
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
   swtchInptHwCfg_t _lftHndInpCfg{};
   swtchBhvrCfg_t _lftHndBhvrCfg{};
   swtchInptHwCfg_t _rghtHndInpCfg{};
   swtchBhvrCfg_t _rghtHndBhvrCfg{};
   swtchInptHwCfg_t _ftInpCfg{};
   swtchBhvrCfg_t _ftBhvrCfg{};

   MpbOtpts_t _lftHndSwtchStts{};
   TmVdblMPBttn* _undrlLftHndMPBPtr{nullptr};
   VdblMPBttn* _undrlLftHndBasePtr{nullptr};


   MpbOtpts_t _rghtHndSwtchStts{};
   TmVdblMPBttn* _undrlRghtHndMPBPtr{nullptr};
   VdblMPBttn* _undrlRghtHndBasePtr{nullptr};

   MpbOtpts_t _ftSwtchStts{};
   SnglSrvcVdblMPBttn* _undrlFtMPBPtr{nullptr};   

   unsigned long int _undrlSwtchsPollDelay{_minPollDelay};

   unsigned long int _curTimeMs{0};
   bool _ltchRlsIsOn{false};
   static bool _ltchRlsPndng;
   unsigned long int _ltchRlsTtlTm{0};
   bool _prdCyclIsOn{false};
   unsigned long int _prdCyclTmrStrt{0};
   unsigned long int _prdCyclTtlTm{0};  
	
   fncVdPtrPrmPtrType _fnWhnBthHndsOnMssd{nullptr};
   fncVdPtrPrmPtrType _fnWhnTrnOffLtchRls {nullptr};
	fncVdPtrPrmPtrType _fnWhnTrnOffPrdCycl {nullptr};
	fncVdPtrPrmPtrType _fnWhnTrnOnLtchRls {nullptr};
	fncVdPtrPrmPtrType _fnWhnTrnOnPrdCycl {nullptr};

	void* _fnWhnBthHndsOnMssdArg {nullptr};
   void* _fnWhnTrnOffLtchRlsArg {nullptr};
	void* _fnWhnTrnOffPrdCyclArg {nullptr};
   void* _fnWhnTrnOnLtchRlsArg{nullptr};
	void* _fnWhnTrnOnPrdCyclArg {nullptr};

   fdaLsSwtchStts _lsSwtchFdaState {stOffNotBHP};
   bool _lsSwtchOtptsChng{false};
   uint32_t _lsSwtchOtptsChngCnt{0};
   TimerHandle_t _lsSwtchPollTmrHndl {NULL};   //FreeRTOS returns NULL if creation fails (not nullptr)
   bool _sttChng{true};
   String _swtchPollTmrName{"lsSwtchPollTmr"};

   TaskHandle_t _tskToNtfyBthHndsOnMssd{NULL};
   TaskHandle_t _tskToNtfyLsSwtchOtptsChng{NULL};
   TaskHandle_t _tskToNtfyTrnOffLtchRls{NULL};
   TaskHandle_t _tskToNtfyTrnOffPrdCycl{NULL};
   TaskHandle_t _tskToNtfyTrnOnLtchRls{NULL};
   TaskHandle_t _tskToNtfyTrnOnPrdCycl{NULL};

	static void lsSwtchPollCb(TimerHandle_t lssTmrCbArg);

   void _ackBthHndsOnMssd();
   void _clrSttChng();
   bool _cnfgHndSwtch(const bool &isLeft, const swtchBhvrCfg_t &newCfg);
   void _getUndrlSwtchStts();
   uint32_t _lsSwtchOtptsSttsPkgd(uint32_t prevVal = 0);
	void _rstOtptsChngCnt();
   static void _setLtchRlsPndng();
   void _setSttChng();
   void _turnOffLtchRls();
   void _turnOnLtchRls();
   void _turnOffPrdCycl();
   void _turnOnPrdCycl();
   unsigned long int _updCurTimeMs();
   void _updFdaState();

public:
  /**
   * @brief Class default constructor
   * 
   */
  LimbsSftyLnFSwtch();
  /**
   * @brief Class constructor
   * 
   * The class models a Limbs Safety switch, a switch that gives an  activation signal to a machine or device when some independent switches, activated in a designated pattern, ensures no risk for the operator limbs is completed.
   * The constructor instantiates the DbncdMPBttn subclasses objects that compose the Limbs Safety switch, i.e. the left hand TmVdblMPBttn, the right hand TmVdblMPBttn and the foot SnglSrvcVdblMPBttn
   * 
   * @param lftHndInpCfg A swtchInptHwCfg_t structure containing the hardware implemented characteristics for the left hand controlled TmVdblMPBttn
   * @param lftHndBhvrCfg A swtchBhvrCfg_t structure containing the behavior characteristics for the left hand controlled TmVdblMPBttn
   * @param rghtHndInpCfg A swtchInptHwCfg_t structure containing the hardware implemented characteristics for the right hand controlled TmVdblMPBttn
   * @param rghtHndBhvrCfg A swtchBhvrCfg_t structure containing the behavior characteristics for the right hand controlled TmVdblMPBttn
   * @param ftInpCfg A swtchInptHwCfg_t structure containing the hardware implemented characteristics for the foot controlled SnglSrvcVdblMPBttn
   * @param ftBhvrCfg A swtchBhvrCfg_t structure containing the behavior characteristics for the foot controlled SnglSrvcVdblMPBttn
   * @param lsSwtchWrkngCnfg A lsSwtchSwCfg_t structure containing the basic parameters needed for the instantiated object to work. The parameters provided for instantiation might be changed by dedicated setters
   */
  LimbsSftyLnFSwtch(swtchInptHwCfg_t lftHndInpCfg,
                     swtchBhvrCfg_t lftHndBhvrCfg,
                     swtchInptHwCfg_t rghtHndInpCfg,
                     swtchBhvrCfg_t rghtHndBhvrCfg,
                     swtchInptHwCfg_t ftInpCfg,
                     swtchBhvrCfg_t ftBhvrCfg,
                     lsSwtchSwCfg_t lsSwtchWrkngCnfg
                    );
   /**
    * @brief Default virtual destructor
    * 
    */
   ~LimbsSftyLnFSwtch();
   /**
	 * @brief Attaches the instantiated object to a timer that monitors the input pins and updates the object status.
    * 
	 * The frequency of the periodic monitoring is passed as a parameter in milliseconds, and is a value that must be small (frequent) enough to keep the object updated, but not so frequent that wastes resources needed by other tasks. As the DbncdMPBttn objects components of the switch have a minimum default value, the same is provided for this method, as it makes no sense to check for changes in objects that refresh themselves slower than the checking length period.
    * 
    * @param pollDelayMs (Optional) unsigned long integer (ulong), the time between status updates in milliseconds.
    * @return The success in starting the updating timer with the provided update time
    * @retval true Timer starting operation success for the object and for the underlying DbncdMPBttn subclasses objects
    * @return false Timer starting operation failure for at least one of the four timers being started
    */
   bool begin(unsigned long int pollDelayMs = _minPollDelay);
   /**
	 * @brief Clears and resets flags, timers and counters modified through the object's signals processing.
	 *
	 * Resets object's attributes to its initialization values to safely continue operations after completing a FDA cycle that might generate unexpected distortions. This avoids risky behavior of the object due to dangling flags or partially consumed time counters.
    * 
    */
   void clrStatus();
   /**
    * @brief Configures the SnglSrvcVdblMPBttn class object used as **Foot Switch**
    * 
    * Some behavior attributes of the DbncdMPBttn subclasses objects components can be configured to adjust the behavior of the LimbsSftyLnFSwtch. In the case of the SnglSrvcVdblMPBttn used as **Foot Switch** the only attribute available for adjustment is the **start delay** value, used to adjust the time the foot switch must be kept pressed after the debounce period, before the switch accepts the input signal. This parameter is used to adjust the "sensibility" of the switch to mistaken, accidental or conditioned reflex presses.
    * 
    * @param newCfg A swtchBhvrCfg_t type structure, from which only the .swtchStrtDlyTm value will be used.
    */
   void cnfgFtSwtch(const swtchBhvrCfg_t &newCfg);
   /**
    * @brief Configures the TmVdblMPBttn class object used as **Left Hand Switch**
    * 
    * Some behavior attributes of the DbncdMPBttn subclasses objects components can be configured to adjust the behavior of the LimbsSftyLnFSwtch. In the case of the TmVdblMPBttn used as **Left Hand Switch** the attributes available for adjustment are:
    * - **Start Delay** (.swtchStrtDlyTm) value, used to adjust the time the hand switch must be kept pressed after the debounce period, before the switch accepts the input signal. This parameter is used to adjust the "sensibility" of the switch to mistaken, accidental or conditioned reflex presses.
    * - **Is Enabled** (.swtchIsEnbld) value, defines if the hand switch will be enabled, in which case it will be considered for the LimbsSftyLnFSwtch state calculation -having to be pressed at the exprected moment, for the expected time and be released when expected to restart the cycle- or disabled, in which case it being pressed or not makes no difference to the LimbsSftyLnFSwtch state calculation.
    * - **Switch Voiding Time** (.swtchVdTm) defines the time period the hand switch might be kept pressed before signaling it as voided, having to proceed to release it and press it back to return to the valid pressed (non-voided) state.
    * 
    * @param newCfg A swtchBhvrCfg_t type structure, containing the parameters values that will be used to modify the configuration of the TmVdblMPBttn class object. 
    * 
    * @warning The swtchBhvrCfg_t type structure has designated default field values, as a consequence any field not expressly filled with a valid value will be set to be filled with the default value. If not all the fields are to be changed, be sure to fill the non changing fields with the current value to ensure only the intended fields are to be changed!
    */
   bool cnfgLftHndSwtch(const swtchBhvrCfg_t &newCfg);
   /**
    * @brief Configures the TmVdblMPBttn class object used as **Right Hand Switch**
    * 
    * Some behavior attributes of the DbncdMPBttn subclasses objects components can be configured to adjust the behavior of the LimbsSftyLnFSwtch. In the case of the TmVdblMPBttn used as **Right Hand Switch** the attributes available for adjustment are:
    * - **Start Delay** (.swtchStrtDlyTm) value, used to adjust the time the hand switch must be kept pressed after the debounce period, before the switch accepts the input signal. This parameter is used to adjust the "sensibility" of the switch to mistaken, accidental or conditioned reflex presses.
    * - **Is Enabled** (.swtchIsEnbld) value, defines if the hand switch will be enabled, in which case it will be considered for the LimbsSftyLnFSwtch state calculation -having to be pressed at the exprected moment, for the expected time and be released when expected to restart the cycle- or disabled, in which case it being pressed or not makes no difference to the LimbsSftyLnFSwtch state calculation.
    * - **Switch Voiding Time** (.swtchVdTm) defines the time period the hand switch might be kept pressed before signaling it as voided, having to proceed to release it and press it back to return to the valid pressed (non-voided) state.
    * 
    * @param newCfg A swtchBhvrCfg_t type structure, containing the parameters values that will be used to modify the configuration of the TmVdblMPBttn class object. 
    * 
    * @warning The swtchBhvrCfg_t type structure has designated default field values, as a consequence any field not expressly filled with a valid value will be set to be filled with the default value. If not all the fields are to be changed, be sure to fill the non changing fields with the current value to ensure only the intended fields are to be changed!
    */
   bool cnfgRghtHndSwtch(const swtchBhvrCfg_t &newCfg);
	/**
	 * @brief Returns the function that is set to execute every time the object's Latch Release is set to **Off State**.
	 *
	 * The function to be executed is an attribute that might be modified by the **setFnWhnTrnOffLtchRlsPtr()** method. If such function is set, it will be executed every time the **_ltchRlsIsOn** attribute flag is set to **false**
	 *
	 * @return A pointer to the function set to execute every time the object's indicated attribute flag value is set as described.
	 * @retval nullptr if there is no function set to execute when the object enters the indicated condition.
	 *
	 * @warning The function code execution will become part of the list of procedures the object executes when entering the indicated condition, including the modification of the affected attribute flags, the unblocking of a task and (maybe) others. Making the function code too time demanding must be handled with care as execution of the function might block other tasks with lower execution priority.
	 */
   fncVdPtrPrmPtrType getFnWhnTrnOffLtchRlsPtr();
	/**
	 * @brief Returns the function that is set to execute every time the object's Production Cycle is set to **Off State**.
	 *
	 * The function to be executed is an attribute that might be modified by the **setFnWhnTrnOffPrdCyclPtr()** method. If such function is set, it will be executed every time the **_prdCyclIsOn** attribute flag is set to **false**
	 *
	 * @return A pointer to the function set to execute every time the object's indicated attribute flag value is set as described.
	 * @retval nullptr if there is no function set to execute when the object enters the indicated condition.
	 *
	 * @warning The function code execution will become part of the list of procedures the object executes when entering the indicated condition, including the modification of the affected attribute flags, the unblocking of a task and (maybe) others. Making the function code too time demanding must be handled with care as execution of the function might block other tasks with lower execution priority.
	 */
   fncVdPtrPrmPtrType getFnWhnTrnOffPrdCyclPtr();
	/**
	 * @brief Returns the function that is set to execute every time the object's Latch Release is set to **On State**.
	 *
	 * The function to be executed is an attribute that might be modified by the **setFnWhnTrnOnLtchRlsPtr()** method. If such function is set, it will be executed every time the **_ltchRlsIsOn** attribute flag is set to **true**
	 *
	 * @return A pointer to the function set to execute every time the object's indicated attribute flag value is set as described.
	 * @retval nullptr if there is no function set to execute when the object enters the indicated condition.
	 *
	 * @warning The function code execution will become part of the list of procedures the object executes when entering the indicated condition, including the modification of the affected attribute flags, the unblocking of a task and (maybe) others. Making the function code too time demanding must be handled with care as execution of the function might block other tasks with lower execution priority.
	 */
   fncVdPtrPrmPtrType getFnWhnTrnOnLtchRlsPtr();
	/**
	 * @brief Returns the function that is set to execute every time the object's Production Cycle is set to **On State**.
	 *
	 * The function to be executed is an attribute that might be modified by the **setFnWhnTrnOnPrdCyclPtr()** method. If such function is set, it will be executed every time the **_prdCyclIsOn** attribute flag is set to **true**
	 *
	 * @return A pointer to the function set to execute every time the object's indicated attribute flag value is set as described.
	 * @retval nullptr if there is no function set to execute when the object enters the indicated condition.
	 *
	 * @warning The function code execution will become part of the list of procedures the object executes when it entering the indicated condition, including the modification of the affected attribute flags, the unblocking of a task and (maybe) others. Making the function code too time demanding must be handled with care as execution of the function might block other tasks with lower execution priority.
	 */
   fncVdPtrPrmPtrType getFnWhnTrnOnPrdCyclPtr();
   /**
    * @brief Get the ftSwcthPtr attribute value
    * 
    * The ftSwcthPtr is the pointer to the SnglSrvcVdblMPBttn class object instantiated to be the "Foot Safety Switch", so to have direct access to it's public members without going through a LimbsSftyLnFSwtch interface.
    * 
    * @return The SnglSrvcVdblMPBttn class pointer to the foot switch
    * 
    * @warning The open access to the underlying SnglSrvcVdblMPBttn complete set of public members may imply risks by letting the developer modify some attributes of the underlying object in unexpected ways. 
    */
   SnglSrvcVdblMPBttn* getFtSwtchPtr();
   /**
    * @brief Get the lftHndSwcthPtr attribute value
    * 
    * The lftHndSwcthPtr is the pointer to the TmVdblMPBttn class object instantiated to be the "Left Hand Safety Switch", so to have direct access to it's public members without going through a LimbsSftyLnFSwtch interface.
    * 
    * @return The TmVdblMPBttn class pointer to the left hand switch
    * 
    * @warning The open access to the underlying TmVdblMPBttn complete set of public members may imply risks by letting the developer to modify some attributes of the underlying object in unexpected ways.
    */
   TmVdblMPBttn*  getLftHndSwtchPtr();
   /**
	 * @brief Returns the value of the **lsSwtchOtptsChng** attribute flag.
	 *
	 * The instantiated objects include attributes linked to their evaluated states, -Attribute Flags- some of them for internal use, some of them for **output related purposes**.
	 * When any of those latter attributes values change, the **outputsChange** flag is set. The flag only signals changes have happened -not which flags, nor how many times changes have taken place- since the last **outputsChange** flag reset, although an internal counter is kept to grant no multithread race conditions affect the correct execution of the outputs updates.
	 * The **outputsChange** flag must be reset (or set if desired) through the setOutputsChange() method.
	 *
    * @retval true: Any of the object's output related behavior flags have changed value since last time **outputsChange** flag was reset.
    * @retval false: no object's output related behavior flags have changed value since last time **outputsChange** flag was reset.
	 */
   const bool getLsSwtchOtptsChng() const;
   /**
    * @brief Returns the relevant attribute flags values for the object state encoded as a 32 bits value, required to pass current state of the object to another thread/task managing the outputs
    *
    * The inter-tasks communication mechanisms implemented on the class includes a xTaskNotify() that works as a light-weigh mailbox, unblocking the receiving tasks and sending to it a 32_bit value notification. This function returns the relevant attribute flags values encoded in a 32 bit value, according the provided encoding documented.
    *
    * @return A 32-bit unsigned value representing the attribute flags current values.
    * 
    * @note For ease of use and resources optimization the encoded 32 bits value includes state information referenced to the underlying DbncdMPBttn subclasses objects isEnabled and isOn for each one of them. Also the bits positions used for relevant information are not used by the underlying objects analog mechanisms, even if no interaction or interference is possible.
    */
   uint32_t getLsSwtchOtptsSttsPkgd();
   /**
    * @brief Returns the ltchRlsIsOn attribute flag value
    * 
    * The ltchRlsIsOn attribute flag indicates if the object is in the latch released state.
    * 
    * @retval true The object is in the latch released state
    * @retval false The object is in the latch not released state
    */
   const bool getLtchRlsIsOn() const;
   /**
    * @brief Returns the time configured to keep the latch at the released state
    * 
    * @return The time in milliseconds the latch will be kept released
    */
   unsigned long int getLtchRlsTtlTm();
   /**
    * @brief Get the prdCyclIsOn object'sattribute flag value
    * 
    * The prdCyclIsOn attribute flag indicates if the object is in the production cycle state.
    * 
    * @retval true The object is in the production cycle state
    * @retval false The object is not in the production cycle state
    */
   const bool getPrdCyclIsOn() const;
   /**
    * @brief Returns the time configured for the production cycle to be completed
    * 
    * Due to the lack of sensors to register the end/completetion of the production cycle a timer will be set to control the time since the production cycle start and until the cycle is considered completed. This method returns the time configured for that timer
    * 
    * @return The time in milliseconds the control will consider being in the production cycle state. After completing the time the cycle will be considered concluded and the limbs safety switches will be re-enabled to start a new cycle.
    */
   unsigned long int getPrdCyclTtlTm();  
   /**
    * @brief Get the rghtHndSwcthPtr attribute value
    * 
    * The rghtHndSwcthPtr is the pointer to the TmVdblMPBttn class object instantiated to be the "Right Hand Safety Switch", so to have direct access to it's setters and getters without going through a LimbsSftyLnFSwtch interface.
    * 
    * @return The TmVdblMPBttn class pointer to the right hand switch
    * 
    * @warning The open access to the underlying TmVdblMPBttn complete set of public members may imply risks by letting the developer to modify some attributes of the underlying object in unexpected ways. The only way to avoid such risks is by blocking this method and replacing the needed objects setters and getters through an in-class interface.
    */
   TmVdblMPBttn*  getRghtHndSwtchPtr();
   /**
	 * @brief Returns the TaskHandle for the task to be unblocked when the object's state changes from the "foot switch enabled" to the "foot switch disabled" instead of the "Production cycle activated" state.
    * 
    * When the object changes it's state as mentioned, it means an opportunity to activate the production cycle, which is the main purpose of the use of the machine, was wasted. The opportunity of unblocking of a task blocked by a xTaskNotifyWait() as part of that state transition gives a tool to deal with this situations. Either to react immediately, either to register this situations, as the proportion of these failures compared to the expected behavior may indicate a potential anomaly either in the parameters configuration, command switches failures and other cases that harm present productivity and might end in a future machine down time. This method returns the TaskHandle for the task to unblock.
    * 
    * @note When the value returned is NULL, the task notification mechanism is disabled. The mechanism can be enabled by setting a valid TaskHandle value by using the setTskToNtfyBthHndsOnMssd(const TaskHandle_t &newTaskHandle) method.
	 */   
   const TaskHandle_t getTskToNtfyBthHndsOnMssd() const;   
   /**
	 * @brief Returns the TaskHandle for the task to be unblocked when the object's lsSwtchOtptsChng attribute flags is set to true
    * 
    * One of the optional mechanisms activated when any attribute flag that includes the mechanism to modify an output pin is the unblocking of a task blocked by a xTaskNotifyWait(). This method returns the TaskHandle for the task to unblock.
    * 
    * @note When the value returned is NULL, the task notification mechanism is disabled. The mechanism can be enabled by setting a valid TaskHandle value by using the setTskToNtfyLsSwtchOtptsChng(const TaskHandle_t &newTaskHandle) method.
	 */
   const TaskHandle_t getTskToNtfyLsSwtchOtptsChng() const;
	/**
	 * @brief Returns the TaskHandle for the task to be unblocked when the object's ltchRlsIsOn attribute flag is set to false
    * 
    * One of the optional mechanisms activated by entering the **Latch Release is Off** (ltchRlsIsOn = false) state is the unblocking of a task blocked by a xTaskNotifyWait(). This method returns the TaskHandle for the task to unblock.
    * 
    * @note When the value returned is NULL, the task notification mechanism is disabled. The mechanism can be enabled by setting a valid TaskHandle value by using the setTskToNtfyTrnOffLtchRls(const TaskHandle_t &newTaskHandle) method.
	 */
   const TaskHandle_t getTskToNtfyTrnOffLtchRls() const;
	/**
	 * @brief Returns the TaskHandle for the task to be unblocked when the object's prdCyclIsOn attribute flag is set to false
    * 
    * One of the optional mechanisms activated by entering the **Production cycle is Off** (prdCyclIsOn = false) state is the unblocking of a task blocked by a xTaskNotifyWait(). This method returns the TaskHandle for the task to unblock.
    * 
    * @note When the value returned is NULL, the task notification mechanism is disabled. The mechanism can be enabled by setting a valid TaskHandle value by using the setTskToNtfyTrnOffPrdCycl(const TaskHandle_t &newTaskHandle) method.
	 */
   const TaskHandle_t getTskToNtfyTrnOffPrdCycl() const;
	/**
	 * @brief Returns the TaskHandle for the task to be unblocked when the object's prdCyclIsOn attribute flag is set to true
    * 
    * One of the optional mechanisms activated by entering the **Latch Release is On** (ltchRlsIsOn = true) state is the unblocking of a task blocked by a xTaskNotifyWait(). This method returns the TaskHandle for the task to unblock.
    * 
    * @note When the value returned is NULL, the task notification mechanism is disabled. The mechanism can be enabled by setting a valid TaskHandle value by using the setTskToNtfyTrnOnLtchRls(const TaskHandle_t &newTaskHandle) method.
	 */
   const TaskHandle_t getTskToNtfyTrnOnLtchRls() const;
	/**
	 * @brief Returns the TaskHandle for the task to be unblocked when the object's prdCyclIsOn attribute flag is set to true
    * 
    * One of the optional mechanisms activated by entering the **Production cycle is On** (prdCyclIsOn = true) state is the unblocking of a task blocked by a xTaskNotifyWait(). This method returns the TaskHandle for the task to unblock.
    * 
    * @note When the value returned is NULL, the task notification mechanism is disabled. The mechanism can be enabled by setting a valid TaskHandle value by using the setTskToNtfyTrnOnPrdCycl(const TaskHandle_t &newTaskHandle) method.
	 */
   const TaskHandle_t getTskToNtfyTrnOnPrdCycl() const;
	/**
	 * @brief Resets the LsSwitch behavior automaton to it's **Initial** or **Start State**
	 *
	 * This method is provided for security and for error handling purposes, so that in case of unexpected situations detected, the driving **Deterministic Finite Automaton** used to compute the objects' states might be reset to it's initial state to safely restart it, maybe as part of an **Error Handling** procedure.
	 */
   void resetFda();
	/**
	 * @brief Sets the function to be executed when the object's state changes from the "foot switch enabled" to the "foot switch disabled" instead of the "Production cycle activated" state.
    * 
    * When the object changes it's state as mentioned, it means an opportunity to activate the production cycle, which is the main purpose of the use of the machine, was wasted. The opportunity to execute a function as part of that state transition gives a tool to register this situation, as the proportion of these failures compared to the expected behavior may indicate an anomaly either in the parameters configuration, command switches failures and other cases that harm present productivity and might end in a future machine down time.
	 * 
	 * @param newFnWhnBthHndsOnMssd Function pointer to the function to be executed when the object makes the transition from  "foot switch enabled" to the "foot switch disabled" instead of the "Production cycle activated" state.
    * 
    * @note When the object is instantiated the function pointer is set to nullptr, value that disables the mechanism. Once a pointer to a function is provided the mechanism will become available. The mechanism can be disabled by setting the pointer value back to nullptr.
	 */
	void setFnWhnBthHndsOnMssd(fncVdPtrPrmPtrType &newFnWhnBthHndsOnMssd);
   /**
	 * @brief Sets the function to be executed when the object's ltchRlsIsOn attribute flag is set to false.
    * 
    * One of the optional mechanisms activated by entering the **Latch Release is Off** (ltchRlsIsOn = false) state is the execution of a fncVdPtrPrmPtrType function, i.e. void (*FnWhnTrnOff)(void*). This method sets the pointer to the function.
    * 
	 * @param newFnWhnTrnOff Function pointer to the function to be executed when the object enters the "Latch Release is Off" state.
    * 
    * @note When the object is instantiated the function pointer is set to nullptr, value that disables the mechanism. Once a pointer to a function is provided the mechanism will become available. The mechanism can be disabled by setting the pointer value back to nullptr.
	 */
	void setFnWhnTrnOffLtchRlsPtr(fncVdPtrPrmPtrType &newFnWhnTrnOff);
	/**
	 * @brief Sets the function to be executed when the object's prdCyclIsOn attribute flag is set to false
    * 
    * One of the optional mechanisms activated by entering the **Production Cycle is Off** (_prdCyclIsOn = false) state is the execution of a fncVdPtrPrmPtrType function, i.e. void (*FnWhnTrnOff)(void*). This method sets the pointer to the function.
    * 
	 * @param newFnWhnTrnOff Function pointer to the function to be executed when the object enters the "Production Cycle is Off" state.
	 * 
    * @note When the object is instantiated the function pointer is set to nullptr, value that disables the mechanism. Once a pointer to a function is provided the mechanism will become available. The mechanism can be disabled by setting the pointer value back to nullptr.
    */
	void setFnWhnTrnOffPrdCyclPtr(fncVdPtrPrmPtrType &newFnWhnTrnOff);
	/**
	 * @brief Sets the function to be executed when the object's ltchRlsIsOn attribute flag is set to true
    * 
    * One of the optional mechanisms activated by entering the **Latch Release is On** (ltchRlsIsOn = true) state is the execution of a fncVdPtrPrmPtrType function, i.e. void (*FnWhnTrnOn)(void*). This method sets the pointer to the function.
    * 
	 * @param newFnWhnTrnOn Function pointer to the function to be executed when the object enters the "Latch Release is On" state.
    * @note When the object is instantiated the function pointer is set to nullptr, value that disables the mechanism. Once a pointer to a function is provided the mechanism will become available. The mechanism can be disabled by setting the pointer value back to nullptr.
	 */
	void setFnWhnTrnOnLtchRlsPtr(fncVdPtrPrmPtrType &newFnWhnTrnOn);
	/**
	 * @brief Sets the function to be executed when the object's prdCyclIsOn attribute flag is set to true
    * 
    * One of the optional mechanisms activated by entering the **Production Cycle is On** (_prdCyclIsOn = true) state is the execution of a fncVdPtrPrmPtrType function, i.e. void (*FnWhnTrnOff)(void*). This method sets the pointer to the function.
    * 
	 * @param newFnWhnTrnOff Function pointer to the function to be executed when the object enters the "Production Cycle is Off" state.
    * @note When the object is instantiated the function pointer is set to nullptr, value that disables the mechanism. Once a pointer to a function is provided the mechanism will become available. The mechanism can be disabled by setting the pointer value back to nullptr.
	 */
	void setFnWhnTrnOnPrdCyclPtr(fncVdPtrPrmPtrType &newFnWhnTrnOn);
   /**
	 * @brief Sets the value of the attribute flag indicating if a change took place in any of the output attribute flags
	 *
	 * The usual path for the **lsSwtchOtptsChng** flag is to be set by any method changing an output attribute flag, the callback function signaled to take care of the hardware actions because of this changes clears back **lsSwtchOtptsChng** after taking care of them. In the unusual case the developer wants to "intercept" this sequence, this method is provided to set (true) or clear (false) lsSwtchOtptsChng value.
    *
    * @param newLsSwtchOtptsChng The new value to set the **lsSwtchOtptsChng** flag to.
    */
	void setLsSwtchOtptsChng(bool newLsSwtchOtptsChng);
   /**
    * @brief Set the Latch Release Total Time (ltchRlsTtlTm) attribute value
    * 
    * The ltchRlsTtlTm attribute holds the time the latching mechanism of the cycle machine will be freed to start the production cycle. Due to the primitive mechanical characteristics of these machines, the mechanical latching mechanism might take different times to be effectively released, so the time must be enough to ensure the correct and full unlatch, but not long enough to keep the machine unlatched when the production cycle is completed, as this might generate the next production cycle to start again, now with no limbs protection provided.
    * 
    * @param newVal Time in milliseconds to keep the unlatch mechanism activated, must be a value greater than 0, and less or equal to the "Production cycle time" (see setPrdCyclTtlTm(const unsigned long int))
    * @return true The parameter value was in the valid range, attribute value updated.
    * @return false The parameter value was not in the valid range, attribute value was not updated.
    */
   bool setLtchRlsTtlTm(const unsigned long int &newVal);
   /**
    * @brief Set the Production Cycle Total Time (prdCyclTtlTm) attribute value
    * 
    * The prdCyclTtlTm attribute holds the total time for the production cycle, starting from  the moment the latching mechanism of the cycle machine will be freed to start the production cycle and until the hands and foot security switches are kept disabled. After the production cycle timer set time is consumed, the limbs security switch enters the Cycle closure state, ending with the hands and foot security switches re-enabled to their respective configuration states. Setting a short value to the attribute will produce the switch to be re-enabled before the production cycle ends, generating undesired security risks, setting an extremely long value to the attribute will produce the switch to be unavailable to start a new production cycle, slowing the production in an unneeded manner. 
    * 
    * @param newVal Time in milliseconds for the production cycle to be active, must be a value greater than 0, and greater or equal to the "Latch Release Total Time" (see setLtchRlsTtlTm(const unsigned long int))
    * @return true The parameter value was in the valid range, attribute value updated.
    * @return false The parameter value was not in the valid range, attribute value was not updated.
    */
   bool setPrdCyclTtlTm(const unsigned long int &newVal);
   /**
    * @brief Sets the pointer to the arguments for the function to be executed when the object's ltchRlsIsOn attribute flag is set to false
    * 
    * The function to be executed is the one set by the setFnWhnTrnOffLtchRlsPtr(fncVdPtrPrmPtrType) method, that expects a void* argument.
    * 
    * @param newVal A void* to the argument to be passed to the mentioned function
    * 
    * @note When no arguments are expected to be passed the newVal value must be nullptr, that is the instantiation default value set.
    */
   void setTrnOffLtchRlsArgPtr(void* &newVal);
   /**
    * @brief Sets the pointer to the arguments for the function to be executed when the object's prdCyclIsOn attribute flag is set to false
    * 
    * The function to be executed is the one set by the setFnWhnTrnOffPrdCyclPtr(fncVdPtrPrmPtrType) method, that expects a void* argument.
    * 
    * @param newVal A void* to the argument to be passed to the mentioned function
    * 
    * @note When no arguments are expected to be passed the newVal value must be nullptr, that is the instantiation default value set.
    */
	void setTrnOffPrdCyclArgPtr(void* &newVal);
   /**
    * @brief Sets the pointer to the arguments for the function to be executed when the object's ltchRlsIsOn attribute flag is set to true
    * 
    * The function to be executed is the one set by the setFnWhnTrnOnLtchRlsPtr(fncVdPtrPrmPtrType) method, that expects a void* argument.
    * 
    * @param newVal A void* to the argument to be passed to the mentioned function
    * 
    * @note When no arguments are expected to be passed the newVal value must be nullptr, that is the instantiation default value set.
    */
   void setTrnOnLtchRlsArgPtr(void* &newVal);
   /**
    * @brief Sets the pointer to the arguments for the function to be executed when the object's prdCyclIsOn attribute flag is set to true
    * 
    * The function to be executed is the one set by the setFnWhnTrnOnPrdCyclPtr(fncVdPtrPrmPtrType) method, that expects a void* argument.
    * 
    * @param newVal A void* to the argument to be passed to the mentioned function
    * 
    * @note When no arguments are expected to be passed the newVal value must be nullptr, that is the instantiation default value set.
    */
	void setTrnOnPrdCyclArgPtr(void* &newVal);	
	/**
	 * @brief Sets the task to be unblocked -by a xTaskNotify()- when the object's state changes from the "foot switch enabled" to the "foot switch disabled" instead of the "Production cycle activated" state.
    * 
    * One of the optional mechanisms activated when the object's state changes from the "foot switch enabled" to the "foot switch disabled" instead of the "Production cycle activated" state is the unblocking of a task blocked by a xTaskNotifyWait(). This method sets the TaskHandle to the task to unblock.
    * When the object changes it's state as mentioned, it means an opportunity to activate the production cycle, which is the main purpose of the use of the machine, was wasted. The opportunity of unblocking of a task blocked by a xTaskNotifyWait() as part of that state transition gives a tool to deal with this situations. Either to react immediately, either to register this situations, as the proportion of these failures compared to the expected behavior may indicate a potential anomaly either in the parameters configuration, command switches failures and other cases that harm present productivity and might end in a future machine down time. This method sets the TaskHandle for the task to unblock.
    * 
	 * @param newTaskHandle The task handle of the task to be unblocked when the object's state changes from the "foot switch enabled" to the "foot switch disabled" instead of the "Production cycle activated" state.
    * 
    * @note When the object is instantiated the task handle is set to NULL, value that disables the mechanism. Once a TaskHandle to a task is provided the mechanism will become available. The mechanism can be disabled by setting the TaskHandle value back to NULL.
    * 
    * @attention Setting a task handle by using this method does not verify if the task handle value set corresponds to an existing, not suspended, not in delete process task. The task setting and valid state to be used by the corresponding method -by executing a xTaskNotify()- is under responsibility of the developer.  
    * 
    * @attention Setting a task handle by using this method to replace an already set TaskHandle_t value will check the status of the previous set task, and if it wasn't in a state of pending deletion proceed to suspend it and set the TaskHandle_t value to the new one. The suspended task will be left to be dealt by the developer, and the resources lost by no deleting that task might be considered under the developer responsibility and design decision.
	 */
   void setTskToNtfyBthHndsOnMssd(const TaskHandle_t &newTaskHandle);   
	/**
	 * @brief Sets the task to be unblocked -by a xTaskNotify()- when **ANY** of the object's attribute flags changes it's value
    * 
    * One of the optional mechanisms activated when any of the relevant attribute flags changes it's state is the unblocking of a task blocked by a xTaskNotifyWait(). This method sets the TaskHandle to the task to unblock. This tool is of great importance for the integration between the electronic logic control and the mechanical device it is intended to control. While the "Latch Release status" (ltchRlsIsOn AF) and the "Production Cycle status" (prdCyclIsOn AF) have their own dedicated tasks assignation capabilities, this generic task handle provides for all the relevant AFs, including both aforementioned.
    * 
	 * @param newTaskHandle The task handle of the task to be unblocked when one or more of the object's relevant AF changes it's value.
    * 
    * @note When the object is instantiated the task handle is set to NULL, value that disables the mechanism. Once a TaskHandle to a task is provided the mechanism will become available. The mechanism can be disabled by setting the TaskHandle value back to NULL.
    * 
    * @attention Setting a task handle by using this method does not verify if the task handle value set corresponds to an existing, not suspended, not in delete process task. The task setting and valid state to be used by the corresponding method -by executing a xTaskNotify()- is under responsibility of the developer.  
    * 
    * @attention Setting a task handle by using this method to replace an already set TaskHandle_t value will check the status of the previous set task, and if it wasn't in a state of pending deletion proceed to suspend it and set the TaskHandle_t value to the new one. The suspended task will be left to be dealt by the developer, and the resources lost by no deleting that task might be considered under the developer responsibility and design decision.
	 */
   void setTskToNtfyLsSwtchOtptsChng(const TaskHandle_t &newTaskHandle);
	/**
	 * @brief Sets the task to be unblocked -by a xTaskNotify()- when the object's ltchRlsIsOn attribute flag is set to false
    * 
    * One of the optional mechanisms activated by entering the **Latch Release is Off** (ltchRlsIsOn = false) state is the unblocking of a task blocked by a xTaskNotifyWait(). This method sets the TaskHandle to the task to unblock.
    * 
	 * @param newTaskHandle The task handle of the task to be unblocked when the object enters the "Latch Release is Off" state.
    * 
    * @note When the object is instantiated the task handle is set to NULL, value that disables the mechanism. Once a TaskHandle to a task is provided the mechanism will become available. The mechanism can be disabled by setting the TaskHandle value back to NULL.
    * 
    * @attention Setting a task handle by using this method does not verify if the task handle value set corresponds to an existing, not suspended, not in delete process task. The task setting and valid state to be used by the corresponding method -by executing a xTaskNotify()- is under responsibility of the developer.  
    * 
    * @attention Setting a task handle by using this method to replace an already set TaskHandle_t value will check the status of the previous set task, and if it wasn't in a state of pending deletion proceed to suspend it and set the TaskHandle_t value to the new one. The suspended task will be left to be dealt by the developer, and the resources lost by no deleting that task might be considered under the developer responsibility and design decision.
	 */
   void setTskToNtfyTrnOffLtchRls(const TaskHandle_t &newTaskHandle);    
	/**
	 * @brief Sets the task to be unblocked -by a xTaskNotify()- when the object's prdCyclIsOn attribute flag is set to false
    * 
    * One of the optional mechanisms activated by entering the **Production cycle is Off** (prdCyclIsOn = false) state is the unblocking of a task blocked by a xTaskNotifyWait(). This method sets the TaskHandle to the task to unblock.
    * 
	 * @param newTaskHandle The task handle of the task to be unblocked when the object enters the "Production cycle is Off" state.
    * 
    * @note When the object is instantiated the task handle is set to NULL, value that disables the mechanism. Once a TaskHandle to a task is provided the mechanism will become available. The mechanism can be disabled by setting the TaskHandle value back to NULL.
    * 
    * @attention Setting a task handle by using this method does not verify if the task handle value set corresponds to an existing, not suspended, not in delete process task. The task setting and valid state to be used by the corresponding method -by executing a xTaskNotify()- is under responsibility of the developer.  
    * 
    * @attention Setting a task handle by using this method to replace an already set TaskHandle_t value will check the status of the previous set task, and if it wasn't in a state of pending deletion proceed to suspend it and set the TaskHandle_t value to the new one. The suspended task will be left to be dealt by the developer, and the resources lost by no deleting that task might be considered under the developer responsibility and design decision.
	 */
	void setTskToNtfyTrnOffPrdCycl(const TaskHandle_t &newTaskHandle);    
	/**
	 * @brief Sets the task to be unblocked -by a xTaskNotify()- when the object's ltchRlsIsOn attribute flag is set to true
    * 
    * One of the optional mechanisms activated by entering the **Latch Release is On** (ltchRlsIsOn = true) state is the unblocking of a task blocked by a xTaskNotifyWait(). This method sets the TaskHandle to the task to unblock.
    * 
	 * @param newTaskHandle The task handle of the task to be unblocked when the object enters the "Latch Release is On" state.
    * 
    * @note When the object is instantiated the task handle is set to NULL, value that disables the mechanism. Once a TaskHandle to a task is provided the mechanism will become available. The mechanism can be disabled by setting the TaskHandle value back to NULL.
    * 
    * @attention Setting a task handle by using this method does not verify if the task handle value set corresponds to an existing, not suspended, not in delete process task. The task setting and valid state to be used by the corresponding method -by executing a xTaskNotify()- is under responsibility of the developer.  
    * 
    * @attention Setting a task handle by using this method to replace an already set TaskHandle_t value will check the status of the previous set task, and if it wasn't in a state of pending deletion proceed to suspend it and set the TaskHandle_t value to the new one. The suspended task will be left to be dealt by the developer, and the resources lost by no deleting that task might be considered under the developer responsibility and design decision.
	 */
   void setTskToNtfyTrnOnLtchRls(const TaskHandle_t &newTaskHandle);    
	/**
	 * @brief Sets the task to be unblocked -by a xTaskNotify()- when the object's prdCyclIsOn attribute flag is set to true
    * 
    * One of the optional mechanisms activated by entering the **Production cycle is On** (prdCyclIsOn = true) state is the unblocking of a task blocked by a xTaskNotifyWait(). This method sets the TaskHandle to the task to unblock.
    * 
	 * @param newTaskHandle The task handle of the task to be unblocked when the object enters the "Production cycle is On" state.
    * 
    * @note When the object is instantiated the task handle is set to NULL, value that disables the mechanism. Once a TaskHandle to a task is provided the mechanism will become available. The mechanism can be disabled by setting the TaskHandle value back to NULL.
    * 
    * @attention Setting a task handle by using this method does not verify if the task handle value set corresponds to an existing, not suspended, not in delete process task. The task setting and valid state to be used by the corresponding method -by executing a xTaskNotify()- is under responsibility of the developer.  
    * 
    * @attention Setting a task handle by using this method to replace an already set TaskHandle_t value will check the status of the previous set task, and if it wasn't in a state of pending deletion proceed to suspend it and set the TaskHandle_t value to the new one. The suspended task will be left to be dealt by the developer, and the resources lost by no deleting that task might be considered under the developer responsibility and design decision.
	 */
	void setTskToNtfyTrnOnPrdCycl(const TaskHandle_t &newTaskHandle);    
   /**
    * @brief Sets the update period length for the DbncdMPBttn subclasses objects used as input by the LimbsSftyLnFSwtch
    * 
    * Sets the periodic timer used to check the inputs and calculate the state of the object, time period value set in milliseconds.
    * 
    * @param newVal Value to set for the update period, the period must be greater than the minimum debounce default value.
    * @return The success in setting the new value to be used to start the DbncdMPBttn subclasses state updates
    * @retval true The value was in the accepted range and successfully changed
    * @retval false The value was not in the accepted range and successfully changed
    * @warning After the begin(unsigned long int) method is executed no other method is implemented to change the periodic update time, so this method must be used -if there's intention of using a non default value- **before** the begin(unsigned long int). Changing the value of the update period after executing the begin method will have no effect on the object's behavior.  
    */
   bool setUndrlSwtchsPollDelay(const unsigned long int &newVal);
};

//===================================================>> END Classes declarations

#endif   //_LIMBSSAFETYSW_ESP32_H_
