# Transceiver "Wolf"
DDC-DUC SDR Tranceiver project https://ua3reo.ru/tag/transiver-ua3reo/

* Community telegram channel: https://t.me/TRX_Wolf
* На русском https://github.com/XGudron/UA3REO-DDC-Transceiver/blob/master/README.ru-RU.md

## Principle of operation

The RF signal is digitized by a high-speed ADC chip and fed to an FPGA processor. <br>
It performs DDC / DUC conversion (digital frequency shift down or up the spectrum) - by analogy with a direct conversion receiver. <br>
The I and Q quadrature signals from the conversions are fed to the STM32 microprocessor. <br>
It filters, (de) modulates and outputs audio to an audio codec / USB. It also handles the entire user interface. <br>
When transmitting, the process occurs in the opposite order, only at the end of the chain there is a DAC, which converts the digital signal back to analog RF. <br>

## Specifications

* Receiving frequencies: 0 MHz - 750 MHz with fading each 61.44 MHz (above 145 MHz - without input filters)
* Transmission frequencies: 0 MHz - 150 MHz with fading on 110 MHz
* TX power (QRP version): 7W+ (HF), 5W (VHF)
* TX power (QRP++ DB5AT version): 20W (HF), 7W (VHF)
* TX power (RU4PN/WF-100D version): 100W (HF), 50W+ (VHF)
* Two antenna inputs
* Modulation types (TX / RX): CW, LSB, USB, AM, FM, WFM, DIGI
* LNA, Preamplifier
* Adjustable attenuator 0-31dB
* Band pass filters
* ADC dynamic range (16 bit) ~100dB
* Supply voltage: 13.8V (overvoltage and polarity reversal protection)
* Consumption current when receiving: ~0.7А (3'2 QRP), 0.9A (7' BIG)
* Current consumption during transmission: ~2.5А+ (QRP), 15A+ (BIG)
* Support for different displays: RA8875/HX8357B/HX8357C/ST7789/ST7796S/ST7735S/ILI9341/ILI9481/ILI9486

## Transceiver Features

* Panorama (spectrum + waterfall) up to 384 kHz wide
* Panorama tweaks and themes
* Dual receiver (mixing A + B or A&B audio in stereo)
* Adjustable bandwidth: HPF from 0Hz to 2700Hz, LPF from 100Hz to 20kHz
* Integrated SWR/power meter (HF)
* Automatic and manual Notch filter
* Switchable AGC (AGC) with adjustable attack rate
* Range map, with the ability to automatically switch modes
* Digital Noise Reduction (DNR), Pulse Noise Reduction (NB)
* CAT virtual COM port (FT-450 / TS-2000 emulation, RTS - PTT, DTR - CW)
* USB operation (audio transmission, IQ, CAT, KEY, PTT)
* RDS/CW/RTTY decoder, self-control, gauss filter
* SWR Graphs
* Spectrum analyzer
* FT8 receiver/transmitter
* Automatic send FT8 qso to log https://allqso.ru/
* WSPR Beacon
* VOX
* Equalizer TX/RX, reverber
* SSB/FM Scanner mode
* Support SDHC/SDSC/SDXC memory cards up to 16Gb
* WAV files radio recording and playback, fast broadcast recorded CQ message
* AGC takes into account the characteristics of human hearing (K-Weighting)
* TCXO frequency stabilization (it is possible to use an external clock source, such as GPS)
* WiFi operation: Time synchronization, virtual CAT interface (see Scheme/WIFI-CAT-instruction.txt)
* Tangent support Yaesu MH-36 и MH-48
* Hardware self-testing
* Firmware update over USB, SD, WiFi
* Digital pre-distortion
* And other.. (see menu)

### RF Parameters

Sensitivity at 10 dB SNR, LNA enabled:

* <150 MHz | -131 dBm | 63.0 nV
* 435 MHz | -121 dBm | 0.2 uV

Out-of-band emissions:

* 14 MHz -69 dBc
* 28 MHz -63 dBc
* 145 MHz -48 dBc -55 dBc

