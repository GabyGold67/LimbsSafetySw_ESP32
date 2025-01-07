# Limbs Safe Activation Switch Library for ESP32 (LimbSafetySw_ESP32)

An ESP32-Arduino library that includes the class, data structures and functions required to model a **Limbs Safety Activation Switch for Cycle Machines and Devices**.

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
 

The switch is modeled for **"launch and forget" cycle machines**, that specific type of machines have the following characteristics:
- The switch starts -"launches"- an individual production cycle.
- Once started there is no control over the cycle execution.