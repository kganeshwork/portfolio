library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

-- Entity definition
entity xtea_uart_top is
    Port (
        clk   : in  std_logic;
        reset : in  std_logic;
        rx    : in  std_logic;
        tx    : out std_logic;
        led   : out std_logic_vector(15 downto 0)
    );
end xtea_uart_top;

-- Architecture definition
architecture rtl of xtea_uart_top is

    component uart is
        generic (
            clk_freq  : integer   := 100_000_000;
            baud_rate : integer   := 9_600;
            os_rate   : integer   := 16;
            d_width   : integer   := 8;
            parity    : integer   := 0;
            parity_eo : std_logic := '0'
        );
        port (
            clk      : in  std_logic;
            reset_n  : in  std_logic;
            tx_ena   : in  std_logic;
            tx_data  : in  std_logic_vector(7 downto 0);
            rx       : in  std_logic;
            rx_busy  : out std_logic;
            rx_error : out std_logic;
            rx_data  : out std_logic_vector(7 downto 0);
            tx_busy  : out std_logic;
            tx       : out std_logic
        );
    end component;

    component xtea_top is
        port (
            clk            : in  std_logic;
            reset_n        : in  std_logic;
            encryption     : in  std_logic;
            key_data_valid : in  std_logic;
            data_word_in   : in  std_logic_vector(31 downto 0);
            key_word_in    : in  std_logic_vector(31 downto 0);
            data_word_out  : out std_logic_vector(31 downto 0);
            data_ready     : out std_logic
        );
    end component;

    signal reset_n : std_logic;

    -- UART
    signal u_rx_data  : std_logic_vector(7 downto 0);
    signal u_rx_busy  : std_logic;
    signal u_rx_busy_r: std_logic;
    signal u_rx_error : std_logic;
    signal u_tx_ena   : std_logic := '0';
    signal u_tx_data  : std_logic_vector(7 downto 0) := (others => '0');
    signal u_tx_busy  : std_logic;

    -- XTEA
    signal x_enc   : std_logic := '0';
    signal x_dv    : std_logic := '0';
    signal x_din   : std_logic_vector(31 downto 0) := (others => '0');
    signal x_kin   : std_logic_vector(31 downto 0) := (others => '0');
    signal x_dout  : std_logic_vector(31 downto 0);
    signal x_ready : std_logic;

    -- Buffers
    type word_arr is array(0 to 3) of std_logic_vector(31 downto 0);
    signal key_buf : word_arr := (others => (others => '0'));
    signal dat_buf : word_arr := (others => (others => '0'));
    signal out_buf : word_arr := (others => (others => '0'));

    -- Eight state FSM implementation
    type state_t is (S_RX_CTRL, S_RX_BYTES, S_FEED_WORD, S_XTEA_WAIT, S_CAPTURE, S_TX_LOAD, S_TX_WAIT_BUSY, S_TX_WAIT_DONE);
    signal state : state_t := S_RX_CTRL;

    signal byte_cnt : integer range 0 to 31 := 0;
    signal word_idx : integer range 0 to 3  := 0;
    signal cap_cnt  : integer range 0 to 3  := 0;
    signal tx_word  : integer range 0 to 3  := 0;
    signal tx_byte  : integer range 0 to 3  := 0;

    signal shift_reg : std_logic_vector(31 downto 0) := (others => '0');
    signal xtea_busy : std_logic := '0';
    
    signal test_leds : std_logic_vector(5 downto 0) := (others => '0');
    signal test_cnt  : integer range 0 to 5 := 0; 