## Build

I ordered the boards in the Chinese service JLCPCB, they and their schemes are in the Scheme folder. <br>
After assembly, you need to flash FPGA and STM32 chips. <br>
If necessary, calibrate the transceiver through the appropriate menu <br>
WiFi module ESP-01 must have fresh firmware with SDK 3.0.4 and higher, and AT commands 1.7.4 and higher <br>


## Management

* **AF GAIN** - Volume
* **RIT / GAIN** - When the RIT function is active - smooth offset from the selected transceiver frequency. Inactive - IF gain control
* **ENC MAIN** - Main encoder for frequency control and menu settings
* **ENC 2** - Auxiliary encoder for menu operation. In normal mode, quickly switches the frequency, in CW fast change WPM
* **ENC 2[click]** - In CW mode, toggles between fast step and WPM selection, in other modes it opens the bandwidth properties.
* **BAND -** - Switch to the band below
* **BAND +** - Switch to higher band
* **MODE -** - Mode group switching SSB-> CW-> DIGI-> FM-> AM
* **MODE +** - Switching subgroup mod LSB-> USB, CW_L-> CW_U, DIGI_U-> DIGI_L, NFM-> WFM, AM-> IQ-> LOOP
* **FAST** - Mode of fast x10 rewinding of the frequency by the main encoder (configurable)
* **FAST [clamp]** - Frequency step settings
* **PRE** - Turn on the preamplifier (LNA)
* **PRE [clamp]** - Turn on the driver and / or amplifier ADC
* **ATT** - Turn on the attenuator
* **REC** - Save audio to SD card
* **CQ** - Play recorded CQ-message from SD card
* **MUTE** - Mute the sound
* **MUTE [clamp]** - Scanner mode
* **AGC** - Turn on AGC (automatic gain control)
* **AGC [clamp]** - AGC settings
* **A = B** - Setting the second bank of the receiver equal to the current
* **BW** - Switch to bandwidth selection menu (LPF)
* **BW [clamp]** - Switch to bandwidth selection menu (HPF)
* **TUNE** - Turn on the carrier for tuning the antenna
* **RF POWER** - Transmitter power selection
* **RF POWER [clamp]** - Squelch setting
* **A / B** - Switches between VFO-A / VFO-B receiver settings banks
* **A / B [clamp]** - Enable automatic mode change by bendmap
* **DOUBLE** - Turn on the dual receiver
* **DOUBLE [clamp]** - Switching between the modes of the dual receiver A&B (each channel of the headphones has its own path) or A + B (mixing signals of 2 receivers)
* **DNR** - Enable digital noise reduction
* **DNR [clamp]** - Turn on the impulse noise suppressor (NB)
* **NOTCH** - Turn on the automatic Notch filter to eliminate narrowband interference
* **NOTCH [clamp]** - Turn on the manual Notch filter to eliminate narrowband interference
* **SPLIT** - Allows you to split transmission and reception to different VFO banks
* **RIT [clamp]** - Enables RIT control from the front panel
* **WPM** - Switch to key speed selection menu (WPM)
* **WPM [clamp]** - Automatic key enable
* **MENU** - Go to the menu
* **MENU [clamp]** - Enable key lock LOCK
* **MENU [at power on]** - Reset transceiver settings
* **MENU [at power on+PRE]** - Reset transceiver settings and calibration
* **RIT** - Offset of the receiving frequency relative to the current frequency (TX without offset)
* **XIT** - Offset of the transmission frequency relative to the current frequency (RX without offset)

## Settings

### TRX Settings

