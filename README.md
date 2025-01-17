# Limbs Safe Activation Switch Library for ESP32 (LimbSafetySw_ESP32)
## [See the working simulation at WOKWI Site](https://wokwi.com/projects/415018098762515457)


An ESP32-Arduino library that includes the class, data structures and functions required to model a **Limbs Safety Activation Switch for Launch and  Forget Cycle Machines and Devices (LimbsSftyLnFSwtch)**.

The definition above imposes requirements and limitations both to the logic development and to the range of machines it might be connected to.  

## Requirements:
The software must model a device that enforces conditioned activation of several sensors to ensure limbs security when the equipment controled is activated. This enforced conditions include sequenced activation order and timings as main parameters.
The most basic configuration requires each hand to be positioned in a certain monitored location to ensure their safety, to enable a **release signal generator** device.  The signal generated then will activate the start of the production cycle.
The most simple and usual devices employed are two pushbuttons for the hands position monitor, which enables a foot switch. The enabled foot switch, when pressed, activates the  machine production cycle.  

## Limitations:
The software is specifically designed to be applied in the activation of **"launch and forget" cycle machines**, altough it might be applied to other kind of machines, previous security fitness analisys and testing.

The **"launch and forget" cycle machines** specific type the software is developed for have the following characteristics:
- The switch starts -"launches"- an individual production cycle.
- Once started there is no control over the cycle execution, including Emergency Exception activations.
- The machine has no signal provision to ensure the successful execution of the cycle start.
- The machine has no signal provision to inform about the production cycle evolution.
- The machine has no signal provision to inform about the production cycle successful end.

The switches modeled by this class ensures the required enforced security practices, while letting some aspects to be configured for specific production tasks on the machine, or to adapt the switch to machines with different security needs.
 
Once the physical security of the operator is ensured, having a programmable device installed and connected to the actionable mechanisms of the machine opens the door to the multiple benefits the modern controlled machines offer: machine failure detection, production control, time and energy use optimization and generated data analisys, local or remotely done by the use of IoT technologies... as was stated: **once the programmable device is properly installed and connected**.  

## Definitions:

### Industrial mechanical cycle machines and devices:
Are machines that perform a series of mechanical operations in a repeating sequence: the **production cycle**. The machine's mechanical power source (usually electric motor) is activated in advance and a release mechanism -trigger- starts a sequence of actions for the production that ends with the machine in the same state and position it started from. Many of these machines have no way of stopping it's production cycle before reaching the end/restart point.


# Limbs Safety Single Shot Switch class (LimbsSftySnglShtSw)   
The ***LimbsSftySnglShtSw*** class models a switch for safely activate **"launch and forget" cycle machines** and devices, which originally provides no other mechanism than a latch release mechanical trigger. This means that once activated the machine will complete a production cycle, return to the starting point and wait for a new **Start**|**Release** signal. As such, the minimum security primitive is to ensure no limbs are placed inside any dangerous machine zone before releasing the cycle trigger. The software development will consider then that the physical action needed to release the latch or trigger is replaced with a device such as a electromagnetic pull, an electrovalve commanded pneumatic actuator or similar device with the hability to be temporarily activated by an electric signal. Once the production cycle is started the limbs security will be the same as the one provided by the machine before the electronic upgrade.

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
         - The pressing signal will be voided if kept for longer than a configured time lapse, the unvoiding procedure must include the efective releasing of the switch.
         - The switch must be **disabled** after the device activation and for an established period of time, or until a signal is received. The **enabling** procedure must include the efective releasing of the switch.
      - If some kind of device or production demands a hand or both to be freed from the pressing action (based on the physical operation ensuring a hand or both are necesarilly kept outside the dangerous areas of the machine), the input must be set to be ignored, or considering the continuous press of the switch as a validating situation.

   - The **foot switch** must comply with the following required characteristics
      - It must ensure a debounced stable signal for a pressing event to be considered valid.  
      - It must ensure a configurable minimum press time before a pressing event is to be considered valid.
      - Must be kept disabled until both hands switches are generating a valid enabling signal.
      - When enabled the hands switches must still be monitored to ensure both keep a valid state, disabling this switch if the situation changes.
      - When enabled and pressed it triggers the machine **Start**|**Release** signal.
      
