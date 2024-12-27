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
#define _minPollDelay 20UL
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

struct lsSwtchOtpts_t{
   bool bthHndsSwArOn;
   bool ltchRlsIsOn;
   bool prdCyclIsOn;
};
//===================================================>> END User defined types

//======================================>> BEGIN General use class prototypes
class LimbsSftySnglShtSwHI;
//========================================>> END General use class prototypes

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
 * @brief Models a Limbs Safety Single Shot Switch (LimbsSftySnglShtSw) for 
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

   LimbsSftySnglShtSwHI* _pnOtHWUpdtrPtr{nullptr};

   unsigned long int _undrlSwtchsPollDelay{_minPollDelay};

   unsigned long int _curTimeMs{0};
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
   void _turnOffLtchRls();
   void _turnOnLtchRls();
   void _turnOffPrdCycl();
   void _turnOnPrdCycl();
   void _updBothHndsSwState();
   void _updCurTimeMs();
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
   * The class models a Limbs Safety Single Shot switch, a switch that gives an  activation signal to a machine or device when some independent switches, activated in a designated pattern, ensures no risk for the operator limbs is completed.
   * The constructor instantiates the DbncdMPBttn subclasses objects that compose the Limbs Safety Single Shot switch, i.e. the left hand TmVdblMPBttn, the right hand TmVdblMPBttn and the foot SnglSrvcVdblMPBttn
   * 
   * @param lftHndInpCfg A swtchInptHwCfg_t structure containing the hardware implemented characteristics for the left hand controlled TmVdblMPBttn
   * @param rghtHndInpCfg A swtchInptHwCfg_t structure containing the hardware implemented characteristics for the right hand controlled TmVdblMPBttn
   * @param ftInpCfg A swtchInptHwCfg_t structure containing the hardware implemented characteristics for the foot controlled SnglSrvcVdblMPBttn
   */
  LimbsSftySnglShtSw(swtchInptHwCfg_t lftHndInpCfg,
                    swtchInptHwCfg_t rghtHndInpCfg,
                    swtchInptHwCfg_t ftInpCfg
                    );
   /**
    * @brief Default virtual destructor
    * 
    */
   ~LimbsSftySnglShtSw();
   /**
	 * @brief Attaches the instantiated object to a timer that monitors the input pins and updates the object status.
    * 
	 * The frequency of the periodic monitoring is passed as a parameter in milliseconds, and is a value that must be small (frequent) enough to keep the object updated, but not so frequent that wastes resources needed by other tasks. As the DbncdMPBttn objects components of the switch have a minimum default value, the same is provided for this method, as it makes no sense to check for changes in objects that refresh themselves slower than the checking lenght period.
    * 
    * @param pollDelayMs (Optional) unsigned long integer (ulong), the time between status updates in milliseconds.
    * @return The success in starting the updating timer with the provided update time
    * @retval true Timer starting operation succes
    * @return false Timer starting operation failure
    */
   bool begin(unsigned long int pollDelayMs = _minPollDelay);
   /**
	 * @brief Clears and resets flags, timers and counters modified through the object's signals processing.
	 *
	 * Resets object's attributes to its initialization values to safely continue operations after completing a FDA cycle that might generate unexpected distorsions. This avoids risky behavior of the object due to dangling flags or partially consumed time counters.
    * 
    */
   void clrStatus();
   /**
    * @brief Get the value of the bothHndsSwOk attribute flag
    * 
    * The method evaluates the _bthHndsSwArOn based on the isOn attribute flag state of each hands' value.
    * 
    * @return true Both hands' isOn AF values are true, value saved to the _bthHndsSwArOn AF
    * @return false At least one of hands' isOn AF values are true, value saved to the _bthHndsSwArOn AF
    */
   bool getBothHndsSwOk();
   /**
    * @brief Get the ltchRlsIsOn object's attribute flag value
    * 
    * The ltchRlsIsOn attribute flag indicates if the object is in the latch release state.
    * 
    * @retval true The object is in the latch release state
    * @retval false The object is not in the latch release state
    */
   const bool getLtchRlsIsOn() const;
   /**
    * @brief Get the prdCyclIsOn object'sattribute flag value
    * 
    * The prdCyclIsOn attribute flag indicates if the object is in the producion cycle state.
    * 
    * @retval true The object is in the producion cycle state
    * @retval false The object is not in the producion cycle state
    */
   const bool getPrdCyclIsOn() const;
   /**
	 * @brief Returns the value of the **outputsChange** attribute flag.
	 *
	 * The instantiated objects include attributes linked to their evaluated states, -Attribute Flags- some of them for internal use, some of them for **output related purposes**.
	 * When any of those latter attributes values change, the **outputsChange** flag is set. The flag only signals changes have happened -not which flags, nor how many times changes have taken place- since the last **outputsChange** flag reset, although an internal counter is kept to grant no multithread race conditions affect the correct execution of the outputs updates.
	 * The **outputsChange** flag must be reset (or set if desired) through the setOutputsChange() method.
	 *
    * @retval true: Any of the object's output related behavior flags have changed value since last time **outputsChange** flag was reseted.
    * @retval false: no object's output related behavior flags have changed value since last time **outputsChange** flag was reseted.
	 */
   const bool getOutputsChange() const;
   /**
    * @brief Configures the SnglSrvcVdblMPBttn class object used as **Foot Switch**
    * 
    * Some behavior attributes of the DbncdMPBttn subclasses objects components can be configured to adjust the behavior of the LimbsSftySnglShtSw. In the case of the SnglSrvcVdblMPBttn used as **Foot Switch** the only attribute available for adjustment is the **start delay** value, used to adjust the time the foot switch must be kept pressed after the debounce period, before the switch accepts the input signal. This parameter is used to adjust the "sensibility" of the switch to mistaken, accidental or condition reflex presses.
    * 
    * @param newCfg A limbSftySwCfg_t type structure, from which only the .swtchStrtDlyTm value will be used.
    */
   void cnfgFtSwtch(const limbSftySwCfg_t &newCfg);
   /**
    * @brief Configures the TmVdblMPBttn class object used as **Left Hand Switch**
    * 
    * Some behavior attributes of the DbncdMPBttn subclasses objects components can be configured to adjust the behavior of the LimbsSftySnglShtSw. In the case of the TmVdblMPBttn used as **Left Hand Switch** the attributes available for adjustment are:
    * - **Start Delay** (.swtchStrtDlyTm) value, used to adjust the time the hand switch must be kept pressed after the debounce period, before the switch accepts the input signal. This parameter is used to adjust the "sensibility" of the switch to mistaken, accidental or conditioned reflex presses.
    * - **Is Enabled** (.swtchIsEnbld) value, defines if the hand switch will be enabled, in which case it will be considered for the LimbsSftySnglShtSw state calculation -having to be pressed at the exprected moment, for the expected time and be released when expected to restart the cycle- or disabled, in which case it being pressed or not makes no difference to the LimbsSftySnglShtSw state calculation.
    * - **Switch Voiding Time** (.swtchVdTm) defines the time period the hand switch might be kept pressed before signaling it as voided, having to proceed to release it and press it back to return to the valid pressed (non-voide) state.
    * 
    * @param newCfg A limbSftySwCfg_t type structure, containing the parameters values that will be used to modify the configuration of the TmVdblMPBttn class object. 
    * 
    * @warning The limbSftySwCfg_t type structure has designated default field values, as a consequence any field not expreselly filled with a valid value will be set to be filled with the default value. If not all the fields are to be changed, be sure to fill the non changing fields with the current value to ensure only the intended fields are to be changed!
    */
   bool cnfgLftHndSwtch(const limbSftySwCfg_t &newCfg);
   /**
    * @brief Configures the TmVdblMPBttn class object used as **Right Hand Switch**
    * 
    * Some behavior attributes of the DbncdMPBttn subclasses objects components can be configured to adjust the behavior of the LimbsSftySnglShtSw. In the case of the TmVdblMPBttn used as **Right Hand Switch** the attributes available for adjustment are:
    * - **Start Delay** (.swtchStrtDlyTm) value, used to adjust the time the hand switch must be kept pressed after the debounce period, before the switch accepts the input signal. This parameter is used to adjust the "sensibility" of the switch to mistaken, accidental or conditioned reflex presses.
    * - **Is Enabled** (.swtchIsEnbld) value, defines if the hand switch will be enabled, in which case it will be considered for the LimbsSftySnglShtSw state calculation -having to be pressed at the exprected moment, for the expected time and be released when expected to restart the cycle- or disabled, in which case it being pressed or not makes no difference to the LimbsSftySnglShtSw state calculation.
    * - **Switch Voiding Time** (.swtchVdTm) defines the time period the hand switch might be kept pressed before signaling it as voided, having to proceed to release it and press it back to return to the valid pressed (non-voide) state.
    * 
    * @param newCfg A limbSftySwCfg_t type structure, containing the parameters values that will be used to modify the configuration of the TmVdblMPBttn class object. 
    * 
    * @warning The limbSftySwCfg_t type structure has designated default field values, as a consequence any field not expreselly filled with a valid value will be set to be filled with the default value. If not all the fields are to be changed, be sure to fill the non changing fields with the current value to ensure only the intended fields are to be changed!
    */
   bool cnfgRghtHndSwtch(const limbSftySwCfg_t &newCfg);
   /**
    * @brief Set the Latch Release Total Time (ltchRlsTtlTm) attribute value
    * 
    * The ltchRlsTtlTm attribute holds the time the latching mechanism of the cycle machine will be freed to start the production cycle. Due to the primitive mechanical characteristics of these machines, the mechanical latching mechanism might take different times to be efectively released, so the time must be enough to ensure the correct and full unlatch, but not long enough to keep the machine unlatched when the production cycle is completed, as this might generate the next production cycle to start again, now with no limbs protection provided.
    * 
    * @param newVal Time in milliseconds to keep the unlatch mechanism activated, must be a value greater than 0, and less or equal to the "Production cycle time" (see setPrdCyclTtlTm(const unsigned long int))
    * @return true The parameter value was in the valid range, attribute value updated.
    * @return false The parameter value was not in the valid range, attribute value was not updated.
    */
   bool setLtchRlsTm(const unsigned long int &newVal);
   /**
	 * @brief Sets the value of the attribute flag indicating if a change took place in any of the output attribute flags (IsOn included).
	 *
	 * The usual path for the **outputsChange** flag is to be set by any method changing an output attribute flag, the callback function signaled to take care of the hardware actions because of this changes clears back **outputsChange** after taking care of them. In the unusual case the developer wants to "intercept" this sequence, this method is provided to set (true) or clear (false) outputsChange value.
    *
    * @param newOutputChange The new value to set the **outputsChange** flag to.
    */
	void setOutputsChange(bool newOutputsChange);
   void setPnOtHWUpdtr(LimbsSftySnglShtSwHI* &newVal);
   LimbsSftySnglShtSwHI* getPnOtHWUpdtrPtr();
   /**
    * @brief Set the Production Cycle Total Time (prdCyclTtlTm) attribute value
    * 
    * The prdCyclTtlTm attribute holds the total time for the production cycle, starting from  the moment the latching mechanism of the cycle machine will be freed to start the production cycle and until the hands and foot security switches are kept disabled. After the production cycle timer set time is consumed, the limbs security switch enters the Cycle closure state, ending with the hands and foot security switches re-enabled to their respective configuration states. Setting a short value to the attribute will produce the switch to be re-enabled before the production cycle ends, generating undesired security risks, setting an extremely long value to the attribute will produce the switch to be unavailable to start a new production cycle, slowing the production in an unneeded manner. 
    * 
    * @param newVal Time in milliseconds for the production cycle to be active, must be a value greater than 0, and greater or equal to the "Latch Release Total Time" (see setLtchRlsTm(const unsigned long int))
    * @return true The parameter value was in the valid range, attribute value updated.
    * @return false The parameter value was not in the valid range, attribute value was not updated.
    */
   bool setPrdCyclTtlTm(const unsigned long int &newVal);
   /**
    * @brief Sets the update period lenght for the DbncdMPBttn subclasses objects used as input by the LimbsSftySnglShtSw
    * 
    * Sets the periodic timer used to check the inputs and calculate the state of the object, time period value set in milliseconds.
    * 
    * @param newVal Value to set for the update period, the period must be greater than the minimum debounce default value.
    * @return The success in setting the new value to be used to start the DbncdMPBttn subclasses state updates
    * @retval true The value was in the accepted range and successfuly changed
    * @retval false The value was not in the accepted range and successfuly changed
    * @warning After the begin(unsigned long int) method is executed no other method is implemented to change the periodic update time, so this method must be used -if there's intention of using a non default value- **before** the begin(unsigned long int). Changing the value of the update period after executing the begin method will have no effect on the object's behavior.  
    */
   bool setUndrlSwtchPollDelay(const unsigned long int &newVal);
   // bool end();
   // fncPtrType getFnWhnTrnOff();
	// fncPtrType getFnWhnTrnOn();
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
  LimbsSftySnglShtSwHI(gpioPinOtptHwCfg_t hndsSwtchsOkPin,
                     gpioPinOtptHwCfg_t ltchRlsActvOtpPin,
                     gpioPinOtptHwCfg_t prdCyclActvOtpPin,
                     swtchOtptHwCfg_t lftHndOtptcfg,
                     swtchOtptHwCfg_t rghtHndOtptCfg,
                     swtchOtptHwCfg_t ftOtptCfg
                     );
~LimbsSftySnglShtSwHI();
void updOutputs(MpbOtpts_t lftHndSwtchStts,
               MpbOtpts_t rghtHndSwtchStts,
               MpbOtpts_t ftSwtchStts,
               lsSwtchOtpts_t lsSwtchStts
               );
};
//===================================================>> END Classes declarations


#endif   //_LIMBSSAFETYSW_ESP32_H_