* **Band Map** - Band Map, automatically switches mode depending on the frequency
* **Beeper** - Beep on key press
* **Callsign** - User callsign
* **Channel Mode** - Channel frequency mode (for LPD/PMR and etc.)
* **Custom Transverter** - Enable external transverter on non-ham band (just offset display frequency)
* **DEBUG Type** - Output of debug and service information to USB / UART ports
* **Encoder Accelerate** - Accelerate encoder on fast rates
* **Fine RIT Tune** - Fine or coarse tuning for RIT/XIT (encoder or resistor)
* **Freq Step xxx** - Frequency step by main encoder in each mode, Hz
* **FAST Step Multiplier** - Frequency step multiplier for FAST mode
* **ENC2 Step Mult** - Frequency step multiplier for ENC2 rotation (for CW/SSB)
* **Notch Step** - Frequency step of manual notch filter
* **Full Duplex** - Full duplex RX+TX mode
* **Locator** - User QTH locator
* **RIT Interval** - Offset range RIT (+ -)
* **XIT Interval** - Offset range XIT (+ -)
* **Split freq sync** - Synchronous VFO frequency change in SPLIT mode
* **Transverter XXcm** - Enable external ham band transverter support
* **URSI Code** - Ionogramm URSI Code https://digisonde.com/index.html#stationmap-section
* **Tropo Region** - Region for tropospheric forecast https://www.dxinfocentre.com/tropo.html 
* **Wolf Cluster** - Display Wolf TRX users on spectrum and send self data

### FILTERS Settings

* **AM/FM/CW/SSB LPF Stages** - Adjusting the slopes of the LPF filters in defferent modes
* **CW LPF Pass** - LPF cutoff frequency when working in CW
* **CW Gauss filter** - Gauss responce LPF filter
* **DIGI LPF Pass** - LPF cutoff frequency when working in DIGI
* **FM LPF Pass** - LPF cutoff frequency when working in FM
* **SSB HPF Pass** - HPF cutoff frequency when operating in SSB
* **SSB LPF Pass** - LPF cutoff frequency when operating in SSB
* **NOTCH Filter width** - Cutting width of Manual Notch filter

### RX Settings

* **ADC Dither** - Enable ADC dither for receiving weak signals
* **ADC Driver** - Turn on the preamplifier-ADC driver
* **ADC Preamp** - Turn on the preamplifier built into the ADC
* **ADC Randomizer** - Enables ADC digital line encryption
* **ADC Shutdown** - Turn off the ADC
* **AGC Gain target, LKFS** - Maximum AGC gain (Maximum volume with AGC on)
* **AGC Spectral** - Enable FFT-based AGC
* **AGC Threshold** - Enabling the AGC opening threshold
* **Att step, dB** - Attenuator tuning step
* **Attenuation, dB** - Current attenuation
* **Auto Snap** - Automaticly track and snap to near signal frequency (CW mode)
* **AutoGainer** - Automatic ATT / PREAMP control depending on the signal level on the ADC
* **A/B Balance** - Channels balance in dual RX modes A+B and A&B
* **CODEC Gain** - Adjusting AF gain in audio codec
* **DNR xxx** - Digital squelch adjustment
* **Squelch** - Enable SSB/AM/CW/FM Squelch
* **FM Squelch level** - FM squelch level
* **IF Gain, dB** - IF gain
* **NB Threshold** - Threshold of Noise blanker
* **Free tune** - Allows you to move the reception frequency across the spectrum without changing the center frequency of the spectrum itself
* **Center After Idle** - Center the spectrum with Free Tune active after 20 seconds of inactivity
* **Noise Blanker** - Activating the Surge Suppressor
* **RX AUDIO Mode** - Audio lines control: Stereo\Left\Right
* **RX AGC Hold time** - Time before release of AGC gain in signal peaks, ms
* **RX AGC Hold limit** - Burst limit for AGC delay time
* **RX AGC Hold step up** - AGC Delay Time Step on Peaks
* **RX AGC Hold step down** - AGC Delay Falloff Step at Peaks
* **RX AGC Max gain** - Limit of AGC gain, dB
* **RX AGC Speed** - AGC (automatic signal level control) response speed for reception (more-faster)
* **RX EQ xxx** - Receiver equalizer levels
* **TRX Samplerate** - Max FFT/samplerate on CW/SSB/NFM/DIGI/etc modes
* **FM Samplerate** - Max FFT/samplerate on NFM/WFM mode
* **VAD Threshold** - VAD voice detector threshold (noise suppressor for SSB mode and SCAN mode)
* **Volume step** - AF gain step by ENC2 (X1, Lite)
* **Volume** - AF gain
* **WFM Stereo** - select WFM stereo or mono decoder
* **WFM Stereo Modulation** - Stereo FM sub-carrier level

### TX Settings

* **ATU Cap** - Tuner Capacitance Combination
* **ATU Enabled** - Turning on the automatic antenna tuner
* **ATU Ind** - Combination of tuner inductances
* **ATU T** - Tuner capacitive arm position
* **ATU Mem step, kHz** - Step of saving tuner values for frequencies into memory
* **Auto Input Switch** - Auto input switch (PTT - mix, CAT - USB)
* **CESSB** - Enable controlled-envelope single-sideband modulation
* **CESSB Compress, dB** - Signal pre-amplification level before compression in CESSB
* **Compressor MaxGain** - Maximum comressor gain
* **Compressor Speed** - TX compressor response speed for transmission (more-faster)
* **CTCSS Frequency** - Transmit FM CTCSS sub-tone frequency
* **FT8 Auto CQ** - Automatic transition to CQ mode after FT8 communication
* **Input Type** - Select audio input (microphone, line in, USB)
* **LINE Gain** - Line input codec gain
* **MIC Boost** - +20db hardware mic amplifier
* **MIC EQ xxx** - Microphone equalizer levels
* **MIC Gain** - Microphone gain
* **MIC Noise Gate** - The level below which the microphone signal is muted, dbFS
* **MIC Reverber** - Microphone reverberator level
* **Power for each band** - Save power settings for each band individually
* **Power for each mode** - Save power settings for each mode individually
* **RF Power** - Transmission power,%
* **Repeater Mode"** -  Operation mode via repeater
* **Repeater offset, kHz"** -  Transmission frequency offset in repeater mode
* **SelfHear Volume** - Self Hearing (CW/DIGI) volume relative to the overall transceiver volume
* **TUNER Enabled** - Turning on the antenna tuner
* **Two Signal tune** - Two-signal generator in TUNE mode (1.0 + 2.2kHz)
* **VOX Threshold, dbFS** - VOX Threshold, dbFS
* **VOX Timeout, ms** - VOX transmission delay after silence, milliseconds
* **VOX** - TX Voice activation

### CW Settings

* **Auto CW Mode** - Automatic transition to CW mode when the key is pressed
* **CW In SSB** - Allow CW TX manipulation in SSB mode
* **DotToDash Rate** - Keyer dash to dot length rate
* **Edges smooth, ms** - Smoothing signal edges
* **Iambic Keyer** - Enable Iambic mode keyer
* **Iambic Type** - Type of Iambic keyer (A/B)
* **Key Invert** - Invert dash/dot on key
* **Key timeout** - Time before stopping the transmission mode after releasing the key
* **Keyer WPM** - Key Speed, WPM
* **Keyer** - Automatic Key
* **One symbol memory** - Keyer memory for one symbol for transmit
* **PTT Type** - RX/TX switch type for CW: by key or external PTT (tangent, etc.)
* **Pitch** - Detuning the receive generator from the transmit frequency
* **Self Hear** - Self-control CW (key press is heard)
* **Macros x** - Setting up macros for CW transmission
* **Macros x name** - Setting the macro name to be displayed in the interface

### SCREEN Settings

