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

entity test_sevseg is
end entity;


library ieee;
  use ieee.std_logic_1164.all;
  use ieee.numeric_std_unsigned.all;
library std;

architecture test of test_sevseg is
  constant max_c : positive := 4;

  signal clk         : std_logic                     := '0';
  signal reset       : std_logic                     := '0';
  signal enable      : std_logic                     := '0';
  -- '0' for digits, '1' for individual segments
  signal mode        : std_logic                     := '0';
  signal digits      : std_logic_vector(31 downto 0) := (others => '0');
  signal ssd_dp      : std_logic;
  signal ssd_cathode : std_logic_vector(6 downto 0);
  signal ssd_anode   : std_logic_vector(3 downto 0);

  procedure wait_nr_ticks(
    signal   sig : in std_ulogic;
    constant num : in positive
  ) is
  begin
    for n in 1 to num loop
      wait until rising_edge(sig);
    end loop;
  end procedure;

begin

  -- 100 MHz clock
  clk <= not clk after 10 ns;

  dut : entity work.sevseg
    generic map (
      max_g => max_c
    )
    port map (
      clk         => clk,
      reset       => reset,
      enable      => enable,
      mode        => mode,
      digits      => digits,
      ssd_dp      => ssd_dp,
      ssd_cathode => ssd_cathode,
      ssd_anode   => ssd_anode
    );


  process
    variable digit_v : std_logic_vector(3 downto 0);
    variable vec_v   : std_logic_vector(7 downto 0);
  begin
    reset <= '1';
    mode  <= '0';
    wait_nr_ticks(clk, 4);
    reset <= '0';
    wait_nr_ticks(clk, 4);
    enable <= '1';
    
    for i in 0 to 15 loop
      digit_v := to_stdlogicvector(i, 4);
      digits <= "0000" & digit_v &
                "1000" & digit_v &
                "0000" & digit_v &
                "1000" & digit_v;
      wait_nr_ticks(clk, max_c * 10);
    end loop;

    wait_nr_ticks(clk, max_c * 10);
    mode <= '1';
    wait_nr_ticks(clk, max_c * 10);
    vec_v := "00000001";
    for i in 0 to 7 loop
      digits <= vec_v & rotate_left(vec_v, 1) & rotate_left(vec_v, 2) & rotate_left(vec_v, 3);
      wait_nr_ticks(clk, max_c * 10);
      -- Shift vec_v
      vec_v := rotate_left(vec_v, 1);
    end loop;

    -- Last values continue to be shown until disabled.
    wait_nr_ticks(clk, max_c * 10);
    enable <= '0';
    mode   <= '0';
    wait_nr_ticks(clk, 20);

    std.env.stop;
    wait;
  end process;

end architecture;
