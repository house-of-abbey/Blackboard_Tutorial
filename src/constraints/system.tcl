###############################################################################
#
# Distributed under MIT Licence
#   See https://github.com/house-of-abbey/Blackboard/blob/main/LICENCE.
#
###############################################################################
#
# Apply constraints to I/O pins in a way that does not create warnings when
# pins are unused. Vivado complains enough as it is!
#
# Derived from Blackboard Rev. D pin assignments and constraints. Hence the
# original copyright is retained.
#
# J D Abbey & P A Abbey, 1 July 2026
#
###############################################################################

###############################################################################
# File: system_revD.xdc
# Author: Tinghui Wang
#
# Copyright (c) 2018-2019, RealDigital.org
#
# Description:
#   Blackboard Rev. D pin assignments and constraints.
#
# History:
#   10/13/18: Initial release
#
# License: BSD 3-Clause
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
###############################################################################

# clk100_in is from the 100 MHz oscillator on Blackboard
# create_clock -period 10.000 -name gclk [get_ports clk100_in]
#set_property -dict {PACKAGE_PIN H16 IOSTANDARD LVCMOS33} [get_ports clk100_in]

# Please never use "set_clock_groups -asynchronous" for anything ever, disable
# timing on individual nets between clock domains instead to avoid secondary
# issues with new nets sneaking across clock domains unnoticed.
#set_clock_groups -name pxl_clk_async -asynchronous -group [get_clocks $fclk0] -group [get_clocks $pxl_clk]

# =============================================================================
# Dictionary data structures in TCL cannot contain comments, so the pins are
# separated into sections so that some comments can be inserted ahead of the
# assignment.
# =============================================================================

# Just use something standard for now to keep the synthesis tool quiet (ns)
set input_delay_min   0.100
set input_delay_max   0.200
set output_delay_min -0.100
set output_delay_max  0.100

# AXI clock
if {[llength [get_pins -quiet {system_i/processing_system7_i/FCLK_CLK0}]] > 0} {
  set fclk0 [get_clocks -of_objects [get_pins {system_i/processing_system7_i/FCLK_CLK0}]]
} else {
  set fclk0 {}
}
# PS Audio clock
if {[llength [get_pins -quiet {system_i/processing_system7_i/FCLK_CLK1}]] > 0} {
  set fclk1 [get_clocks -of_objects [get_pins {system_i/processing_system7_i/FCLK_CLK1}]]
} else {
  set fclk1 {}
}
# This is the HDMI 5x clock
if {[llength [get_pins -quiet {system_i/clk_wiz_i/clk_out2}]] > 0} {
  set hdmi_clk [get_clocks -of_objects [get_pins {system_i/clk_wiz_i/clk_out2}]]
} else {
  set hdmi_clk {}
}

# pinMap is an array in the format:
#
#   required,  required,       required,         required, optional dictionary
#
# {
#   {pin_name} {{pin_location} {pin_io_standard} {clock} {options}?}
#   {pin_name} {{pin_location} {pin_io_standard} {clock} {{min_max} {<min> <max>}
#                                                         {iob}     {false}}}
# }
#
# options is a dictionary with keys: {min_max} and {iob}
#  * {min_max} [list <min> <max>] in ns, the minimum and maximum input or output delays.
#  * {iob} {true|false}, defaults to true if omitted, whether to pack the first or last
#    register into the IOB for optimal I/O timing.

# EMIO-GPIO connected to 3 PMOD connectors (Rev. D)
# GPIO pins 23-0 three PMOD connectors
# The PMODs have been assigned so that software that ran on the Zedboard sees
# the same A-B-C mapping w.r.t. the GPIO number assignments
# Pmod Header JA (XADC)
# Note that JA1P is rewired to left channel of PDM speaker
#{PS_GPIO_tri_io[0]} {F16}
set Header_JA [dict create \
  {PS_GPIO_tri_io[0]} [list {F17} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[1]} [list {G19} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[2]} [list {G20} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[3]} [list {E18} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[4]} [list {E19} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[5]} [list {E17} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[6]} [list {D18} {LVCMOS33} $fclk0] \
]

# Pmod Header JB
set Header_JB [dict create \
  {PS_GPIO_tri_io[7]}  [list {D19} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[8]}  [list {D20} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[9]}  [list {F19} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[10]} [list {F20} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[11]} [list {C20} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[12]} [list {B20} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[13]} [list {B19} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[14]} [list {A20} {LVCMOS33} $fclk0] \
]