* **Bottom navi buttons** - Show bottm menu navigation buttons (for 7 inch displays)
* **Color Theme** - Select colors theme (0 - black, 1 - white, 2 - black with colored frequency)
* **Layout Theme** - Select interface theme (0 - default)
* **DX Cluster Type** - Selecting a Data Source for a DX Cluster
* **FFT 3D Mode** - Enable FFT 3D mode (0 - disabled, 1 - lines, 2 - pixels)
* **FFT Automatic** - Automatic FFT scale
* **FFT Averaging** - FFT burst averaging level. Full - automatic selection of upper and lower boundaries, Half - auto lower and manual upper, No - manual setting of the range of spectrum levels. (Remember to select the correct FFT Scale Type so that your range will fit on the screen)
* **FFT BW Style** - FFT bandwidth style: high opacity, low opacity, lines
* **FFT BW Position** - Position of the receiving band on FFT/WTF (everywhere \ at the top \ at the top + lines \ top + bottom with time delay)
* **FFT Background** - Gradient FFT background
* **FFT Color** - FFT colors: 1(blue -> yellow -> red), 2(black -> yellow -> red), 3(black -> yellow -> green), 4(black -> red), 5(black -> green), 6(black -> blue), 7(black -> white)
* **FFT Compressor** - Enable FFT peak comressor
* **FFT DXCluster Azimuth** - Add azimuth data to DX-cluster
* **FFT DXCluster Timeout** - Timeout of DX-cluser spots in minutes
* **FFT DXCluster** - Show DXCluster info over FFT
* **FFT Enabled** - Enable waterfall and FFT
* **FFT Freq Grid** - FFT and waterfall grids: 1(no grid), 2(fft grid), 3(fft+wtf grids), 4(wtf grid)
* **FFT Height** - FFT and waterfall proportional height
* **FFT Hold Peaks** - Show peaks on FFT spectrum
* **FFT Lens** - Enable FFT lens mode (magnify center of spectrum)
* **FFT Manual Bottom, dBm** - Bottom FFT threshold in manual mode
* **FFT Manual Top, dBm** - Top FFT threshold in manual mode
* **FFT Scale Type** - Y-axis style on FFT between signal amplitude and dBm to fit more dynamic range on screen (Useful when manually adjusting FFT range).
* **FFT Sensitivity** - Top threshold of FFT sensitivity in automatic mode (30 - scale to strongest signal)
* **FFT Speed** - FFT and waterfall speed
* **FFT Style** - FFT style: 1(gradient), 2(fill), 3(dots), 4(contour), 5(gradient + contour)
* **FFT Window** - Select FFT window (1-Dolph–Chebyshev 2-Blackman-Harris 3-Nutall 4-Blackman-Nutall 5-Hann 6-Hamming 7-No window)
* **FFT Zoom CW** - FFT spectrum zooming for CW Mode
* **FFT Zoom** - FFT spectrum zooming
* **FFT dBm Grid** - FFT signal power grid
* **LCD Brightness** - Set LCD brightness (not all lcd support)
* **LCD Sleep Timeout** - Idle time before the screen brightness decreases, sec (0 - disabled)
* **Show Sec VFO** - Show secondary VFO position on spectrum
* **WTF Color** - Waterfall colors: 1(blue -> yellow -> red), 2(black -> yellow -> red), 3(black -> yellow -> green), 4(black -> red), 5(black -> green), 6(black -> blue), 7(black -> white)
* **WTF Moving** - Mowe waterfall with frequency changing

### Decoders
* **CW Decoder** - Software CW receive decoder
* **CW Decoder Threshold** - CW receive decoder sensivity
* **RDS Decoder** - Enable RDS Decoder for WFM mode
* **RTTY Freq** - Central frequency of RTTY decoding
* **RTTY InvertBits** - RTTY invert 0 and 1 bits
* **RTTY Shift** - RTTY carrier shifts
* **RTTY Speed** - Speed of RTTY data
* **RTTY StopBits** - RTTY Stop bits

### WIFI Settings

* **WIFI Enabled** - Enable WiFi module (need restart after enable)
* **WIFI Network** - WiFi hotspot selection
* **WIFI Network Pass** - Set password for WiFi hotspot
* **WIFI Timezone** - Time zone (for updating the time via the Internet)
* **WIFI CAT Server** - Server for receiving CAT commands via WIFI
* **WIFI Update ESP firmware** - Start cloud autoupdate firmware for ESP-01 (if present)
* **ALLQSO.RU Token/LogId** - Token и LogID from https://allqso.ru/ web-log

### SD Card

