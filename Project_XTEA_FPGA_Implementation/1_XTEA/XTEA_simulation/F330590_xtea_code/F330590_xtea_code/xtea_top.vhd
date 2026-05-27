library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

-- Entity definition
entity xtea_top is
    PORT (
        clk            : in  std_logic;
        reset_n        : in  std_logic;
        encryption     : in  std_logic;
        key_data_valid : in  std_logic;
        data_word_in   : in  std_logic_vector(31 DOWNTO 0);
        key_word_in    : in  std_logic_vector(31 DOWNTO 0);
        data_word_out  : out std_logic_vector(31 DOWNTO 0);
        data_ready     : out std_logic
    );
end entity xtea_top;

-- Architecture definition
architecture rtl of xtea_top is
    -- Five state FSM implementation
    type fsm_state_xtea is (IDLE, LOAD, COMPUTE, OUTPUT_DATA, OUTPUT_DONE);
    signal state : fsm_state_xtea;
    
    -- Constants that are needed for the encryption and decrytion.
    CONSTANT DELTA : unsigned(31 DOWNTO 0) := x"9E3779B9";
    CONSTANT SUM   : unsigned(31 DOWNTO 0) := x"C6EF3720";
  
    type key_array_t is array (0 to 3) of unsigned(31 DOWNTO 0);
    signal current_key      : key_array_t;
    signal current_y0       : unsigned(31 DOWNTO 0);
    signal current_z0       : unsigned(31 DOWNTO 0);
    signal current_y1       : unsigned(31 DOWNTO 0);
    signal current_z1       : unsigned(31 DOWNTO 0);
    signal current_sum      : unsigned(31 DOWNTO 0);
    signal pipe_y0          : unsigned(31 downto 0);
    signal pipe_y1          : unsigned(31 downto 0);
    signal pipe_sk         : unsigned(31 downto 0);
    signal pipe_sum         : unsigned(31 DOWNTO 0);
    signal compute_phase    : std_logic := '0';
    signal enc_mode         : std_logic;
    signal word_cnt         : integer range 0 to 3;
    signal round_cnt        : integer range 0 to 31;


    function mix(x : unsigned(31 DOWNTO 0)) return unsigned is
        variable shift_left4  : unsigned(31 DOWNTO 0);
        variable shift_right5 : unsigned(31 DOWNTO 0);
    begin
        shift_left4  := x(27 DOWNTO 0) & "0000";       
        shift_right5 := "00000" & x(31 DOWNTO 5);     
        return (shift_left4 XOR shift_right5) + x;
    end function mix;

    function lookup_key(key : key_array_t; idx : unsigned(1 DOWNTO 0))
        return unsigned is
        variable i : integer range 0 to 3;
    begin
        i := 3 - TO_INTEGER(idx);
        return key(i);
    end function lookup_key;

