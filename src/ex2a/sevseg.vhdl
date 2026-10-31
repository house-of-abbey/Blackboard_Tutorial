-------------------------------------------------------------------------------------
--
-- Distributed under MIT Licence
--   See https://github.com/Blackboard_Examples/blob/main/LICENSE
--
-------------------------------------------------------------------------------------
--
-- A basic 4 x 7 segment display decoder.
--
-- References:
--  * Seven Segment Display
--    Overview and use of Blackboard's seven-segment display
--    https://www.realdigital.org/doc/ad0c5fbdaddde0e930abbd9eda84f88b
--
--  * Seven Segment Controller
--    A scanning display controller for the seven-segment display
--    https://www.realdigital.org/doc/586fb4c3326dcd493a5774b2a6050f41
--
-------------------------------------------------------------------------------------

library ieee;
  use ieee.std_logic_1164.all;

entity sevseg is
  generic (
    max_g : positive := 100000 -- 100 MHz / max_g = 1 kHz, digit swap frequency
  );
  port (
    clk         : in  std_logic;
    -- Persuade the block diagram editor to display a positive reset pin.
    -- X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 reset RST"
    -- X_INTERFACE_PARAMETER = "POLARITY ACTIVE_HIGH"
    reset       : in  std_logic;
    enable      : in  std_logic;
    mode        : in  std_logic;                     -- '0' for digits, '1' for individual segments
    digits      : in  std_logic_vector(31 downto 0); -- 4 x (7 segments + decimal point)
    ssd_dp      : out std_logic;
    ssd_cathode : out std_logic_vector(6 downto 0);
    ssd_anode   : out std_logic_vector(3 downto 0)
  );
end entity;


architecture rtl of sevseg is

  signal div_cnt   : natural range 0 to max_g - 1 := 0;
  signal digit_sel : natural range 0 to 3         := 3;
  signal enable_d  : std_logic                    := '0';
  signal digit     : std_logic_vector(7 downto 0) := x"00"; -- D6543210 or D---XXXX (BCD)
  signal display   : std_logic_vector(3 downto 0) := x"0";

  -- Seven-Segment Display
  -- Basic Combinational Circuit Blocks
  -- https://www.realdigital.org/doc/b49d5b28e4b41191f30d43e79430192c
  function sevseg_lut(digit : std_logic_vector) return std_logic_vector is
  begin
    --     A
    --    ---
    -- F |   | B
    --    ---
    -- E | G | C
    --    --- 
    --     D
    case digit is --        "GFEDCBA"
      when x"0"   => return "0111111"; -- 0
      when x"1"   => return "0000110"; -- 1
      when x"2"   => return "1011011"; -- 2
      when x"3"   => return "1001111"; -- 3
      when x"4"   => return "1100110"; -- 4
      when x"5"   => return "1101101"; -- 5
      when x"6"   => return "1111101"; -- 6
      when x"7"   => return "0000111"; -- 7
      when x"8"   => return "1111111"; -- 8
      when x"9"   => return "1101111"; -- 9
      when x"A"   => return "1110111"; -- A
      when x"B"   => return "1111100"; -- B
      when x"C"   => return "0111001"; -- C
      when x"D"   => return "1011110"; -- D
      when x"E"   => return "1111001"; -- E
      when x"F"   => return "1110001"; -- F
      when others => return "0000000";
    end case;

  end function;

begin

  process(clk)
  begin
    if rising_edge(clk) then
      if reset = '1' then
        div_cnt   <= 0;
        digit_sel <= 0;
        display   <= "0001";
        ssd_anode <= "1111"; -- All off
        digit     <= (others => '0');
        enable_d  <= '0';
      else
        if enable = '1' then
          if div_cnt = max_g-1 then
            div_cnt <= 0;
            -- Increments once every 1 ms
            -- There's an optimisation here to directly create the walking '1's pattern
            -- "0001" -> "0010" -> "0100" -> "1000" -> "0001" and so on. But the code for
            -- the digits then needs to use variables and becomes a little more clever
            -- for an initial exercise.
            if (digit_sel = 3) then
              digit_sel <= 0;
            else
              digit_sel <= digit_sel + 1;
            end if;
          else
            div_cnt <= div_cnt + 1;
          end if;
        end if;

        case digit_sel is
          when 0 =>
            display <= "0001";
            digit <= digits( 7 downto  0);
          when 1 =>
            display <= "0010";
            digit <= digits(15 downto  8);
          when 2 =>
            display <= "0100";
            digit <= digits(23 downto 16);
          when 3 =>
            display <= "1000";
            digit <= digits(31 downto 24);
        end case;

        -- Active low
        ssd_anode <= not display;
        enable_d  <= enable;
      end if;

      if enable_d = '1' then
        -- ssd_dp and ssd_cathode are active low
        if mode = '0' then
          ssd_dp      <= not(digit(7));
          ssd_cathode <= not(sevseg_lut(digit(3 downto 0)));
        else
          ssd_dp      <= not(digit(7));
          ssd_cathode <= not(digit(6 downto 0));
        end if;
      else
        ssd_dp      <= '1';
        ssd_cathode <= "1111111";
      end if;
    end if;
  end process;

end architecture;