# Rev. C adds Pmod Header JC
# Pin 23 has been borrowed below
set Header_JC [dict create \
  {PS_GPIO_tri_io[15]} [list {V15} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[16]} [list {W15} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[17]} [list {V16} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[18]} [list {T16} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[19]} [list {M19} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[20]} [list {G14} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[21]} [list {G17} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[22]} [list {G15} {LVCMOS33} $fclk0] \
]

# 9-axis inertial module: LSM9DS1
# 3D Accelerometer, 3D gyroscope, 3D magnetometer
# Interrupts: GYRO_DRDY_M, GYRO_INT1_AG, GYRO_INT_M
# GYRO_DEN_AG is connected to PS_GPIO pin 23
set inertial [dict create \
  {GYRO_SPC}           [list {H20} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {GYRO_SDI}           [list {J19} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {GYRO_SDO_AG}        [list {J20} {LVCMOS33} $fclk0] \
  {GYRO_SDO_M}         [list {L17} {LVCMOS33} $fclk0] \
  {GYRO_SS_AG}         [list {K17} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {GYRO_SS_M}          [list {K16} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {GYRO_DRDY_M}        [list {L20} {LVCMOS33} $fclk0] \
  {GYRO_INT1_AG}       [list {M20} {LVCMOS33} $fclk0] \
  {GYRO_INT_M}         [list {L19} {LVCMOS33} $fclk0] \
  {PS_GPIO_tri_io[23]} [list {J14} {LVCMOS33} $fclk0] \
]

# Speaker and microphone pins
# JA1P (PmodA 1) rewired to pdm_speaker_r
# Actual delays for primitives in the output path of 'pdm_mic_mclk':
#        Min     Max
# BUFG: 0.026 - 0.101 ns
# OBUF: 1.232 - 3.532 ns
set fclk1_min 1.258
set fclk1_max 3.633
set audio [dict create \
  {pdm_speaker_l} [list {G18} {LVCMOS33} $fclk1] \
  {pdm_speaker_r} [list {F16} {LVCMOS33} $fclk1] \
  {pdm_mic}       [list {L14} {LVCMOS33} $fclk1] \
  {pdm_mic_mclk}  [list {N15} {LVCMOS33} $fclk1 [dict create {min_max} [list [expr 1.000 + $fclk1_min] \
                                                                             [expr 1.200 + $fclk1_max]] \
                                                             {iob}     {false}]] \
]

# ESP32 UART1
set esp_uart [dict create \
  {esp32_uart1_rxd} [list {U14} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {esp32_uart1_txd} [list {U15} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
]

set switches [dict create \
  {sws_12bits_tri_i[0]}  [list {R17} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[1]}  [list {U20} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[2]}  [list {R16} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[3]}  [list {N16} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[4]}  [list {R14} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[5]}  [list {P14} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[6]}  [list {L15} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[7]}  [list {M15} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[8]}  [list {T10} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[9]}  [list {T12} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[10]} [list {T11} {LVCMOS33} $fclk0] \
  {sws_12bits_tri_i[11]} [list {T14} {LVCMOS33} $fclk0] \
]

set buttons [dict create \
  {btns_4bits_tri_i[0]} [list {W14} {LVCMOS33} $fclk0] \
  {btns_4bits_tri_i[1]} [list {W13} {LVCMOS33} $fclk0] \
  {btns_4bits_tri_i[2]} [list {P15} {LVCMOS33} $fclk0] \
  {btns_4bits_tri_i[3]} [list {M14} {LVCMOS33} $fclk0] \
]

set leds [dict create \
  {leds_10bits_tri_o[0]} [list {N20} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {leds_10bits_tri_o[1]} [list {P20} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {leds_10bits_tri_o[2]} [list {R19} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {leds_10bits_tri_o[3]} [list {T20} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {leds_10bits_tri_o[4]} [list {T19} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {leds_10bits_tri_o[5]} [list {U13} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {leds_10bits_tri_o[6]} [list {V20} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {leds_10bits_tri_o[7]} [list {W20} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {leds_10bits_tri_o[8]} [list {W19} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
  {leds_10bits_tri_o[9]} [list {Y19} {LVCMOS33} $fclk0 [dict create {iob} {false}]] \
]

# RGB LEDs
#
# rgb_pwm[5] LD11_R
# rgb_pwm[4] LD11_G
# rgb_pwm[3] LD11_B
# rgb_pwm[2] LD10_R
# rgb_pwm[1] LD10_G
# rgb_pwm[0] LD10_B
set rgb_leds [dict create \
  {rgb_pwm[5]} [list {Y14} {LVCMOS33} $fclk0] \
  {rgb_pwm[4]} [list {Y16} {LVCMOS33} $fclk0] \
  {rgb_pwm[3]} [list {Y17} {LVCMOS33} $fclk0] \
  {rgb_pwm[2]} [list {W18} {LVCMOS33} $fclk0] \
  {rgb_pwm[1]} [list {W16} {LVCMOS33} $fclk0] \
  {rgb_pwm[0]} [list {Y18} {LVCMOS33} $fclk0] \
]

# Seven Segment Display
#
# ssd_anode[0]   SSEG_AN0
# ssd_anode[1]   SSEG_AN1
# ssd_anode[2]   SSEG_AN2
# ssd_anode[3]   SSEG_AN3
# ssd_cathode[0] SSEG_CA
# ssd_cathode[1] SSEG_CB
# ssd_cathode[2] SSEG_CC
# ssd_cathode[3] SSEG_CD
# ssd_cathode[4] SSEG_CE
# ssd_cathode[5] SSEG_CF
# ssd_cathode[6] SSEG_CG
# ssd_dp         SSEG_DP
set seven_seg [dict create \
  {ssd_anode[0]}   [list {K19} {LVCMOS33} $fclk0] \
  {ssd_anode[1]}   [list {H17} {LVCMOS33} $fclk0] \
  {ssd_anode[2]}   [list {M18} {LVCMOS33} $fclk0] \
  {ssd_anode[3]}   [list {L16} {LVCMOS33} $fclk0] \
  {ssd_cathode[0]} [list {K14} {LVCMOS33} $fclk0] \
  {ssd_cathode[1]} [list {H15} {LVCMOS33} $fclk0] \
  {ssd_cathode[2]} [list {J18} {LVCMOS33} $fclk0] \
  {ssd_cathode[3]} [list {J15} {LVCMOS33} $fclk0] \
  {ssd_cathode[4]} [list {M17} {LVCMOS33} $fclk0] \
  {ssd_cathode[5]} [list {J16} {LVCMOS33} $fclk0] \
  {ssd_cathode[6]} [list {H18} {LVCMOS33} $fclk0] \
  {ssd_dp}         [list {K18} {LVCMOS33} $fclk0] \
]

# HDMI (DVI) outputs
# Here min > max output delay.
# * This is required to keep Vivado happy, but according to Google's "AI Mode"
#   this is entirely possible. If only I understood the references it provided.
# * Reduce min (more negative) and there are hold timing violations.
# * Negative output delay specifications are turned into positive delays on
#   the destination clock path.
#
# The Vivado block diagram adds a Clock Wizard, which redefines the clock on
# its input, which causes two Methodology critical warnings. These cannot be
# helped as it is Vivado redefining itself.
#
# The 'hdmi_hpd_tri_i[0]' input is untested for timing so expect it to need
# amending in future.
set hdmi [dict create \
  {hdmi_out_tmds_clk_n}     [list {U19} {TMDS_33}  $hdmi_clk [dict create {min_max} {-1.8 -3.5} \
                                                                          {iob}     {false}]] \
  {hdmi_out_tmds_clk_p}     [list {U18} {TMDS_33}  $hdmi_clk [dict create {min_max} {-1.8 -3.5} \
                                                                          {iob}     {false}]] \
  {hdmi_out_tmds_data_n[0]} [list {V18} {TMDS_33}  $hdmi_clk [dict create {min_max} {-1.8 -3.5} \
                                                                          {iob}     {false}]] \
  {hdmi_out_tmds_data_p[0]} [list {V17} {TMDS_33}  $hdmi_clk [dict create {min_max} {-1.8 -3.5} \
                                                                          {iob}     {false}]] \
  {hdmi_out_tmds_data_n[1]} [list {P18} {TMDS_33}  $hdmi_clk [dict create {min_max} {-1.8 -3.5} \
                                                                          {iob}     {false}]] \
  {hdmi_out_tmds_data_p[1]} [list {N17} {TMDS_33}  $hdmi_clk [dict create {min_max} {-1.8 -3.5} \
                                                                          {iob}     {false}]] \
  {hdmi_out_tmds_data_n[2]} [list {P19} {TMDS_33}  $hdmi_clk [dict create {min_max} {-1.8 -3.5} \
                                                                          {iob}     {false}]] \
  {hdmi_out_tmds_data_p[2]} [list {N18} {TMDS_33}  $hdmi_clk [dict create {min_max} {-1.8 -3.5} \
                                                                          {iob}     {false}]] \
  {hdmi_hpd_tri_i[0]}       [list {P16} {LVCMOS33} $hdmi_clk [dict create {min_max} {-1.8 -3.5}]] \
]

# Display Data Channel (DDC)
# This port is primarily used to read the monitor's EDID (Extended Display
# Identification Data), allowing the hardware to discover supported
# resolutions, refresh rates, and display timings. The DDC interface relies on
# the I2C bus protocol (utilizing SCL and SDA lines) to enable communication
# between the board's AMD/Xilinx Zynq SoC and a connected monitor. 
set hdmi_i2c [dict create \
  {hdmi_ddc_scl_io} [list {T17} {LVCMOS33} $fclk0] \
  {hdmi_ddc_sda_io} [list {R18} {LVCMOS33} $fclk0] \
]

# XADC Vp, Vn
set xadc [dict create \
  {vp_vn_v_p} [list {K9}  {LVCMOS33} $fclk0] \
  {vp_vn_v_n} [list {L10} {LVCMOS33} $fclk0] \
]

# Servos
set servos [dict create \
  {servo_1} [list {U12} {LVCMOS33} $fclk0] \
  {servo_2} [list {V12} {LVCMOS33} $fclk0] \
  {servo_3} [list {T15} {LVCMOS33} $fclk0] \
  {servo_4} [list {V13} {LVCMOS33} $fclk0] \
]


# Only if a pin is used, apply the pin location to the pin name and apply additional
# XDC-based constraints.
#
# pinMap is a dictionary in the format:
#
# {
#   {pin_name} {{pin_location} {pin_io_standard} {clock} {options}?}
#   {pin_name} {{pin_location} {pin_io_standard} {clock} {options}?}
# }
#
proc configure_pins {pinMap} {
  global input_delay_min
  global input_delay_max
  global output_delay_min
  global output_delay_max

  puts "** Start pin configuration **"

  foreach k [dict keys $pinMap] {
    set p [get_ports $k -quiet]
    if {[llength $p] > 0} {
      set direction [get_property DIRECTION $p]
      puts "Configure $k direction $direction"
      # values is the list: {{pin_location} {pin_io_standard} {clock} {options}?}
      set values [dict get $pinMap $k]
      set_property PACKAGE_PIN [lindex $values 0] $p
      set_property IOSTANDARD  [lindex $values 1] $p
      if {([llength $values] > 3) && ([dict exists [lindex $values 3] {iob}])} {
        set_property IOB [dict get [lindex $values 3] {iob}] $p
      } else {
        # Default is true
        set_property IOB true $p
      }
      set pin_clk [lindex $values 2]
      if {$direction == "IN"} {
        if {([llength $values] > 3) && ([dict exists [lindex $values 3] {min_max}])} {
          # Use specifically supplied values
          set_input_delay  -clock $pin_clk -min [lindex [dict get [lindex $values 3] {min_max}] 0] $p
          set_input_delay  -clock $pin_clk -max [lindex [dict get [lindex $values 3] {min_max}] 1] $p
        } else {
          set_input_delay  -clock $pin_clk -min $input_delay_min $p
          set_input_delay  -clock $pin_clk -max $input_delay_max $p
        }
      } elseif {$direction == "OUT"} {
        if {([llength $values] > 3) && ([dict exists [lindex $values 3] {min_max}])} {
          # Use specifically supplied values
          set_output_delay -clock $pin_clk -min [lindex [dict get [lindex $values 3] {min_max}] 0] $p
          set_output_delay -clock $pin_clk -max [lindex [dict get [lindex $values 3] {min_max}] 1] $p
        } else {
          set_output_delay -clock $pin_clk -min $output_delay_min $p
          set_output_delay -clock $pin_clk -max $output_delay_max $p
        }
      } else {
        puts "No configuration code for direction $direction"
      }
    } else {
      puts "No port $k"
    }
  }
  puts "** End pin configuration **"
}

# Concatenate the separate lists
configure_pins [dict merge $Header_JA $Header_JB $Header_JC $inertial $audio $esp_uart \
  $switches $buttons $leds $rgb_leds $seven_seg $hdmi $hdmi_i2c $xadc $servos]


# Bitstream Settings
set_property BITSTREAM.GENERAL.COMPRESS true [current_design]
set_property BITSTREAM.CONFIG.UNUSEDPIN PULLDOWN [current_design]
