
Interrupt-Driven Reaction Game on DE1-SoC

This project was developed as part of coursework at California State University, Fresno.
It implements a reaction game on the DE1-SoC FPGA board using the Nios II processor and interrupts.

📌 Overview

Board: Altera DE1-SoC (Cyclone V)
Tools: Quartus Prime Lite 18.1, Platform Designer (Qsys), Nios II SBT for Eclipse
Game Goal: Match the LED pattern using push-buttons within a 60-second session

🎮 Features

Interrupt-driven design (timer + PIO ISRs)

Random LED patterns on LEDR[3:0]

Scoring system: +10 for correct, −5 for wrong or timeout (range 0–99)

Session timer: 60 seconds (600 ticks @ 100 ms each)

Display: HEX[1:0] shows score in real time

Controls:

SW1 → Start/Restart game

SW0 → Difficulty (normal/hard, halves pattern interval)

KEY[1–3] → Input pattern

🔎 How It Works

The interval timer ISR generates a tick every 100 ms.

Tracks elapsed time (game ends at 600 ticks = 60 seconds).

Decides when to show the next random LED pattern.

A random 4-bit pattern (1–15) is displayed on LEDR[3:0].

The player presses KEY[1–3] to match the pattern:

If correct → +10 points

If incorrect or too late → −5 points

The score is updated and shown on HEX[1:0].

After 60 seconds, the game ends automatically and can be restarted with SW1.

📂 Files

homework4.qpf → Quartus project

homework4.qsf → Pin assignments

binduhw4.qsys → Platform Designer system

binduhw4.qip, binduhw4.v → Qsys-generated HDL

homework4top.v → Top-level Verilog

main.c → Game logic (Nios II HAL with ISRs)

homework4_time_limited.sof → Bitstream (time-limited)

ECE 278_HW4.pdf → Homework report

⚙️ Build & Run

Open homework4.qpf in Quartus Prime Lite.

Add .qip, .v, and homework4top.v to the project.

Compile and program the FPGA with .sof.

In Nios II SBT, create BSP + app from .sopcinfo.

Add main.c, build, and run on hardware.

Start the game with SW1, score shows on HEX.

🙏 Note

This project demonstrates real-time embedded design concepts: interrupts, hardware/software co-design, and FPGA-based gameplay.

drive link: vedio
https://drive.google.com/file/d/1rSZuq_gVoIeJNFoa4RgvjXaG9vT-2OZk/view
