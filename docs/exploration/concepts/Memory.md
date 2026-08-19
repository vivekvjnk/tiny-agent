
# DDR PHY Architecture & Calibration Summary

---

## 1. Essential DDR Signals

| Signal | Full Name | Direction | Functional Role |
| --- | --- | --- | --- |
| **`DQ`** | Data Input/Output | Bidirectional | Payload data lines carrying binary bits ($1$s/$0$s). |
| **`DQS` / `DQS#**` | Data Strobe | Bidirectional | Differential strobe clock source-synchronously latching `DQ` data. |
| **`CS#`** | Chip Select | Input (DRAM) | Selects target rank/chip to execute command/address signals. |
| **`DM` / `DBI#**` | Data Mask / Bus Inversion | Input (DRAM) | Masks write byte lanes or toggles data polarity to minimize power/noise. |

---

## 2. DQS Write Leveling (Time-Domain Training)

### Problem

Modern memory uses **Fly-By Topology** for Clock (`CK`) and Command/Address (`CA`) routing to maintain signal integrity across ranks. This introduces propagation delays, causing severe timing skew between `CK` and `DQS` at each DRAM chip.

```
       +-------------------+
       | Memory Controller |
       +--------+----------+
                |
    CK (Fly-By) |==================+==================+
                |                  |                  |
               (Δt0)              (Δt1)              (Δt2)
                v                  v                  v
         +--------------+   +--------------+   +--------------+
         |  DRAM Rank 0 |   |  DRAM Rank 1 |   |  DRAM Rank N |
         +--------------+   +--------------+   +--------------+
                ^                  ^                  ^
    DQS Lines   | (Swept Delay)    | (Swept Delay)    | (Swept Delay)
                +------------------+------------------+

```

### Calibration Algorithm

1. **Enable Mode:** Memory controller sets DRAM into Write Leveling mode via `MR1`.
2. **Phase Sweep:** Controller incrementally sweeps `DQS` output delay.
3. **Sampling Feedback:** DRAM samples `CK` using `DQS` rising edge and responds over `DQ`.
4. **Lock:** Delay value locks at the $0 \rightarrow 1$ transition point to align `DQS` to `CK`.

---

## 3. VREF Training (Voltage-Domain Training)

Optimizes the vertical DC reference voltage threshold ($V_{\text{REF}}$) to maximize the vertical aperture of the **Data Eye**.

```
              Voltage (V)
                   ^
   V_REF_MAX ------| - - - - - - - /---------------\ - - - - - - - 
                   |              /    DATA EYE     \
   V_REF_OPT ------|-------------[    OPTIMAL CENTER ]-------------
                   |              \                 /
   V_REF_MIN ------| - - - - - - - \---------------/ - - - - - - - 
                   |
                   +---------------------------------------------> Time (t)
                                 |<--- t_DIV --->|

```

### 2D Eye Search Procedure

1. **Step Sweep:** Incrementally adjust internal $V_{\text{refDQ}}$ via DRAM Mode Registers.
2. **Window Measurement:** Measure usable timing window width ($t_{\text{DIV}}$) at each voltage level.
3. **Threshold Detection:** Identify upper ($V_{\text{REF\_MAX}}$) and lower ($V_{\text{REF\_MIN}}$) error boundaries.
4. **Optimal Lock:** Calculate and lock midpoint:

$$\text{V}_{\text{REF\_OPT}} = \frac{\text{V}_{\text{REF\_MAX}} + \text{V}_{\text{REF\_MIN}}}{2}$$

---

## 4. Generational Comparison Matrix

| Feature | DDR3 | DDR4 | DDR5 |
| --- | --- | --- | --- |
| **I/O Signaling** | SSTL (Push-Pull) | POD12 (Pseudo Open Drain) | POD11 / LP-POD |
| **VREF Generation** | External Divider | Internal DRAM ($V_{\text{refDQ}}$) | Per-Receiver Internal LDO |
| **Target $V_{\text{REF}}$ Level** | $50\%\,V_{\text{DDQ}}$ (Symmetrical) | $70\text{--}80\%\,V_{\text{DDQ}}$ (Asymmetrical) | Variable / Dynamic Centering |
| **Training Scope** | Fixed Board Level | Per-Channel / Per-Rank | Per-Pin / Per-Lane Precision |
| **CA VREF Training** | No | Yes ($V_{\text{refCA}}$) | Independent Per-Channel $V_{\text{refCA}}$ |




