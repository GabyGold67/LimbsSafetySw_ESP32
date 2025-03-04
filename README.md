# Limbs Safe Activation Switch Library for ESP32 (LimbSafetySw_ESP32)
## [See the working simulation at WOKWI Site](https://wokwi.com/projects/415018098762515457)  
## [See complete manual at Github Pages](https://gabygold67.github.io/LimbsSafetySw_ESP32/)  
### STM32 versions available by request.

This is an ESP32-Arduino library that includes the class, data structures and functions required to model a **Limbs Safety Activation Switch for Launch and Forget Cycle Machines and Devices (LimbsSftyLnFSwtch)**. The switches implemented through the proper use of the tools provided in this library lets the developer generate **ISO 13849-1 (2023)** compliant secure machine required activation controls (please read the corresponding normative documentation at the [ISO Online Browsing Platform](https://www.iso.org/obp/ui/#iso:std:iso:13849:-1:ed-4:v1:en)).

The provided API gives access to enough tools included to extend this control into the center of a real-time productivity control, data logger, data analysis and IoT node.

The definition above imposes the need to detail the requirements and limitations both to the logic development and to the range of machines it might be connected to.  

## Requirements:
The software must model a device that enforces conditioned activation of several sensors to ensure limbs security when the equipment controlled is activated. This enforced conditions include sequenced activation order and timings as main parameters.
The most basic configuration requires each hand to be positioned in a certain monitored location to ensure their safety, to enable a **release signal generator** device.  The signal generated then will activate the start of the production cycle.
The most simple and usual devices employed are two pushbuttons for the hands position monitor, which enables a foot switch. The enabled foot switch, when pressed, activates the  machine production cycle.  

## Limitations:
The software is specifically designed to be applied in the activation of **"launch and forget" cycle machines**, although it might be applied to other kind of machines, previous security fitness analysis and testing required, not only as a complete solution for the specified security requirement, but also as part of a more complex security solution depending on the machine controlled.   

The **"launch and forget" cycle machines** specific type the software is developed for have the following characteristics:
- The switch starts -_launches_- an individual production cycle.
- Once started there is no control over the cycle execution, including Emergency Exception activations.
- The machine has no signal provision to ensure the successful execution of the cycle start.
- The machine has no signal provision to inform about the production cycle evolution.
- The machine has no signal provision to inform about the production cycle successful end.

The switches modeled by this class ensures the required enforced security practices, while letting some aspects to be configured for specific production tasks on the machine, or to adapt the switch to machines with different security needs.
 
## Projecting beyond the limitations
Once the physical security of the operator is ensured, having a programmable device installed and connected to the actionable mechanisms of the machine opens the door to the multiple benefits the modern controlled machines offer: 
- machine failure detection 
- production control
- time and energy use optimization 
- generated data analysis (local or remotely done by the use of IoT technologies)
... as was stated: **once the programmable device is properly installed and connected**.  

## Definitions:

### Industrial mechanical cycle machines and devices:
Are machines that perform a series of mechanical operations in a repeating sequence: the **production cycle**. The machine's mechanical power source (usually electric motor) is activated in advance and a release mechanism -trigger- starts a sequence of actions for the production that ends with the machine in the same state and position it started from. Many of these machines have no way of stopping it's production cycle before reaching the end/restart point.  

# Limbs Safety Single Shot Switch class (LimbsSftyLnFSwtch)   
The ***LimbsSftyLnFSwtch*** class models a switch for safely activate **"launch and forget" cycle machines** and devices, which originally provides no other mechanism than a latch release mechanical trigger. This means that once activated the machine will complete a production cycle, return to the starting point and wait for a new **Start**|**Release** signal. As such, the minimum security primitive is to ensure no limbs are placed inside any dangerous machine zone before releasing the cycle trigger. The software development will consider then that the physical action needed to release the latch or trigger is replaced with a device such as a electromagnetic pull, an electrovalve commanded pneumatic actuator or similar device with the ability to be temporarily activated by an electric signal. Once the production cycle is started the limbs security will be the same as the one provided by the machine before the electronic upgrade.  

---

## Requirements for the ***LimbsSftyLnFSwtch*** class objects:  
The library must model a switch that ensures the hands positioned in a secure place and a foot to be used to activate the release mechanism, so it must include **three inputs** and **one output** in this most basic configuration:
- The first input is a signal ensuring the left hand is placed away from the dangerous mechanisms of the machine.
- The second input is a signal ensuring the right hand is placed away from the dangerous mechanisms of the machine.
- The third input is a signal intended to be originated in a foot press button, to generate the **Start**|**Release** of the production mechanism if the first two inputs signals comply to the configuration established for the object.
- The output generated includes the value setting of an attribute flag, the option of a function execution and the option of a task unblocking to provide a flexible means of activating the release mechanism.

- Inputs/Outputs requirements
   - The **hands switches** must comply with the following required characteristics:  
      - They must ensure a debounced stable signal for a pressing event to be considered valid.  
      - They must ensure a configurable minimum press time before a pressing event is to be considered valid.
      - They must ensure they are not kept pressed down by artificial meanings -tampering, blocking, mocking- bypassing the security purpose of the switch. For fulfilling that end at least two policies must be enforced:
         - The pressing signal will be voided if kept for longer than a configured time lapse, the un-voiding procedure must include the effective releasing of the switch.
         - The switch must be **disabled** after the device activation and for an established period of time, or until a signal is received. The **enabling** procedure must include the effective releasing of the switch.
      - If some kind of device or production demands a hand or both to be freed from the pressing action (based on the physical operation ensuring a hand or both are necessarily kept outside the dangerous areas of the machine), the input must be set to be ignored, or considering the continuous press of the switch as a validating situation.

   - The **foot switch** must comply with the following required characteristics
      - It must ensure a debounced stable signal for a pressing event to be considered valid.  
      - It must ensure a configurable minimum press time before a pressing event is to be considered valid.
      - Must be kept disabled until both hands switches are generating a valid enabling signal.
      - When enabled the hands switches must still be monitored to ensure both keep a valid state, disabling this switch if the situation changes.
      - When enabled and pressed it triggers the machine **Start**|**Release** signal.
      
### Configuration and parameters for the ***LimbsSafetySw*** library class objects:
Modeling such devices that demand strict compliance to certain rules to ensure the security of the operator and being flexible enough to apply them to a diverse universe of machines demands it to be configurable to adapt to large range of possible scenarios.  
So the configuration of the **LimbsSafetySw** library objects must be understood as separated by areas or **categories**, strongly related to:
   1. **Hardware** implementation of the _LimbsSafetySw_ switch.
   2. **Logic** implementation of the _LimbsSafetySw_, as composed by several **DbncdMPBttn** subclasses' objects.
   3. **Hardware** implementation of the _LimbsSafetySw_-machine interface.
   4. **Logic** parametrization for different production related use cases.

The parameters for each category will have to be provided by different sources and be available for use at different points of execution of the control firmware using the ***LimbsSftyLnFSwtch***. Also considering the security provided by the final instantiated model will depend on the right configuration of it, and as specifically required by the **ISO 13849-1 (2023)**, the access to some parameters configuration must be granted to be protected and limited to administration clearance level operators.

So to make a general description of the parameters required by each category, it's source and execution stage at which they are required:

### 1. Hardware implementation related parameters: 
Must be provided by the hardware implementation documentation, ahead of the solution implementation. If the hardware design gives some grade of liberty, a hardware implementation source of information must be given in the form of dip switches, non-volatile memory registers or other, information set and granted by the hardware design team, including the switch selected and connection to the MCU characteristics. So this category must include the following information configuration parameters:
For the hands switches:
- Required:
   - Input connection pin
   - Type of switch: NO|NC
   - Connection logic: PullUp|PullDown
   - Debounce time
- Optional:
   - isOn attribute flag state
      - Output pin
      - Activation level
   - isDisabled attribute flag state
      - Output pin
      - Activation level
   - isVoided attribute flag state
      - Output pin
      - Activation level
For the foot switch
   - Required:
      - Input connection pin
      - Type of switch: NO|NC
      - Connection logic: PullUp|PullDown
      - Debounce time
   - Optional:
      - Foot switch enabled state
         - Output pin
         - Activation level
- For the latch release mechanism
   - Required:
      - Output pin
      - Activation level
- Other optional hardware related parameters
- Production cycle timer active.
   - Output pin
   - Activation level

### 2. Logic implementation of the class related parameters
For the class to present the behavior described in the requirements the DbncdMPBttn subclasses components' attributes must be configured ideally at LimbsSftySnglShtSw object constructor. If not possible it must be done before the ***LimbsSftyLnFSwtch*** `.begin()` method is invoked by the use of the corresponding "setters" methods. It's important to note that default values are provided in the corresponding constructors to enable the instantiation of the required objects adjusted to standard values that guarantees the security, but are not optimized for productivity performance per se.  
The information must be provided by the security policy administrator, depending on his evaluation of the controlled machine characteristics, the standard operator expected behavior and productivity policies. The parameters might be changed by **Security Administrator** level user.
- For the TmVdblMPBttn hands switches
   - Missing parameters at the hands switches constructor
      - VoidTime
      - StartDelayTime
- For the  SnglSrvcMPBttn foot switch
   - Missing parameters at the foot switch constructor
      - StartDelayTime

###   3. Hardware LimbsSftySnglShtSw - machine interface.
Being the ***LimbsSftyLnFSwtch*** class objects designed for machines with the characteristics already described, the following parameters must be set depending on the controlled machine's individual characteristics. The parameters might be changed by a **Security Administrator** or a **Production Administrator** level user. The "Release Latch time" parameter is related to the time needed by the release mechanism to effectively start the production cycle. A too short time will leave the machine locked, a too long time will risk the possibility of letting the machine start a new production cycle without any security policies enforced, and without any warning. The "Production Cycle time" is expected to be set to the total time a complete production cycle takes to complete. As the machine has no sensors to indicate the cycle end, a time based parameter is included to replace it. The time set must be enough to cover the real time the production cycle takes since the moment the Release latch starts to be activated, and until the cycle is completed. Giving a shortest time parameter risks security, giving a longer time parameter compromises the productivity. So setting a longer time than needed is a safe bet. The time might be adjusted by a setter execution, according to the development implemented mechanisms.
   - Release Latch time (rlsLtchTm)
   - Production Cycle time (prdCyclTm)

### 4. Logic parametrization for production related use cases
   - Left hand switch
      - isEnabled
   - Right hand switch
      - isEnabled
=========================================================================

Users Levels:
   - Developer level users
   - Technical maintenance level users
   - Security supervisor level users
   - Administrative level users
   - Production supervisor level users
   - Operator level users