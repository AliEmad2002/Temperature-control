# Prototyping:

## Introduction:

At prototyping phase, multiple amplifiers are to be tested and compared
with each other. For each of these tests, a small PCB with pre-defined
inputs and outputs is to be printed. On the main PCB, there is a
pre-defined socket / header for this small amplifier test PCB. This
accelerates prototyping phase and saves money.

1.  Amplifier's pre-defined block diagram:
 
   ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/95e95426-62af-4128-9db6-a1842bb5dd44)


3.  Amplifier's pre-defined input / output header position:

![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/0eabcd61-d681-4fa6-add0-235504eaeda4)


# Tested amplifiers:

##  Single BJT transistor amplifier:

Using BC337 BJT NPN transistor, the following circuit is implemented. It
gives a gain of 5 \[V/V\], enabling an ADC resolution of
$\frac{1}{4096}*\frac{3.3}{5} \approx 161\mu V/LSB$

![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/c9e80349-e4bf-482d-a522-7b8057c19882)


### Design notes:

-   As BJT amplifies $I_{B}$ at $I_{C}$, and since
    $I_{C} = \frac{3.3 - v_{o}}{R_{C}}$, value of $R_{C}$ must be low
    enough such that the amplified current is achievable across it. This
    could also be achieved by checking that
    $v_{CE,\ Fully\ On} \cong 0.2V$. Otherwise, if
    $v_{CE,\ Fully\ On} \cong few\ mV$, the operation will not be as
    expected.

### Disadvantages:

-   Has a low gain. To obtain higher gain transistors should be
    cascaded, which takes more cost when compared to the following
    circuit.

-   Automatic offset calibration is not stable.

## Op-Amp based amplifier:

-   Using UA741CN Op-Amp, the following circuit is implemented. It gives
    a gain of 33 \[V/V\], enabling an ADC resolution of
    $\frac{1}{4096}*\frac{3.3}{33} \approx 24.4\mu V/LSB$. It also
    provides stable automatic offset calibration.

![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/7e8926f5-78db-4b62-bcbf-2a3ff68d7047)


### Design notes:

-   As this is -- obviously -- an inverting amplifier, thermocouple is
    connected in reverse to obtain positive output.

-   **CTRL2** signal is used to select whether thermocouple is connected
    to the circuit or not. This is achieved by switching **CTRL2**
    between 0-volts level, and open-circuit level respectively.

-   **CTRL1** signal is used to select whether circuits input voltage is
    a fixed 0-volts, or thermocouple voltage. This is achieved by
    switching **CTRL1** between 0-volts level, and open-circuit level
    respectively.

-   **CTRL3** is fixed at 0-volts level all time, and used as a **GND**
    for the op-amp circuit. This is done so that when **CTRL1** or
    **CTRL2** signals are low, they appear as real zeros to the op-amp.
    As low level voltage of the I/O module in the MCU is always a little
    value above zero (real circuit **GND**).

-   To calibrate offset, thermocouple is disconnected (**CTRL2** is
    open-circuit), and amplifier's input is set to zero (**CTRL1** is
    zero). Hence, output value is equal to that of the offset).

-   To obtain thermocouple's voltage, thermocouple is -- of course -
    connected (**CTRL2** is low), and amplifier's input is dedicated to
    TC only (**CTRL1** is open-circuit). Hence, output value is equal
    to:

    ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/5a5d7dec-b789-4011-b81c-bc7fd8c2eb34)

 By subtracting offset value (previously obtained at a periodic
 calibration) from output's value, thermocouple's voltage can be
 obtained as follows:
 
 ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/8116687f-bcb0-440d-9efe-1180ad474d54)


# Temperate calculation:

As thermocouple temperature calculation depends on its voltage, and
temperature of the -- virtual -- cold junction. Temperature of cold
junction is assumed to be room temperature, which could be measured
using STM32's on-chip temperature sensor.

Temperature can be calculated as follows:
![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/fbe1d865-a0d8-4501-9de6-a61312b0ef69)

### In the above equation:
-   The term ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/161006cf-5436-459a-af28-b2df7b0c0822)
 is
    thermocouple's voltage ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/98d090fb-3098-4509-9f8b-17cd467ff7ae)
 (Previous section
    discussed how to obtain it).

-   The term ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/fb8a0fff-e5ae-4892-8ddd-256bf0fe3618)
could be obtained by interpolating output of the on-chip temperature
    sensor, in thermocouple's reference table.

-   Hence, the term ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/90473bc6-2727-43f4-952e-d264048aa52e)
 can be obtained from
    the equation as follows:
    ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/5adbce2b-f01c-4b96-ae38-c8867901ec4d)


-   Voltage ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/5524a1a0-d8ce-4411-b735-7b9675c14b82)
 is then interpolated in
    thermocouple's reference table to obtain value of ![image](https://github.com/AliEmad2002/Temperature-control/assets/99054912/40c81605-2ee4-4bcd-af6f-93cfb6206087)
.

# Noise elimination:

## Temperature noise:

From the previous calculation, the obtained temperature is coupled with
noise (Due to amplifier circuitry). These are eliminated using a SW
defined LPF.

## On / Off control hysteresis:

To avoid multiple fast relay switching which may affect both relay
circuit and the load device, a hysteresis filter is defined.

Also, relay protection SW is implemented. All it does is inserting
minimum delay between switches requested by the controlling SW.

# I/O:

-   Set-point temperature is controlled using the on-board rotary
    encoder.

-   Set-point temperature and real temperature are shown on a
    multiplexed 3-digit 7-segment display.

# Porting and configuration:

-   This SW is built to be portable for variety of MCU's, just replace
    FreeRTOS and COTS-OS/MCAL port files with these of the used target
    MCU.

-   SW is modular and can be easily modified when using different HW
    setup, or when implementing additional features.

# Prototype:
