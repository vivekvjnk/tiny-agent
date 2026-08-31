
## Terminology
The following is a brief explanation of some terms used in this document:
1. **Initiator**: Module able to initiate read and write requests to the chip interconnect (typically: processors,
DMA, etc.).
2. **Target**: Unlike an initiator, a target module cannot generate read/write requests to the chip interconnect, but it can respond to these requests. However, it may generate interrupts or a DMA request to the system (typically: peripherals, memory controllers). Note: A module can have several separate ports; therefore, a module can be an initiator and a target.
3. **Agent**: Each connection of one module to one interconnect is done using an agent, which is an adaptation(sometimes configurable) between the module and the interconnect. A target module is connected by a target agent (TA), and an initiator module is connected by an initiator agent (IA). Interconnect: The decoding, routing, and arbitration logic that 
4. **Register Target (RT)**: Special TA used to access the interconnect internal configuration registers.