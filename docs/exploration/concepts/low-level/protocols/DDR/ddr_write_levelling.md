**Stub Reflection**
A stub reflection occurs when a high-speed signal propagates along a main transmission line, hits an offshoot branch (a "stub" connecting to an IC pin or slot), and encounters an impedance mismatch at the un-terminated end. Part of the signal's energy bounces backward toward the main line.

This reflected wave interferes with incoming signal transitions, causing ringing, signal overshoot/undershoot, and ISI (Inter-Symbol Interference). In legacy star topologies, long stubs to each DRAM chip caused severe reflections, limiting max clock speeds.

---

**Fly-By Routing Skew**
Fly-by routing replaces long individual stubs with a single daisy-chain trace for Command, Address, and Control/Clock (CK) lines. While this minimizes stub lengths and reflections, it introduces a physical distance delta to each chip.

Because electrical signals travel at a finite propagation velocity through PCB traces (roughly $150\text{ ps/inch}$ in FR4), the central clock signal arrives at `DRAM 0` first, `DRAM 1` second, and `DRAM 2` last.

```text
CK Arrival Time:  t0 (DRAM 0) -------> t0 + Δt1 (DRAM 1) -------> t0 + Δt2 (DRAM 2)
DQS Arrival Time: Constant point-to-point delay from controller to each DRAM

```

This intentional time difference between the arrival of the shared central clock (CK) and the individual point-to-point data strobes (DQS) at different DRAM chips across the module is **fly-by routing skew**.

---

**Write Leveling**
Write Leveling is an automated calibration training procedure performed by the memory controller during system boot to eliminate fly-by routing skew.

Because CK is delayed progressively down the fly-by chain, the controller must delay its DQS output for each specific DRAM byte lane so that write data arrives in phase with the CK signal at that specific chip package.

```text
                          Write Leveling Calibration Loop
                          
+-------------------+           Drive DQS Edge          +-------------------+
|                   |---------------------------------->|                   |
|                   |                                   |     DRAM Chip     |
| Memory Controller |      Samples CK using DQS edge    | (Feedback Mode)   |
|                   |<----------------------------------|                   |
+-------------------+      Send 0 or 1 via DQ line      +-------------------+
          |
   Increment DQS Phase Delay 
   until 0-to-1 transition detected

```

* **Step 1:** The controller puts the DRAM into Write Leveling mode.
* **Step 2:** The controller sends a DQS pulse to a target DRAM chip.
* **Step 3:** The DRAM uses that DQS edge as a sampling clock to capture the current state of its local CK clock signal.
* **Step 4:** The DRAM sends the sampled value (`0` if CK is LOW, `1` if CK is HIGH) back to the controller over the DQ bus.
* **Step 5:** The controller incrementally delays the DQS signal's phase shift until it sees the DQ feedback change from `0` to `1` (indicating the exact rising edge of CK). It then locks in that unique phase delay for that specific byte lane.