begin

     stim_proc : process(clk, reset_n)
        variable new_y0  : unsigned(31 DOWNTO 0);
        variable new_y1  : unsigned(31 DOWNTO 0);
        variable new_z0  : unsigned(31 DOWNTO 0);
        variable new_z1  : unsigned(31 DOWNTO 0);
        variable new_sum : unsigned(31 DOWNTO 0);
        variable sk0     : unsigned(31 DOWNTO 0);
        variable sk1     : unsigned(31 DOWNTO 0);
        variable adj_sum : unsigned(31 DOWNTO 0);

    begin
        if reset_n = '0' then
            state         <= IDLE;
            data_ready    <= '0';
            enc_mode      <= '0';
            data_word_out <= (others => '0');
            word_cnt      <= 0;
            round_cnt     <= 0;
            current_sum   <= (others => '0');
            compute_phase <= '0';
            pipe_y0       <= (others => '0');
            pipe_y1       <= (others => '0');
            pipe_sk       <= (others => '0');
            pipe_sum      <= (others => '0');
            current_y0    <= (others => '0');
            current_z0    <= (others => '0');
            current_y1    <= (others => '0');
            current_z1    <= (others => '0');
            for j in 0 to 3 loop
                current_key(j) <= (others => '0');
            end loop;
 
        elsif rising_edge(clk) then
            data_ready <= '0';
 
                -- Waits for valid key and data
                case state is
                    when IDLE =>
                        word_cnt <= 0;          
                        if key_data_valid = '1' then
                            enc_mode   <= encryption;
                            current_key(0) <= unsigned(key_word_in);
                            current_y0 <= unsigned(data_word_in);  
                            word_cnt   <= 1;
                            state      <= LOAD;
                        end if;
                        
                    -- Loads remaining key and data words  
                    when LOAD =>
                        case word_cnt is
                            when 1 =>
                                current_key(1) 	<= unsigned(key_word_in);
                                current_z0  <= unsigned(data_word_in);
                            when 2 =>
                                current_key(2) 	<= unsigned(key_word_in);
                                current_y1  <= unsigned(data_word_in);
                            when 3 =>
                                current_key(3)  <= unsigned(key_word_in);
                                current_z1 	<= current_y0;                  
                                current_y1 	<= current_z0;                  
                                current_z0 	<= current_y1;                  
                                current_y0 	<= unsigned(data_word_in);
								
                                if enc_mode = '1' then
                                    current_sum <= (others => '0');    
                                else
                                    current_sum <= SUM;      
                                end if;

                                round_cnt <= 0;
                                state     <= COMPUTE;

                            when others => NULL;
                        end case;

                        if word_cnt < 3 then
                            word_cnt <= word_cnt + 1;
                        end if;
						
                    -- Performs XTEA encryption or decryption rounds
                    when COMPUTE =>
						-- Encryption
                         if compute_phase = '0' then
                            if enc_mode = '1' then
                                adj_sum := current_sum + DELTA;
                                sk0     := current_sum + lookup_key(current_key, current_sum(1 DOWNTO 0));
                                sk1     := adj_sum     + lookup_key(current_key, adj_sum(12 DOWNTO 11));
 
                                new_y0  := current_y0 + (mix(current_z0) XOR sk0);
                                new_y1  := current_y1 + (mix(current_z1) XOR sk0);
 
                                pipe_y0  <= new_y0;   
                                pipe_y1  <= new_y1;
                                pipe_sk  <= sk1;      
                                pipe_sum <= adj_sum;  
                            else
                                adj_sum := current_sum - DELTA;
                                sk1     := current_sum + lookup_key(current_key, current_sum(12 DOWNTO 11));
                                sk0     := adj_sum     + lookup_key(current_key, adj_sum(1 DOWNTO 0));
 
                                new_z0  := current_z0 - (mix(current_y0) XOR sk1);
                                new_z1  := current_z1 - (mix(current_y1) XOR sk1);
 
                                pipe_y0  <= new_z0;   
                                pipe_y1  <= new_z1;
                                pipe_sk  <= sk0;      
                                pipe_sum <= adj_sum;
                            end if;
 
                            compute_phase <= '1';
 
                        else
                            if enc_mode = '1' then
                                new_z0 := current_z0 + (mix(pipe_y0) XOR pipe_sk);
                                new_z1 := current_z1 + (mix(pipe_y1) XOR pipe_sk);
                                current_y0 <= pipe_y0;
                                current_z0 <= new_z0;
                                current_y1 <= pipe_y1;
                                current_z1 <= new_z1;
                            else
                                new_y0 := current_y0 - (mix(pipe_y0) XOR pipe_sk);
                                new_y1 := current_y1 - (mix(pipe_y1) XOR pipe_sk);
                                current_z0 <= pipe_y0;   
                                current_y0 <= new_y0;
                                current_z1 <= pipe_y1;
                                current_y1 <= new_y1;
                            end if;
 
                            current_sum   <= pipe_sum;
                            compute_phase <= '0';

                            if round_cnt = 31 then
                                word_cnt <= 0;
                                state    <= OUTPUT_DATA;
                            else
                                round_cnt <= round_cnt + 1;
                            end if;
                        end if;
                        
					-- Outputs four encrypted or decrypted words
                    when OUTPUT_DATA =>
                        data_ready <= '1';

                        case word_cnt is
                            when 0 => data_word_out <= std_logic_vector(current_z1);
                            when 1 => data_word_out <= std_logic_vector(current_y1);
                            when 2 => data_word_out <= std_logic_vector(current_z0);
                            when 3 => data_word_out <= std_logic_vector(current_y0);
                            when others => NULL;
                        end case;

                        if word_cnt = 3 then
                            state <= OUTPUT_DONE;
                        else
                            word_cnt <= word_cnt + 1;
                        end if;
						
					-- Resets counter and returns to IDLE
                    when OUTPUT_DONE =>
                        word_cnt <= 0;
                        state    <= IDLE;
                end case;
            end if;
    end process stim_proc;
end architecture rtl;