* **File Manager** - Show SD Card file manager, support WAV playback and deleteing files, listening and broadcasting recordings, as well as updating firmware from a memory card
* **Record CQ message** - Record a short message for quick broadcast
* **USB SD Card Reader** - Enable USB SD Card reader
* **Export Settings** - Export settings and calibration data to SD card
* **Import Settings** - Import settings and calibration data from SD card
* **Format SD card** - Format media drive

### Satellites

* **SAT Mode** - Enable satellite mode
* **Download TLE** - Download current satellite data
* **Select SAT** - Select a satellite from the downloaded TLE file
* **QTH xxx** - Setting the Latitude\Longitude\Altitude of your location

### Calibration [appears by long pressing the MENU button in the settings menu]

* **ALC Port Enabled** - Activation of transceiver power control by an external amplifier via the ALC line: up to 1 volt - ALC is disabled and the power set in the transceiver is used. from 1.0v to 3.0v - power adjustment from 0 to 100%
* **ALC Inverted** - Invert ALC logic (3.0v - 0%, 1.0v - 100%)
* **AM Modulation Index** - Set TX AM Modulation Scale
* **ATU Averaging** - The number of steps of averaging SWR values during measurements in the operation of an automatic tuner
* **BPF x** - Bandpass filter parameters
* **CAT Type** - Type of CAT subsystem (FT-450 / TS-2000)
* **COM CAT DTR/RTS** - Configuring the functions of the DTR/RTS control lines of the CAT port
* **COM DEBUG DTR/RTS** - Configuring the functions of the DTR/RTS control lines of the Debug port
* **CICCOMP Shift** - Bit shift after CIC compensator
* **Calibrate reset** - Reset all calibrations to defaults
* **DAC Driver Mode** - DAC Driver OPA2673 bias mode (2 = 100% bias, 1 = 75% bias, 0 = 50% bias)
* **DAC Shift** - Bit shift of the output to the DAC
* **ENABLE 2200m/60m/6m/4m/FM/2m/AIR/Marine Band/70cm** - Enable hidden bands
* **EXT xxx** - External port control by band (EXT3, EXT2, EXT1, EXT0) - open drain
* **Encoder acceleration** - The encoder acceleration on faster rotation speeds
* **Encoder debounce** - Time of debouncing contacts of the main encoder
* **Encoder invert** - Invert the rotation of the main encoder
* **Encoder on falling** - The encoder is triggered only when level A falls
* **Encoder slow rate** - Deceleration rate of the main encoder
* **Encoder2 debounce** - Time of debouncing contacts of the additional encoder
* **Encoder2 invert** - Invert the rotation of the additional encoder
* **Encoder2 on fall** - Mode of operation of the secondary encoder (some encoders skip a step or make an extra one)
* **FAN Full start** - Temperature of the PA for starting the fan at full speed
* **FAN Medium start** - Temperature of the PA for starting the fan at medium speed
* **FAN Medium stop** - Temperature of the PA for stopping the fan
* **FAN Medium PWM** - Adjusting the duty cycle of a PWM fan in Medium mode
* **FM Deviation Scale** - Set TX FM Deviation Scale
* **Flash GT911** - Starting the touchpad update procedure according to LCD screen resolution
* **HPF START** - HPF filter parameters
* **IF Gain MIN/MAX** - IF Gain adjusting limits
* **INA226** - Activation and calibration of the INA226 voltage/current sensor on the I2C bus
* **KTY81 Calibration** - Calibration of the KTY81 sensor (setting the resistance of the divider arm)
* **LCD Rotate** - Rotate screen at 180 degrees
* **LNA Compensation** - Compensates the S-meter value when the LNA is turned on, dBm
* **ATT Compensation** - Compensates the S-meter value when the ATT is turned on
* **LPF END** - LPF filter parameters
* **Linear Pwr Control** - Sets a linear way to change the signal amplitude when adjusting the power (if disabled - logarithmic)
* **MAX ChargePump, kHz** - Maximum PWM frequency for ChargePump circuit
* **MAX PWR on Meter** - Maximim RF power (for indication)
* **MAX Power in TUNE** - Maximum RF power in Tune mode
* **MAX RF Temp** - Maximum temperature of the PA before the protection operation
* **MAX SWR** - Maximum VSWR before protection operation
* **NOTX xxx** - Disable TX on selected bands
* **OTA Update** - Enable OTA firmware update over WiFi
* **RF GAIN xxx** - Calibration of the maximum TX output power for each range
* **RF-Unit Type** - RF-Unit Type (QRP / BIG)
* **RTC COARSE CALIBR** - Very coarse clock crystal calibration
* **RTC FINE CALIBR** - Clock crystal calibration, one division is 0.954 ppm
* **S METER** - S-meter calibration
* **SSB Power addition** - Addition of RF power in SSB power, %
* **SWR FWD/BWD RATE** - Adjustment of the transformation ratio of the SWR meter (forward / return)
* **Swap USB IQ** - Swap USB IQ output
* **Settings reset** - Reset all settings to defaults
* **TCXO Frequency, kHz** - Frequency adjustment of the reference oscillator
* **VCXO Correction** - Correction for main generator frequency offset
* **Touchpad flip** - Flip the touchpad operation horizontally/vertically
* **Touchpad timeout** - Minimum time between taps (protection against false trigger), ms
* **Touchpad click threshold** - Minimum time for triggering a short click, ms
* **Touchpad click timeout** - Maximum time for triggering a short click, ms
* **Touchpad hold timeout** - Time before a long press is triggered, ms
* **Touchpad swipe threshold** - Minimum distance to trigger a swipe, px
* **TSignal Balance** - Sets the power balance between signals in Two signal tune mode
* **TX CICCOMP Shift** - Bit shift after TX CIC compensator
* **TX Start Delay** - Delay before the RF signal is applied (ms), so that the relay has time to trigger
* **Tangent Type** - Select tangent type
* **Transverter Offset, MHz** - Offset frequency shown on the display (for custom transverters)
* **Transverter xxx IF, MHz** - Setting the IF frequency of the transverter
* **Transverter xxx RF, MHz** - Setting the RF frequency of the transceiver to match the IF

