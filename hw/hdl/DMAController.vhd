LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
use work.register_file_pkg.all;

ENTITY DMAController IS

	PORT (

		-- Global signals
		clk : IN STD_LOGIC;
		nReset : IN STD_LOGIC;

		-- Signals From Register File

		Flags : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		ImageAddress : IN STD_LOGIC_VECTOR(31 downto 0);
		ImageLength : IN STD_LOGIC_VECTOR(31 downto 0);
		reset_flag_lcdenable : OUT STD_LOGIC;
		-- Possibly reset?

		-- Avalon Master
		address : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
		read : OUT STD_LOGIC;
		readdata : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		readdatavalid : IN STD_LOGIC;
		waitRequest : IN STD_LOGIC;
		-- Size of burstcount???
		burstcount : OUT STD_LOGIC_VECTOR(4 DOWNTO 0);


		-- Output signals to FIFO
		data : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		wrreq : OUT STD_LOGIC;
		almost_full : IN STD_LOGIC

		-- DEBUG
		--cnt_addr : OUT unsigned(31 downto 0);
		--cnt_len : OUT unsigned(31 downto 0);
        --dma_state_out : OUT DMAState
	);

END DMAController;

ARCHITECTURE dma_control_arch OF DMAController IS
	--signals
	signal CurrentAddr : unsigned (31 downto 0);
	signal CurrentLen : unsigned (31 downto 0);
	signal current_burst : unsigned (15 downto 0);
	constant N : std_logic_vector(4 downto 0) := "10000";
	signal state : DMAState;
BEGIN

	data <= readdata;
	wrreq <= readdatavalid;
	
	-- Avalon Master Read.
	PROCESS (clk, nReset, almost_full, waitRequest, readdatavalid, readdata, Flags, ImageAddress, ImageLength)
	BEGIN
		IF nReset = '0' THEN
			CurrentAddr <= (others => '0');
			CurrentLen <= (others => '0');
			state <= Idle;
			current_burst <= x"0001";
			reset_flag_lcdenable <= '1';
		ELSIF rising_edge(clk) THEN
				case state is
					when Idle =>
						reset_flag_lcdenable <= '1';
						CurrentAddr <= unsigned(ImageAddress);
						CurrentLen <= unsigned(ImageLength);
						-- Flags(0) is LCD_enable
						if Flags(0) = '1' and ImageLength /= x"00000000" then
							state <= WaitFifo;
						end if;
					when WaitFifo =>
						if almost_full = '0' then
							state <= Request;
							read <= '1';
							burstcount <= std_logic_vector(N);
							address <= std_logic_vector(CurrentAddr);
						end if;
					when Request =>
						current_burst <= x"0001";
						if waitRequest = '0' then
							state <= WaitData;
							read <= '0';
						end if;
					when WaitData =>
						if readdatavalid = '1' then
							CurrentAddr <= CurrentAddr + 2;
							CurrentLen <= CurrentLen - 2;
							current_burst <= current_burst + 1;
							-- Careful underflow
						end if;
						if current_burst = unsigned(N) then
							state <= CheckData;
						end if;
					when CheckData =>
						if CurrentLen = 0 then
							--image finished
							state <= ResetFlag;
							reset_flag_lcdenable <= '0';
						else 
							state <= WaitFifo;
						end if;
					when ResetFlag =>
						state <= Idle;
				end case;
			
		END IF;
	END PROCESS;

	--dma_state_out <= state;
	--cnt_addr <= CurrentAddr;
	--cnt_len <= CurrentLen;

END dma_control_arch;

					--when WaitData =>
						--if readdatavalid = '1' then
							--CurrentAddr <= CurrentAddr + 2;
							--CurrentLen <= CurrentLen - 2;
							--current_burst <= current_burst + 1;
							---- Careful underflow
							--state <= RecvData;
						--end if;
					--when RecvData =>
						--if CurrentLen = 0 then
							----image finished
							--state <= Idle;
						--elsif current_burst = unsigned(N) then
							----burst finished
							--state <= WaitFifo;
						--else 
							--if readdatavalid = '1' then
								--CurrentAddr <= CurrentAddr + 2;
								--if CurrentLen = 2 then
									--reset_flag_lcdenable <= '0';
								--end if;
								--CurrentLen <= CurrentLen - 2;
								--current_burst <= current_burst + 1;
							--else
								--state <= WaitData;
							--end if;
						--end if;