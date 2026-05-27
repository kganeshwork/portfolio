library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity mini_router_tb is
end mini_router_tb;

architecture tb of mini_router_tb is

    component mini_router is
        port (
            clk      : in  std_logic;
            reset    : in  std_logic;
            req1     : in  std_logic;
            data1    : in  std_logic_vector(9 downto 0);
            req2     : in  std_logic;
            data2    : in  std_logic_vector(9 downto 0);
            grant1   : out std_logic;
            grant2   : out std_logic;
            valid    : out std_logic;
            data_out : out std_logic_vector(7 downto 0)
        );
    end component;

    constant CLK_PERIOD : time := 10 ns;

    signal clk      : std_logic := '0';
    signal reset    : std_logic := '1';
    signal req1     : std_logic := '0';
    signal data1    : std_logic_vector(9 downto 0) := (others => '0');
    signal req2     : std_logic := '0';
    signal data2    : std_logic_vector(9 downto 0) := (others => '0');
    signal grant1   : std_logic;
    signal grant2   : std_logic;
    signal valid    : std_logic;
    signal data_out : std_logic_vector(7 downto 0);

begin

    DUT : mini_router port map (clk, reset, req1, data1, req2, data2,
                                grant1, grant2, valid, data_out);

    clk <= not clk after CLK_PERIOD / 2;

    process
    begin

        -- Reset
        reset <= '1';
        wait for CLK_PERIOD * 2;
        reset <= '0';
        wait for CLK_PERIOD;

        req1  <= '0'; req2  <= '0';
        data1 <= "00" & x"AA"; data2 <= "00" & x"BB";
        wait until rising_edge(clk); wait for 1 ns;
        assert (valid = '0' and grant1 = '0' and grant2 = '0')
            report "Error 1: valid should be 0 when no req is high" severity error;

        req1  <= '1'; req2  <= '0';
        data1 <= "10" & x"A5"; data2 <= "00" & x"00";
        wait until rising_edge(clk); wait for 1 ns;
        assert (data_out = x"A5" and grant1 = '1' and grant2 = '0' and valid = '1')
            report "Error 2: req1 only - link 1 should be granted" severity error;

        req1  <= '0'; req2  <= '1';
        data1 <= "00" & x"00"; data2 <= "01" & x"B6";
        wait until rising_edge(clk); wait for 1 ns;
        assert (data_out = x"B6" and grant1 = '0' and grant2 = '1' and valid = '1')
            report "Error 3: req2 only - link 2 should be granted" severity error;

        req1  <= '1'; req2  <= '1';
        data1 <= "11" & x"C1"; data2 <= "01" & x"D1";
        wait until rising_edge(clk); wait for 1 ns;
        assert (data_out = x"C1" and grant1 = '1' and grant2 = '0' and valid = '1')
            report "Error 4: L1 pri=11 > L2 pri=01, link 1 should win" severity error;

        req1  <= '1'; req2  <= '1';
        data1 <= "01" & x"C2"; data2 <= "11" & x"D2";
        wait until rising_edge(clk); wait for 1 ns;
        assert (data_out = x"D2" and grant1 = '0' and grant2 = '1' and valid = '1')
            report "Error 5: L2 pri=11 > L1 pri=01, link 2 should win" severity error;
        wait for CLK_PERIOD;

        req1  <= '1'; req2  <= '1';
        data1 <= "01" & x"E1"; data2 <= "01" & x"F1";
        wait until rising_edge(clk); wait for 1 ns;
        assert (data_out = x"E1" and grant1 = '1' and grant2 = '0' and valid = '1')
            report "Error 6: " severity error;

        req1  <= '1'; req2  <= '1';
        data1 <= "01" & x"E2"; data2 <= "01" & x"F2";
        wait until rising_edge(clk); wait for 1 ns;
        assert (data_out = x"F2" and grant1 = '0' and grant2 = '1' and valid = '1')
            report "Error 7: " severity error;

        req1  <= '1'; req2  <= '1';
        data1 <= "01" & x"E3"; data2 <= "01" & x"F3";
        wait until rising_edge(clk); wait for 1 ns;
        assert (data_out = x"E3" and grant1 = '1' and grant2 = '0' and valid = '1')
            report "Error 8: )" severity error;

        report "Simulation complete" severity note;
        wait;

    end process;

end tb;