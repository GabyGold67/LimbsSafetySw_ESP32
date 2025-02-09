# Limbs Safety Switches Technical Solution Implementation  

### Limbs Safety Switch library
The **Limbs Safety Switch library** contains the definition for one or more classes and support code -structures, tasks, timers, etc.- for modeling limbs safety switches for cycle machines. Each class will model a different switch with it's own activation execution and control capabilities depending of the existence or possibilities of addition of sensors and actuators capable of providing information through the cycle execution.

---

# Limbs Safety for Launch and Forget Switch class (LimbsSftyLnFSwtch)   
The ***LimbsSftyLnFSwtch*** class models a switch for safely activate **"launch and forget" cycle machines** and devices, which originally provides no other mechanism than a latch release mechanical trigger. This means that once activated the machine will complete a production cycle, return to the starting point and wait for a new **Start**|**Release** signal. As such, the minimum security primitive is to ensure no limbs are placed inside any dangerous machine zone before releasing the cycle trigger. The software development will consider then that the physical action needed to release the latch or trigger is replaced with a device such as a electromagnetic pull, an electrovalve commanded pneumatic actuator or similar device with the ability to be temporarily activated by an electric signal. Once the production cycle is started the limbs security will be the same as the one provided by the machine before the electronic upgrade.

---

## Requirements for the ***LimbsSftySnglShtSw*** class objects:  
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
         - The pressing signal will be voided if kept for longer than a configured time lapse, the unvoiding procedure must include the effective releasing of the switch.
         - The switch must be **disabled** after the device activation and for an established period of time, or until a signal is received. The **enabling** procedure must include the effective releasing of the switch.
      - If some kind of device or production demands a hand or both to be freed from the pressing action (based on the physical operation ensuring a hand or both are necessarily kept outside the dangerous areas of the machine), the input must be set to be ignored, or considering the continuous press of the switch as a validating situation.

   - The **foot switch** must comply with the following required characteristics
      - It must ensure a debounced stable signal for a pressing event to be considered valid.  
      - It must ensure a configurable minimum press time before a pressing event is to be considered valid.
      - Must be kept disabled until both hands switches are generating a valid enabling signal.
      - When enabled the hands switches must still be monitored to ensure both keep a valid state, disabling this switch if the situation changes.
      - When enabled and pressed it triggers the machine **Start**|**Release** signal.
      

#### Inputs/Outputs requirements conclusion:
According to the aforementioned requirements the design of the ***LimbsSftySnglShtSw*** should include the **ButtonToSwitch library** defined classes as inputs.
   - A **TmVdblMPBttn** class object for the left hand input.
   - A **TmVdblMPBttn** class object for the right hand input.
   - A **SnglSrvcMPBttn** class object for the foot input.  

---  

## Implementation:
### Finite States Machine modeling
As the class is modeled for applications where no other signals -inputs or outputs- are expected once the **"launch and forget" cycle ** starts due to lack of possibilities to install sensors or actuators, the execution logic must be the following:

1. Ensure the placement of the hands, enabling the foot activation sensor.  
2. Ensure the timely activation of the foot sensor.
3. Start the production cycle:
   - Disable the three input signal generators
   - Note: ***Point of no return for any internal or external Emergency signal***
   - Activate the release mechanism
   - Hold the release mechanism activated for a established period of time.
4. Deactivate the release mechanism.
5. Wait for a established period of time to consider the production cycle completed.
6. Consider the production cycle completed. 
   - ***Point of reactivation for any internal or external Emergency signal***
   - Activate "End of cycle" flag, function and/or task (useful for ready alerts, starting wasted waiting timers, cycle counters, etc.)
   - Reset the release and cycle timers
   - Enable the configured hands switches. Restart all the activation parameters for a new production cycle.

---  

### Configuration and parameters for the ***LimbsSftySnglShtSw*** class objects:
- The configuration of the **LimbsSftySnglShtSw** must be understood as separated by areas or **categories**, strongly related to:
   1. Hardware **LimbsSftySnglShtSw** implementation.
   2. Logic implementation of the **LimbsSftySnglShtSw** class, as composed by several **DbncdMPBttn** subclasses objects.
   3. Hardware **LimbsSftySnglShtSw** - machine interface.
   4. Logic parametrization for different production related use cases (keeping 1., 2. and 3. settings fixed).

The parameters for each category must be provided by different sources and be available for use at different points of execution of the control firmware using the **LimbsSftySnglShtSw**:
   ### 1. Hardware implementation related parameters: 
   Must be provided by the hardware implementation documentation, including the switch selected characteristics and connection to the MCU. So this category must include the following configuration parameters:
   - Left hand switch
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
   - Right hand switch
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
   - Foot switch
      - Required:
         - Input connection pin
         - Type of switch: NO|NC
         - Connection logic: PullUp|PullDown
         - Debounce time
   - Other hardware related parameters
      - Optional:
         - Foot switch enabled state (Both hands isOn == true)
            - Output pin
            - Activation level
         - Release latch mechanism timer active.
            - Output pin
            - Activation level
            - Activation time
         - Production cycle timer active.
            - Output pin
            - Activation level
            - Activation time
         - End of cycle signal
            - Output pin
            - Activation level
            - Activation time
      
   ### 2. Logic implementation of the class related parameters
   For the class to present the behavior described in the requirements the DbncdMPBttn subclasses components' attributes must be configured ideally at LimbsSftySnglShtSw object constructor. If not possible it must be done before the LimbsSftySnglShtSw `.begin()` method is executed.  
   - For the TmVdblMPBttn hands switches
      - All the parameters at the hands switches constructor
         - VoidTime
         - StartDelayTime
   - For the  SnglSrvcMPBttn foot switch
      - All the parameters at the foot switch constructor
         - StartDelayTime

   ###   3. Hardware LimbsSftySnglShtSw - machine interface.
      - Production Cycle time (prdCyclTm)
      - Release Latch time (rlsLtchTm)


### 4. Logic parametrization for production related use cases
   - Left hand switch
      - isEnabled
   - Right hand switch
      - isEnabled
