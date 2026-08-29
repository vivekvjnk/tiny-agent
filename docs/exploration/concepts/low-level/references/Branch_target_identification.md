Branch Target Identification (BTI) is a hardware-based control flow integrity (CFI) feature introduced in Armv8.5-A to protect processors against Jump-Oriented Programming (JOP) and Call-Oriented Programming (COP) attacks. [1, 2]  
How BTI Works 

• Landing Pads: BTI introduces special  instructions (landing pads) that mark valid destinations for indirect branch instructions like  (branch to register) and  (branch with link to register). 
• Guarded Pages: Memory regions can be marked as guarded using page-table attributes. Inside these regions, the processor tracks branch types using internal state (`PSTATE.BTYPE`). 
• Exception Generation: If an indirect branch targets an instruction that is not a compatible  landing pad within a guarded region, the processor immediately generates a Branch Target Exception, halting the unauthorized control flow. 
• Backward Compatibility: Outside of guarded memory regions, or on older processors, a  instruction safely executes as a  (no-operation). [3, 6]  

Implementation and Usage 

• Compilation: Software developers enable BTI support during compilation by passing the `-mbranch-protection=bti` or `-mbranch-protection=standard`  flag in toolchains like GCC and Clang. 
• Ecosystem Integration: Modern operating systems and distributions (such as OpenBSD and various Linux distributions) incorporate BTI across kernels, C libraries, and userland binaries to enforce system-wide security hardening. [6, 7, 8, 9]  

Would you like to explore how BTI works alongside Pointer Authentication (PAC) to prevent return-oriented programming, or do you need help enabling these flags in a GCC/Clang compilation pipeline? 
AI responses may include mistakes.

[1] https://optee.readthedocs.io/en/latest/architecture/arm_security_extensions.html
[2] https://developer.arm.com/community/arm-community-blogs/b/tools-software-ides-blog/posts/code-reuse-attacks-the-compiler-story
[3] https://developer.arm.com/documentation/111108/2026-03/Base-Instructions/BTI--Branch-target-identification-
[4] https://developer.arm.com/community/arm-community-blogs/b/architectures-and-processors-blog/posts/armv8-1-m-pointer-authentication-and-branch-target-identification-extension
[5] https://newsroom.arm.com/blog/pac-bti
[6] https://fedoraproject.org/wiki/Changes/Aarch64_PointerAuthentication
[7] https://archive.fosdem.org/2025/schedule/event/fosdem-2025-5517-enabling-architectural-features-in-debian-pac-and-bti-on-arm64/
[8] https://rsadowski.de/posts/2024-05-14-branch-target-identification/
[9] https://support.arm.com/documentation/102433/0200/Applying-these-techniques-to-real-code