# References

[1] https://docs.nxp.com/bundle/AN14594/page/topics/write_leveling.html
[2] https://www.linkedin.com/pulse/ddr-write-read-leveling-protocol-training-institute-wzktc
[3] https://docs.amd.com/r/en-US/pg313-network-on-chip/Write-Leveling
[4] https://blogs.sw.siemens.com/electronic-systems-design/2017/10/24/ddr-design-write-leveling-for-better-dq-timing/
[5] https://daffy1108.wordpress.com/2010/09/02/understanding-ddr3-write-leveling-and-read-leveling/
[6] https://www.linkedin.com/pulse/ddr-technology-research-006-nianhang-hu-upczc
[7] https://docs.amd.com/r/en-US/ug586_7Series_MIS/Write-Leveling
[8] https://docs.amd.com/r/en-US/pg353-versal-acap-soft-ddr4-mem-ip/Write-Leveling 



[1] https://electronics.stackexchange.com/questions/408458/data-strobe-in-ddr-memory
[2] https://en.wikipedia.org/wiki/Synchronous_dynamic_random-access_memory
[3] https://blogs.sw.siemens.com/electronic-systems-design/2017/09/13/the-back-and-forth-of-the-ddr-data-bus/
[4] https://www.youtube.com/watch?v=ZSlHA1deH-w
[5] https://www.allpcb.com/ar-SA/allelectrohub/ddr-vs-sdram-functional-and-structural-differences
[6] https://patents.google.com/patent/US6885959B2/en
[7] https://www.avnet.com/americas/products/avnet-boards/support/faq/zynq-pins-deep-dive/
[8] https://tp.prosoft.ru/docs/shared/webdav_bizproc_history_get/92723/92723/?ncc=1&force_download=1
[9] https://semiconductor.samsung.com/resources/data-sheet/117223ds_ddr3_2gb_b-die_based_sodimm_rev103.pdf


[1] [https://electronics.stackexchange.com](https://electronics.stackexchange.com/questions/645103/what-is-mean-by-vref-training-in-ddr4)
[2] [https://www.systemverilog.io](https://www.systemverilog.io/design/ddr4-initialization-and-calibration/)
[3] [https://community.cadence.com](https://community.cadence.com/cadence_blogs_8/b/fv/posts/device-trainings-for-high-speed-drams)
[4] [https://www.flywing-tech.com](https://www.flywing-tech.com/blog/ddr4-vs-ddr5-embedded-ai/)
[5] [https://patents.google.com](https://patents.google.com/patent/CN113728385A/en)
[6] [https://docs.amd.com](https://docs.amd.com/r/en-US/pg456-integrated-mc/VREF-Calibration)
[7] [https://cdn.teledynelecroy.com](https://cdn.teledynelecroy.com/files/whitepapers/new_ddr3_debug_techniques.pdf)
[8] [https://www.youtube.com](https://www.youtube.com/watch?v=nCIc1mWOzMM&t=409)
[9] [https://www.linkedin.com](https://www.linkedin.com/pulse/ddr-technology-research-003-nianhang-hu-dzjtc)
[10] [https://community.altera.com](https://community.altera.com/kb/knowledge-base/what-are-the-vref-requirements-for-a-ddr4-interface-using-the-arria-10-or-strati/345093)
[11] [https://electronics.stackexchange.com](https://electronics.stackexchange.com/questions/645103/what-is-mean-by-vref-training-in-ddr4)
[12] [https://docs.amd.com](https://docs.amd.com/r/en-US/pg353-versal-acap-soft-ddr4-mem-ip/Read-and-Write-VREF-Calibration)
[13] [https://icnavigator.com](https://icnavigator.com/technology/voltage-current-references/ddr-vref-mid-reference/)
[14] [https://www.linkedin.com](https://www.linkedin.com/pulse/ddr-technology-research-003-nianhang-hu-dzjtc)
[15] [https://pcbcool.com](https://pcbcool.com/technical-guides/ddr4-design-guide/)
[16] [https://www.micron.com](https://www.micron.com/sales-support/sales/faqs)
