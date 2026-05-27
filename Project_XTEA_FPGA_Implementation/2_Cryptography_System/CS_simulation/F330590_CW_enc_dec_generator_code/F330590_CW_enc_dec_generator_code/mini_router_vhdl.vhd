library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity mini_router is
    port (
        -- In channels --
        clk    : in  std_logic;
        reset  : in  std_logic;
        req1   : in  std_logic;
        data1  : in  std_logic_vector(9 downto 0);
        req2   : in  std_logic;
        data2  : in  std_logic_vector(9 downto 0);
        -- Out channels --
        grant1   : out std_logic;
        grant2   : out std_logic;
        valid    : out std_logic;
        data_out : out std_logic_vector(7 downto 0)
    );
end entity;

architecture arch of mini_router is
    -- Code block 1 -- internal signals
    signal sig_r1  : std_logic;
    signal sig_r2  : std_logic;
    signal sig_d1  : std_logic_vector(9 downto 0);
    signal sig_d2  : std_logic_vector(9 downto 0);
    signal sig_rr  : std_logic;
    signal sig_g1  : std_logic;
    signal sig_g2  : std_logic;
    signal sig_val : std_logic;
    signal sig_dout: std_logic_vector(7 downto 0);
	
begin
    -- Code block 2 -- connect ports to internal signals
    sig_r1 <= req1;
    sig_r2 <= req2;
    sig_d1 <= data1;
    sig_d2 <= data2;
    grant1   <= sig_g1;
    grant2   <= sig_g2;
    valid    <= sig_val;
    data_out <= sig_dout;
	
    mini_router: process(clk, reset)
    begin
        if reset = '1' then
            sig_g1   <= '0';
            sig_g2   <= '0';
            sig_val  <= '0';
            sig_dout <= (others => '0');
            sig_rr   <= '0';
 
        elsif rising_edge(clk) then
            sig_g1  <= '0';
            sig_g2  <= '0';
            sig_val <= '0';
            
                if sig_r1 = '1' and sig_r2 = '0' then
                    -- link 1 is chosen as req1 is '1'
                    sig_dout <= sig_d1(7 downto 0);
                    sig_g1   <= '1';
                    sig_val  <= '1';
                elsif sig_r1 = '0' and sig_r2 = '1' then
                    -- link 2 is chosen as req2 is '1'
                    sig_dout <= sig_d2(7 downto 0);
                    sig_g2   <= '1';
                    sig_val  <= '1';
                -- Both req1 and req2 are '1', therefore compare priority bits [9:8]
                elsif sig_r1 = '1' and sig_r2 = '1' then
                    -- link 1 has a higher priority
                    if sig_d1(9 downto 8) > sig_d2(9 downto 8) then
                        sig_dout <= sig_d1(7 downto 0);
                        sig_g1   <= '1';
                        sig_val  <= '1';

                    -- link 2 has a higher priority
                    elsif sig_d1(9 downto 8) < sig_d2(9 downto 8) then
                        sig_dout <= sig_d2(7 downto 0);
                        sig_g2   <= '1';
                        sig_val  <= '1';
                    else
                        -- Both priority bits are equal, therefore the link is chosen using the round robin algorithm. 
                        if sig_rr = '0' then
                            sig_dout <= sig_d1(7 downto 0);
                            sig_g1   <= '1';
                            sig_val  <= '1';
                            sig_rr   <= '1';
                        else
                            sig_dout <= sig_d2(7 downto 0);
                            sig_g2   <= '1';
                            sig_val  <= '1';
                            sig_rr   <= '0';
                        end if;
                    end if;
                end if;
            end if;
    end process;
    -- Code block
end architecture;