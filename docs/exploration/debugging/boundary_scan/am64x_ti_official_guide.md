# [FAQ] AM625 / AM623 / AM620-Q1 / AM625-Q1 / AM625SIP / AM62L / AM62Ax / AM62D-Q1 / AM62Px / AM64x / AM243x (ALV, ALX) Custom board hardware design – JTAG - Processors forum - Processors - TI E2E support forums
Hi Board designers, 

**Additional inputs for AM64x**

AM2434: How to do boundary scan testing  
[e2e.ti.com/.../5165506](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1354393/am2434-how-to-do-boundary-scan-testing/5165506?tisearch=e2e-sitesearch&keymatch=Am6442%25252525252525252520serdes0%25252525252525252520%25252525252525252520boundary%25252525252525252520scan#5165506)

![](https://e2e.ti.com/resized-image/__size/640x480/__key/communityserver-discussions-components-files/791/pastedimage1725690047312v1.png)

**For JTAG connector, TRST and TCK should pull up or pull down?**

The AM62x TRSTn signal should have an external pull-down on it and the other JTAG signals should have external pull-ups on them, including the EMU\[1:0\] pins. This prevents the JTAG pins from floating when a debugger is not connected.

[https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1396794/am6442-for-jtag-connector-trst-and-tck-should-pull-up-or-pull-down/5345412](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1396794/am6442-for-jtag-connector-trst-and-tck-should-pull-up-or-pull-down/5345412?tisearch=e2e-sitesearch&keymatch=userdisplayname%25252525253A%252525252522Peaves%252525252522%252525252520%252525252526%252525252526%252525252520push-pull#5345412)

  
The IEEE 1149 specification recommends pull-ups on all of the JTAG signals. However, TI doesn't follow that recommendation for TRST. We recommend TRST to be pulled low to hold the JTAG TAP controller in reset during normal operation to prevent any chance of noise generating conditions that cause the debug or boundary scan functions to do something unexpected. The only side-effect associated with not following the IEEE 1149 recommendation is some debuggers may need to be configured to operate their TRST output in push-pull mode if they default to open-drain mode. The debugger TRST output needs to operate in push-pull mode so it can drive the TRST signal high.  
Thanks for the clarify, what about TCK?  
A pull-up, as recommended by the IEEE 1149 standard.

![](https://e2e.ti.com/resized-image/__size/640x480/__key/communityserver-discussions-components-files/791/pastedimage1761277291195v1.png)

External pulls are recommended for the JTAG signals since these signal traces can get long when a debugger is connected and not actively driving the signals to a valid logic state. A external pulldown on TRSTn is recommended. The JTAG TAP controller is expected to be held in reset (TRSTn low) during normal operation.

Each of the TDO to TDI daisy chain signals have point to point connectivity and they would need to have an external pull-up if the output buffer of the device sourcing the TDO to TDI signal is ever tri-stated.

A 1-input to (n+1)-output buffer is recommended on the TCK signal because each device and the RTCK needs its own CLK signal, so no device is connected to a mid-point of shared fly-by topology CLK signal as shown in the schematic. Using a fly-by CLK signal is likely to cause the devices connect to a mid-point to experience internal clock glitches, which has a good chance of causing the JTAG TAP controller in these devices to do unpredictable things.

The EMU\[1:0\] pins are bi-directional on our processors, where the debugger sources these signal in some use cases. So the buffer shown on EMU0 would be a problem.

The TMS, and EMU\[1:0\] signals can be connected via a fly-by topology as shown as long as they reduce the CLK operating frequency to ensure any reflections from the stubs have time to settle and still provide enough setup time margin.

The other problem they need to consider is the JTAG connector, external pulls, and the IO supplies of all devices connected to the JTAG signals need to be connected to the same power source so there are no sequence issues.

![](https://e2e.ti.com/resized-image/__size/640x480/__key/communityserver-discussions-components-files/791/pastedimage1761277398418v2.png)

![](https://e2e.ti.com/resized-image/__size/640x480/__key/communityserver-discussions-components-files/791/pastedimage1761277435315v3.png)

[(+) AM2432: AM2432 JTAG Chain for Boundary Scan Test - Arm-based microcontrollers forum - Arm-based microcontrollers - TI E2E support forums](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1453198/am2432-am2432-jtag-chain-for-boundary-scan-test/5579229?tisearch=e2e-sitesearch&keymatch=boundary%252520scan%252520chain)

TI doesn't provide boundary scan test software, so you should be discussing these questions with the boundary scan tool/software vendor.

The EMU inputs are typically pulled high with an external resistor. You may need to implement a jumper or switch on your board to pull the appropriate EMU inputs low during boundary scan testing if the boundary scan tool does not have a way to pull these signals low.

I have 3 other devices (two DP83867 PHY chips and a Intel FPGA ) on the JTAG daisy chain with AM2432, I want to use a single JTAG header (TI 20pin cTI) for both XDS110 debugger and boundary scan test. Will this affect XDS110 debugging of the AM2432 ? 

I'm going to assign this thread to the team that supports our CCS debugger software. Hopefully they will know how to configure CCS such that it knows to bypass the other two devices in the scan chain.

There should not be any hardware issue with connecting multiple devices in the same scan chain as long as the IOs associated with the JTAG pins of each device are powered from the same source. The IOs implemented in AM243x devices are not fail-safe, so you must ensure no potential is applied to any of the AM243x IOs until the respective IO power rail is valid. Note: The other two devices may have similar requirements since most CMOS inputs are not fail-safe.

The heavy lifting of boundary scan is done by the third-party boundary scan tool selected by the customer. The tool connects to the boundary scan TAP controller inside the AM62Ax device via JTAG. The tool is used to create and scan test vectors into the AM62Ax device.

The most important thing to consider for schematic capture is providing a way for the EMU\[1:0\] pins to be set to the appropriate logic state before the rising edge of TRSTn. The logic state presented to the EMU\[1:0\] pins will be latched on the rising edge of TRSTn and the latched value determines if the device operates in boundary scan mode or normal mode. The EMU\[1:0\] pins are typically connected to the JTAG connector, which allows the boundary scan tool to set the appropriate value. Your customer may need to confirm their boundary can tool is capable of sourcing the EMU\[1:0\] pins. If not, they may need to have a way to manually change the pull-ups connected to the EMU\[1:0\] pins to pull-downs.

For more information see the Boundary Scan Compliance table in the device TRM.

**AM625-Q1: Unable to access the JTAG DAP on custom AM625 board**

Not connecting the AM62x TRSTn input to the debugger is not a valid connectivity option for the JTAG signals. The TRSTn input has an internal pull-down, which holds the debugger TAP controller in reset during normal operation. We expect the AM62x TRSTn input to be connect to the debugger with an external pull-down. The debugger must force the signal high before communicating with the debugger TAP controller.

The Connectivity Requirements table found in the datasheet clearly tells you how to connect the TRSTn and EMU\[1:0\] inputs.

TRSTn - This input must be connected to VSS through separate external pull resistors to ensure the inputs associated with these balls are held to a valid logic low level if a PCB signal trace is connected and not actively driven by an attached device. The internal pull-down can be used to hold a valid logic low level if no PCB signal trace is connected to the ball.

EMU\[1:0\] - These inputs must be connected to the corresponding power supply through separate external pull resistors to ensure the inputs associated with these balls are held to a valid logic high level if a PCB signal trace is connected and not actively driven by an attached device. The internal pull-up can be used to hold a valid logic high level if no PCB signal trace is connected to the ball.

do not attempt to change the internal pull from a pull-down to a pull-up while an external pull-down is connected to the signal because this will apply a steady-state mid-supply potential to the pin which can damage the input buffer.

The internal pulls may not be strong enough to hold a valid logic state on signals when noise couples to the JTAG signals. This is why we recommend an external pull in addition to the internal pulls when signal traces are connected to the JTAG pins.

I have even seen multiple cases where noise couples to the TRSTn signal and produces unexpected resets while the debugger is connected and actively driving the TRSTn signal high. Most debugger cables are not shielded, which makes it easy for noise to couple into the TRSTn signal. This is why I typically recommend system designers to insert a 100-ohm series resistor and 0.1-uF shunt capacitor low pass filter on the TRSTn signal near the JTAG connector. This will block noise transients that get coupled into the TRSTn signal via the debugger cable.

The combination of the internal pulls and additional 10k external pull are typically good enough to prevent noise from coupling into the PCB trace, assuming your PCB was designed to have 50-ohm impedance-controlled signal traces.

Thanks for the clarification and cautionary note about the internal pull. The TRSTz pin was missed by our HW team due to an assumption that the JTAG connection from the AM26x MCU would apply as-is to the A62x MPU (the MPU clearly has additional requirements as you've point out). We may try to add a blue wire to bring out the TRSTz pin and reflow the MPU to still test and confirm this is the issue.

Though the JTAG is out of commission, we should still be able to boot from another source (i.e. UART), correct? (E.g., TRSTz accessibility does not prevent us from booting in UART mode.) Is there a way to boot from UART (or other source), and then enable the JTAG connection by removing the MPU's debugger TAP controller checks during run-time? It may not be as efficient, but still gives us a post-boot in through the JTAG. Just thinking of potential workarounds while we plan for a board spin...

The AM62X device should boot as expected, assuming the EMU\[1:0\] inputs are pulled high when the MCU\_PORz input transitions from low to high after the device reference clock and all power rails are valid.

I suggest you read the Boot Process, Boot Mode Pins, and Boot Modes sections in the TRM Initialization chapter to understand your boot options.

**AM623: Question about JTAG/Debug Mode (EMU0/1)**

_Are EMU0 and EMU1 required only for advanced debug/trace functionality, or are they mandatory for all supported JTAG debuggers (XDS110, XDS560, Lauterbach)?_

_In other words, can the AM62 be fully debugged using only TCK, TMS, TDI, TDO and TRSTn, with EMU0/EMU1 being optional but recommended?_

_Or are there debug scenarios where EMU0/EMU1 are strictly required?_

_If they are optional, is it enough for our hardware engineers to power EMU0/1 through a resistor or do we also connect theses 2 signals also to JTAG connector (which pins)?_

These pins operate as inputs initially, after power is applied. The AM62x device samples the logic state applied to the EMU\[1:0\] inputs on the rising edge of MCU\_PORz to determine if the device operates in normal mode or enters wait-in-reset mode. Both of these inputs must be high when the MCU\_PORz input rises for the AM62x device to enter normal mode. You have two connectivity choices, which are described in the datasheet Connectivity Requirements table.

Each of these balls must be connected to the corresponding power supply through separate external pull resistors to ensure the inputs associated with these balls are held to a valid logic high level if a PCB signal trace is connected and not actively driven by an attached device. The internal pull-up can be used to hold a valid logic high level if no PCB signal trace is connected to the ball.

I'm fairly sure these pins are optional for some debug functions, where they may be configured to operate as an input and/or an output by the debugger. You would need to contact someone familiar with the specific hardware and software capabilities of your debugger and ask them if these pins are optional for the debugger features you need to use. I can only answer questions related to electrical and timing characteristics associated with the JTAG peripheral.

These pins are also sampled on the rising edge of TRSTn to determine if the JTAG peripheral is connected to the debugger or the boundary scan TAP controllers. Both of these inputs must be high when the TRSTn input rises for the debugger TAP controller to be selected.

It is possible to design a system that does not connect any signal trace to the EMU\[1:0\] pins, but it is not recommended.

Note: The TRM initialization chapter has additional information on how to configure these inputs to enter wait-in-reset or boundary scan mode.  

JTAG Interface

![](https://e2e.ti.com/resized-image/__size/640x480/__key/communityserver-discussions-components-files/791/pastedimage1785836383565v1.png)

Debug Boot Mode

![](https://e2e.ti.com/resized-image/__size/640x480/__key/communityserver-discussions-components-files/791/pastedimage1785836469448v2.png)

Boundary Scan Compliance

![](https://e2e.ti.com/resized-image/__size/640x480/__key/communityserver-discussions-components-files/791/pastedimage1785836503792v3.png)

Is it mandatory to use EMU0/EMU1 Pin to enter Boundary Scan Mode? Is there a possibility without using them?

The answer is no.  The state of the EMU pins must change to enter boundary scan mode. The boundary scan mode has the EMU0:1 and TRSTn status I am not aware of an alternate method to support or enter boundary scan. dependency.

  [AM64x: Boundary Scanning Queries](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1603203/am64x-boundary-scanning-queries) 

The heavy lifting of boundary scan is done by the third-party boundary scan tool selected by the customer. The tool connects to the boundary scan TAP controller inside the AM62Ax device via JTAG. The tool is used to create and scan test vectors into the AM62Ax device.

The most important thing to consider for schematic capture is providing a way for the EMU\[1:0\] pins to be set to the appropriate logic state before the rising edge of TRSTn. The logic state presented to the EMU\[1:0\] pins will be latched on the rising edge of TRSTn and the latched value determines if the device operates in boundary scan mode or normal mode. The EMU\[1:0\] pins are typically connected to the JTAG connector, which allows the boundary scan tool to set the appropriate value. Your customer may need to confirm their boundary can tool is capable of sourcing the EMU\[1:0\] pins. If not, they may need to have a way to manually change the pull-ups connected to the EMU\[1:0\] pins to pull-downs.

For more information see the Boundary Scan Compliance table in the device TRM.

 [RE: AM625: About Am6254 M core JTAG](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1592380/am625-about-am6254-m-core-jtag/6140609) 

You should populate the pull-up resistors for EMU\[1:0\].  Do not count on the internal pulls to hold these signals high.

I do not recommend 100k pull-up resistors because the AM625x pin may have up to 10uA of leakage, which means the resulting signal potential may be marginally higher than VIH and VIHSS. You should consider changing the 100k ohm resistors to 10k ohms. However, I'm not expecting this to be causing the issue described above.

Have you checked the JTAG signals with an oscilloscope to see if they are in the proper state before the device is released from reset and toggling as expected with the proper timing relationship to TCK? If not, this should be the next thing you check.

Make sure the EMU\[1:0\] signals are in a valid high logic state when the MCU\_PORx input is driven from low to high after power is applied.  Also confirm the EMU\[1:0\] signals are in a valid high logic state when the TRSTn signal is driven from low to high by the debugger.

Do you see the TMS and TDI signals toggle. Do you see the TDO signal toggle?

If all of the above looks good, have you tried to reduce the JTAG operating frequency to a very slow clock rate and see if you experience the same problem?

[e2e.ti.com/.../6008158](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1560530/am2432-am2432-reset-by-jtag-trstn/6008158)  
[e2e.ti.com/.../am623-am623-10-pin-jtag-adaptor-and-trstn](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1291480/am623-am623-10-pin-jtag-adaptor-and-trstn)  
We also recommend a pull-down on the TRSTn signal to ensure the TAP controller never receives any spurious commands during normal operation. Therefore, the TRSTn signal must be connected to the debugger and must be driven high when communicating with device via JTAG.  
The nRESET signal is an optional system reset that allows the debugger to reset the entire system.  
[e2e.ti.com/.../am2432-jtag-reset-trstn](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1108168/am2432-jtag-reset-trstn)  
The TRSTn is an active low reset signal. This means that as long as the signal is held low, the device TAP controller will be in reset. When the signal is pulled high by an external debugger, the TAP controller can then be used.  
[www.interfacebus.com/Design\_Connector\_JTAG\_Bus.html](http://www.interfacebus.com/Design_Connector_JTAG_Bus.html)  
1) This is correct, since the TRSTn is an active low signal, the signal should be pulled low when a debugger isn't required. Pulling down the TRSTn signal is not an abnormal case, having an external pull down is actually suggested to prevent possible noise from falsely bouncing high and enabling the TAP controller.  
2) This is correct, the external debugger is in charge of pulling the TRSTn signal high to enable an active debug state.  
[e2e.ti.com/.../4360282](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1159478/am2431-what-to-do-with-unused-trstn/4360282)  
[e2e.ti.com/.../6140609](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1592380/am625-about-am6254-m-core-jtag/6140609)  
You should populate the pull-up resistors for EMU\[1:0\]. Do not count on the internal pulls to hold these signals high.

I do not recommend 100k pull-up resistors because the AM625x pin may have up to 10uA of leakage, which means the resulting signal potential may be marginally higher than VIH and VIHSS. You should consider changing the 100k ohm resistors to 10k ohms. However, I'm not expecting this to be causing the issue described above.

Have you checked the JTAG signals with an oscilloscope to see if they are in the proper state before the device is released from reset and toggling as expected with the proper timing relationship to TCK? If not, this should be the next thing you check.

Make sure the EMU\[1:0\] signals are in a valid high logic state when the MCU\_PORx input is driven from low to high after power is applied. Also confirm the EMU\[1:0\] signals are in a valid high logic state when the TRSTn signal is driven from low to high by the debugger.

Do you see the TMS and TDI signals toggle. Do you see the TDO signal toggle?

If all of the above looks good, have you tried to reduce the JTAG operating frequency to a very slow clock rate and see if you experience the same problem?

Is it ok to not have PU for JTAG connection?  
External pulls are recommended for the JTAG signals since these signal traces can get long when a debugger is connected and not actively driving the signals to a valid logic state. A external pulldown on TRSTn is recommended. The JTAG TAP controller is expected to be held in reset (TRSTn low) during normal operation.

Each of the TDO to TDI daisy chain signals have point to point connectivity and they would need to have an external pull-up if the output buffer of the device sourcing the TDO to TDI signal is ever tri-stated.

A 1-input to (n+1)-output buffer is recommended on the TCK signal because each device and the RTCK needs its own CLK signal, so no device is connected to a mid-point of shared fly-by topology CLK signal as shown in the schematic. Using a fly-by CLK signal is likely to cause the devices connect to a mid-point to experience internal clock glitches, which has a good chance of causing the JTAG TAP controller in these devices to do unpredictable things.

The EMU\[1:0\] pins are bi-directional on our processors, where the debugger sources these signal in some use cases. So the buffer shown on EMU0 would be a problem.

The TMS, and EMU\[1:0\] signals can be connected via a fly-by topology as shown as long as they reduce the CLK operating frequency to ensure any reflections from the stubs have time to settle and still provide enough setup time margin.

The other problem they need to consider is the JTAG connector, external pulls, and the IO supplies of all devices connected to the JTAG signals need to be connected to the same power source so there are no sequence issues.

[e2e.ti.com/.../6099924](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1582647/am2431-subject-jtag-connection-fails-with-sc_err_path_broken--233-on-am2431-board/6099924)

Inputs related to use of JTAG buffers:  
You should have removed all of the level-shifter/buffers and connected the JTAG signals directly to the AM243x device along with the external pull-down on TRSTn and external pull-ups on the other JTAG signals. The level-shifter/buffers were being used on the EVM to protect the AM243x device from a non-fail-safe operating condition when the on-board XDS was powered while the AM243x device was powered off. The external Blackhawk debugger already has similar circuits to take care of this concern when the debugger is connected while the AM243x device is turned off. The power pin on the JTAG header is used to power the SOC side of the debuggers level-shifters.

You did not connect the JTAG clock (TCK) to the debugger return clock (RTCK). This could be a problem if the debugger is configured to use RTCK to capture the TDO signal returning to the debugger. The debugger may have a way to be configured to use TCK to capture the TDO signal returning to the debugger rather than RTCK. If so, select the CLK option as the TDO capture clock and see if your implementation works.

[e2e.ti.com/.../5760254](https://e2e.ti.com/support/processors-group/processors---internal/f/processors---internal-forum/1498946/processor-sdk-am65x-jtag-programming/5760254)  
I suggest they provision for the 20-pin connector described in the following document.

[www.ti.com/.../spru655i.pdf](https://www.ti.com/lit/ug/spru655i/spru655i.pdf?ts=1744211092790)

They would not need to connect anything to the EMU\[4:2\] pins since the AM65x device doesn't have these signals. The /RESET pin is optional. However, they could use this as a system reset source. This would allow the debugger to assert system reset, which can be useful if debugging software from a remote location and the system needs to be reset.

The AM65x EMU\[1:0\], TRSTn, TCK, TDI, TMS, and TDO signals, VSS, and the IO power supply associated with these signals should be made available for connection to the debugger.

The optional TRC\_x signals are used for parallel trace.

I forgot to mention, an external pull-down should be used to hold TRSTn low when not driven by the debugger, and external pull-ups should be used to hold the EMU\[1:0\], TCK, TDI, TMS, and TDO signals high when not driven by the debugger.

[e2e.ti.com/.../5434783](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1418543/faq-am625-am623-am620-q1-am625-q1-am625sip-custom-board-hardware-design-jtag-pulldown-pullup/5434783)  
If they are connecting any signal trace to the JTAG device pins, the EMU\[1:0\], TCK, TDI, TDO, and TMS signals should have an external pull-up resistor and the TRSTn signal should have an external pull-down resistor. External pulls are required because the internal pulls may not be able hold the signals in a valid logic state when noise is coupled to the JTAG signals. The noise coupling concern can occur when the JTAG signals are routed to a connector and a debugger is not connected and/or driving the signal to a valid logic state.

[e2e.ti.com/.../am625-jtag-trst-signal-pull-down](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1310299/am625-jtag-trst-signal-pull-down)  
I agree the pull-down recommendation is not consistent with the IEEE 1149 standard.

The 1149 standard was initially developed to support testing of complete PCB assemblies via boundary scan, but this peripheral was later adopted as a way to control code execution when trying to debug system software.

TI deviated from the 1149 standard many years ago because the primary purpose for the JTAG peripheral on TI processors was to support debug functions rather than boundary scan. The recommendation for holding the TRSTn input low during normal operation was to prevent any chance of noise on the other JTAG signals from accidently causing the debug subsystem from altering code execution during normal operation. The recommendation remains the same today.

Most debuggers support an option to operate their TRST output as open-drain or push-pull. The TRST output of the debugger will need to be configured to operate as a push-pull output so the TRST signal with a pull-down can be driven high when communicating with the TAP controller in the AM62x processor.

[e2e.ti.com/.../5173967](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1356270/am623-sc_err_path_broken---am62---xds-110-jtag-error/5173967)  
The AM62x TRSTn signal should have an external pull-down on it and the other JTAG signals should have external pull-ups on them, including the EMU\[1:0\] pins. This prevents the JTAG pins from floating when a debugger is not connected.

Since the AM62x TRSTn signal is expected to have a pull-down on it, the debugger must use a push-pull driver to source this signal so it can force the pull-down high.

The LP-XDS110ET board does not appear to support that capability since pin 15 of U10 is operating as an open-drain output. I do not see any way to reconfigure the LP-XDS110ET board where it is able to operate pin 15 of U10 in push-pull mode.

You need to find a different debugger, that is able to operate its TRSTn output in push-pull mode.

[e2e.ti.com/.../am2431-regarding-jtag-queries](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1349818/am2431-regarding-jtag-queries)

The buffers inserted in the TMS, TDI, TDO, and TRST signal paths are being used as level shifters to ensure the debugger does not applying any potential to the AM243x pins when the debugger is connected, and power is turned off to the AM243x device. The AM243x pins are not fail-safe, so your system implementation must never apply any potential to the AM243x pins when it is not powered. See the "Steady-state max voltage at all other IO pins" parameter in the Absolute Maximum Ratings table found in the AM243x datasheet.

The buffers in the TCK and RTCK paths provide the same voltage level translation function as well as preserving signal quality of the debugger clock source that needs to branch into to two paths, where one path sources the AM243x device and the other path returns a delayed clock to the debugger to improve timing margin at high operating speeds.

Your debugger may not require a RTCK. If so, you may not need to branch the debugger clock source.

Some debuggers have an internal level translator, where the same AM243x IO power source is routed to the JTAG header and powers the downstream side of the level translator in the debugger. If so, you may not need the level translators.

You need to understand these types of details associated with the debugger you plan to attach and design your system to accommodate the requirements of both AM243x and the debugger under all possible use cases. This is the system designer's responsibility.

The internal pulls are turned on by default during and after reset. They will remain on as long as software doesn't turn them off.

External pulls are recommended anytime PCB traces are connected to the JTAG pins. The internal pulls are weak and they are not true linear resistors. They get weaker as the voltage approaches the voltage rail the resistor is pulling toward. Therefore, the internal pulls may not be able to hold signals in a valid logic state when external noise sources couple into the PCB signal traces.

The external pulls should be connected to the same power rail as the internal pulls. For example, the EMU\[1:0\], TCK, TDI, TDO, and TMS pins are pulled to VDDSHV\_MCU and the TRSTn pin is pulled to VSS.

The customer should consider inserting a low pass filter on the TRSTn signal near the AM62Px pin, by inserting a 100 series resistor on TRSTn signal with a 0.1uF shunt capacitor on the processor side of the series resistor. I have seen external noise events couple into the TRSTn signal and creating random resets of the TAP controller while operating a debugger. This low pass filter will block transient noise events that cause unexpected resets to the TAP controller.

Minimum recommendation is pull down resistor on TRSTn, pull-up resistor on TCK, TMS, and TDI. A pull resistor on TDO is optional.

[e2e.ti.com/.../4384930](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1164867/faq-am625-jtag-schematics-to-implement-in-customer-board/4384930)  
They should connect TMS, TDI, TDO, TCK, EMU0, and EMU1 from the connector directly to the AM62x device with separate external pull-ups on each of these signals.

They should connect TRSTn from the connector directly to the AM62x device with an external pull-down on this signal.

They will need to research the debugger expectation for the RTCK connection. Some debuggers may require a RTCK, while others have a configurable option to use it or ignore it. I have seen people simply loop TCK back to RTCK near the connector, but this can cause signal integrity issues. Splitting a signal trace like this can cause reflections that can introduce clock glitches. I suggest placing two resistors near the TCK connector pin and inserting one in series with the TCK signal connecting to the AM62x device and the other in series with the RTCK path when TCK is looped back to RTCK. This acts like series terminations and the resistor values can be adjusted to impedance match the signal transition into two paths which also attenuates any undesired reflections.

The supply connected to the debugger connector should be the same supply that is connected to IO power rail of the AM62x JTAG IOs (VDDSHV\_MCU).

For more information on emulation and trace headers refer to the Emulation and Trace Header TRM: [www.ti.com/.../spru655i.pdf](https://www.ti.com/lit/ug/spru655i/spru655i.pdf)

  
3\. Each JTAG pin, EMU\[1:0\], TCK, TDI, TDO, TMS, and TRSTn, has an internal pull turned on by default. So external pulls are only necessary when signal traces are connected to the pins. External pull resistors in the same direction as the internal pulls are also recommended when these pins are connected to signal traces since the internal pull resistors are high impedance and may not be able hold the pins to valid logic levels when noise sources couple to the signal traces. We recommend connecting these signals to the standard 14-pin JTAG header/connector in case you find a need to use JTAG. You could remove the connector from production systems and install it only when needed.

The TI 20-pin connector may be a better choice since it consumes a smaller footprint on the PCB.

Regarding 3.: Can a minimum JTAG interface avoid the voltage translator and the clock buffer? they are trying to find the minimum component number...  
And what about EMC and ESD filtering? (or do you want me to start a new thread on this one?)

I think most of the debuggers include a level-shifter and use it to scaled the IO voltage to match the target PCB based on the voltage applied to the JTAG header. However, I'm not sure if that is the case for all debuggers. I also do not know if they removed potential from the JTAG pins when our device is not powered. Most of our IOs are not fail-safe so it is important to ensure no potential is applied to them while the device is not powered.

Most debuggers provide a return clock to help account for the round trip PCB and cable delays. This allows the JTAG interface to be run faster. Some of our older DSPs actually took the JTAG clock into our SoC and looped it back for a better representation of the full clock to return data delay. This pin was removed because most customers prefer to have this pin for other signal functions rather than a faster JTAG interface. You occasionally see the JTAG clock branched such that one path is connected to the target SoC and the other to the debugger return clock near the JTAG header. However, branching a clock signal like this can be problematic and can cause glitches on the clock. Including a buffer in the path will resolve the signal integrity issues caused by the branch, but it may insert delay which will reduce the max operating frequency of the JTAG interface. I think most debuggers can be configured to not use the return clock. If so, it may be better if the JTAG clock is not branched or buffered and simply connected directly from the debugger to the target SoC. However, your customer may need to discuss this with their debugger supplier before deciding how to connect the JTAG clock on their PCB.

[(+) AM67A: If not using can JTAG pins be left as unterminated/NC? - Processors forum - Processors - TI E2E support forums](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/1349183/am67a-if-not-using-can-jtag-pins-be-left-as-unterminated-nc?tisearch=e2e-sitesearch&keymatch=TRSTn)

Datasheet (DS) says that if no traces are connected then TRSTn will be pulled low internally and TMS, TDI and TCK will be pulled high. 

DS says the EMU0,1 musts be pulled high externally, even though internal PUs are present during reset and after.  Why is this?.  What value of PU should be used?.  Can these pins be just tied high?

The internal pull resistor are weak, and if connecting to trace/pad with capacitance some capacitive load can cause intermittent signal level.  This over time can cause failure.  The recommended pull-up values are 4.7K-10K ohms. 

Yes - can connected directly to supply if desired.

 [RE: AM5728: TRSTn pin](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/601053/am5728-trstn-pin/2214710) 

 [RE: AM5716: Pullup on TMS and TCLK signals](https://e2e.ti.com/support/processors-group/processors/f/processors-forum/671016/am5716-pullup-on-tms-and-tclk-signals/2468518) 

The AM5716 contains internal pull resistors to keep them at their inert levels.  If track is added, these signals should have external pull resistors added for optimum noise immunity.  Therefore, TCK, TDI and TMS should have external pull-up resistors and TRSTn should have an external pull-down resistor when an emulator connection is implemented.  This is consistent with the general JTAG guidance provided in SPRU655 and related documents.

Regards,

Sreenivasa