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
=========================================================================
