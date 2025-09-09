module homework4top (
    input  wire        CLOCK_50,  // 50 MHz system clock input.
    input  wire [3:0]  KEY,       // 4 push buttons (KEY[3:0]).
	input wire  [9:0]  SW,        // 10 slide switches (SW[9:0]).
    output wire [9:0]  LEDR,      // 10 red LEDs (LEDR[9:0]).
    output wire [6:0]  HEX0,      // 7-segment display 0.
    output wire [6:0]  HEX1,      // 7-segment display 1.
    output wire [6:0]  HEX2,      // 7-segment display 2.
    output wire [6:0]  HEX3,      // 7-segment display 3.
    output wire [6:0]  HEX4,      // 7-segment display 4.
    output wire [6:0]  HEX5,      // 7-segment display 5.
	
    // SDRAM interface signals.
    output wire [12:0] DRAM_ADDR,    // SDRAM Address.
    output wire [1:0]  DRAM_BA,      // SDRAM Bank Address.
    output wire        DRAM_CAS_N,   // SDRAM Column Address Strobe.
    output wire        DRAM_CKE,     // SDRAM Clock Enable.
    output wire        DRAM_CS_N,    // SDRAM Chip Select.
    inout  wire [15:0] DRAM_DQ,      // SDRAM Data bus.
    output wire        DRAM_LDQM,    // SDRAM Lower Data Mask.
    output wire        DRAM_RAS_N,   // SDRAM Row Address Strobe.
    output wire        DRAM_UDQM,    // SDRAM Upper Data Mask.
    output wire        DRAM_WE_N,    // SDRAM Write Enable.
    output wire        DRAM_CLK      // SDRAM Clock output.
);

    wire pll_locked; // Wire for PLL lock status.

    // Instantiation of the Qsys-generated system module.
    binduhw4 u0 (
        .pll_ref_clk_clk             (CLOCK_50),                 // Connects 50 MHz clock to PLL.
        .pll_ref_reset_reset         (~KEY[0]),                  // Inverted KEY[0] as active-low system reset.

        // I/O connections to Qsys system.
        .keys_external_connection_export     (KEY),              // Push buttons.
        .leds_external_connection_export     (LEDR),             // Red LEDs.
        .hex0_export                 (HEX0),                     // HEX0 display.
        .hex1_export                 (HEX1),                     // HEX1 display.
        .hex2_export                 (HEX2),                     // HEX2 display.
        .hex3_export                 (HEX3),                     // HEX3 display.
        .hex4_export                 (HEX4),                     // HEX4 display.
        .hex5_export                 (HEX5),                     // HEX5 display.
		.switches_export             (SW),                       // Slide switches.

        // SDRAM connections to Qsys SDRAM controller.
        .sdram_wire_addr             (DRAM_ADDR),
        .sdram_wire_ba               (DRAM_BA),
        .sdram_wire_cas_n            (DRAM_CAS_N),
        .sdram_wire_cke              (DRAM_CKE),
        .sdram_wire_cs_n             (DRAM_CS_N),
        .sdram_wire_dq               (DRAM_DQ),
        .sdram_wire_dqm              ({DRAM_UDQM, DRAM_LDQM}),   // Concatenate DQM signals.
        .sdram_wire_ras_n            (DRAM_RAS_N),
        .sdram_wire_we_n             (DRAM_WE_N),
        .pll_sdram_clk_clk           (DRAM_CLK)                  // PLL-generated SDRAM clock.
        // .sdram_pll_locked_export (pll_locked) // PLL locked signal (commented out).
    );

endmodule