begin

    reset_n <= not reset;

    u_uart : uart
        generic map (
            clk_freq  => 100_000_000,
            baud_rate => 9_600,
            os_rate   => 16,
            d_width   => 8,
            parity    => 0,
            parity_eo => '0'
        )
        port map (
            clk      => clk,
            reset_n  => reset_n,
            tx_ena   => u_tx_ena,
            tx_data  => u_tx_data,
            rx       => rx,
            rx_busy  => u_rx_busy,
            rx_error => u_rx_error,
            rx_data  => u_rx_data,
            tx_busy  => u_tx_busy,
            tx       => tx
        );

    u_xtea : xtea_top
        port map (
            clk            => clk,
            reset_n        => reset_n,
            encryption     => x_enc,
            key_data_valid => x_dv,
            data_word_in   => x_din,
            key_word_in    => x_kin,
            data_word_out  => x_dout,
            data_ready     => x_ready
        );

    main : process(clk, reset_n)
        variable assembled : std_logic_vector(31 downto 0);
    begin
        if reset_n = '0' then
            state       <= S_RX_CTRL;
            byte_cnt    <= 0;
            word_idx    <= 0;
            cap_cnt     <= 0;
            tx_word     <= 0;
            tx_byte     <= 0;
            x_enc       <= '0';
            x_dv        <= '0';
            x_din       <= (others => '0');
            x_kin       <= (others => '0');
            u_tx_ena    <= '0';
            u_tx_data   <= (others => '0');
            u_rx_busy_r <= '0';
            shift_reg   <= (others => '0');
            xtea_busy   <= '0';
            key_buf     <= (others => (others => '0'));
            dat_buf     <= (others => (others => '0'));
            out_buf     <= (others => (others => '0'));
            test_leds   <= (others => '0');
            test_cnt    <= 0;

        elsif rising_edge(clk) then
            u_rx_busy_r <= u_rx_busy;
            x_dv        <= '0';
            u_tx_ena    <= '0';

            case state is
                -- Waits for the first UART byte
                when S_RX_CTRL =>
                    xtea_busy <= '0';
                    if u_rx_busy_r = '1' and u_rx_busy = '0' then
                        x_enc    <= u_rx_data(0);
                        byte_cnt <= 0;
                        state    <= S_RX_BYTES;
                    end if;

                -- Receives 32 bytes over UART
                when S_RX_BYTES =>
                    if u_rx_busy_r = '1' and u_rx_busy = '0' then
                        shift_reg <= shift_reg(23 downto 0) & u_rx_data;
                        if (byte_cnt mod 4) = 3 then
                            assembled := shift_reg(23 downto 0) & u_rx_data;
                            if byte_cnt < 16 then
                                key_buf(byte_cnt / 4) <= assembled;
                            else
                                dat_buf((byte_cnt - 16) / 4) <= assembled;
                            end if;
                        end if;
                        if byte_cnt = 31 then
                            word_idx <= 0;
                            state    <= S_FEED_WORD;
                        else
                            byte_cnt <= byte_cnt + 1;
                        end if;
                    end if;

                -- Feeds data and key to XTEA
                when S_FEED_WORD =>
                    xtea_busy <= '1';
                    x_din     <= dat_buf(word_idx);
                    x_kin     <= key_buf(word_idx);
                    x_dv      <= '1';              
                    if word_idx = 3 then
                        state <= S_XTEA_WAIT;        
                    else
                        word_idx <= word_idx + 1;
                    end if;
                    
                -- Waits for XTEA to complete
                when S_XTEA_WAIT =>
                    if x_ready = '1' then
                        out_buf(0) <= x_dout;
                        cap_cnt    <= 1;
                        state      <= S_CAPTURE;
                    end if;

                -- Collects remaining XTEA output words
                when S_CAPTURE =>
                    out_buf(cap_cnt) <= x_dout;
                    if cap_cnt = 3 then
                        tx_word <= 0;
                        tx_byte <= 0;
                        state   <= S_TX_LOAD;
                    else
                        cap_cnt <= cap_cnt + 1;
                    end if;
                    
                -- Loads next output byte to transmit
                when S_TX_LOAD =>
                    case tx_byte is
                        when 0 => u_tx_data <= out_buf(tx_word)(31 downto 24);
                        when 1 => u_tx_data <= out_buf(tx_word)(23 downto 16);
                        when 2 => u_tx_data <= out_buf(tx_word)(15 downto  8);
                        when 3 => u_tx_data <= out_buf(tx_word)( 7 downto  0);
                        when others => null;
                    end case;
                    u_tx_ena <= '1';
                    state    <= S_TX_WAIT_BUSY;
                    
                -- Confirms UART transmitter has accepted byte
                when S_TX_WAIT_BUSY =>
                    if u_tx_busy = '1' then
                        state <= S_TX_WAIT_DONE;
                    end if;
                    
                -- Waits for UART transmission to finish
                when S_TX_WAIT_DONE =>
                    if u_tx_busy = '0' then
                        if tx_byte = 3 then
                            tx_byte <= 0;
                            if tx_word = 3 then
                                if test_cnt <= 5 then
                                    test_leds(5 - test_cnt) <= '1';
                                    test_cnt <= test_cnt + 1;
                                end if;
                                xtea_busy <= '0';
                                state     <= S_RX_CTRL;
                            else
                                tx_word <= tx_word + 1;
                                state   <= S_TX_LOAD;
                            end if;
                        else
                            tx_byte <= tx_byte + 1;
                            state   <= S_TX_LOAD;
                        end if;
                    end if;
            end case;
        end if;
    end process main;

    led(0) <= u_rx_busy;
    led(1) <= xtea_busy;
    led(2) <= u_tx_busy;
    led(3) <= u_rx_error;
    
    led(9 downto 4) <= (others => '0'); -- LEDs 9 to 4 are kept off
    led(15 downto 10) <= test_leds; -- LEDs 15 to 10 turn on as each test is passed

end architecture rtl;