### Memory Channels

* Changing names for transceiver memory channels

### Set Clock Time

* Clock setting

### DFU Mode

* Launch STM32 USB firmware update over DFU mode

### OTA Update

* Launch FPGA/STM32 firmware update over internet

### System info

* Show info about TRX

## Services

### Spectrum Analyzer

* **Spectrum START** - Start the spectrum analyzer
* **Begin, kHz** - Starting frequency of the analyzer with a step of 1 kHz
* **End, kHz** - End frequency of the analyzer in 1 kHz steps
* **Top, dBm** - Upper threshold of the graph
* **Bottom, dBm** - Lower threshold of the graph

### WSPR Beacon

* **WSPR Beacon START** - Start WSPR beacon
* **Freq offset** - Offset from center of WSPR band
* **WSPR Band xxx** - Select bands to WSPR beacon

### DX Cluster 

* Show DX Cluster info by current band (from internet)

### Propagation 

* Show propagation statistics (from internet)

### Tropo 

* Show tropospheric forecast

### DayNight Map 

* Displaying the day/night terminator on the map (from internet)

### Ionogram 

* Displaying the ionogram for the selected URSI Code (from internet)

### SWR Analyser

* Run SWR analizer (Band SWR - on current band, HF SWR - on all HF bands, Custom SWR - in selected range)

### RDA Statistics 

* Print statistics from RDA award (from internet)

### FT8

* FT8 Receiver / transmitter

### Locator Info

* Calculate distance and azimuth from QTH-locator

### Callsign Info

* Show callsign data

### Time Beacons

* List of beacons of the exact time (for adjusting the frequency of reception by carrier)

### Self Test

* Run hardware self diagnostic

### Auto calibration

* **Calibrate SWR** - Assistant in measuring SWR and power meter
* **Calibrate Power** - Assistant in adjusting RF Power output