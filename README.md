## Hardware: Recommend to use this board:
[Github Link](https://github.com/johanneswilkens/Heat-Pump-Controller-PCB)

## State of main branch: 
Current state of main: new feature (switch of HP when backup heat runs) not fully tested  
Seems to run fine, but not tested all cases yet, so please report.
Remains experimental (so use at your own risk) but functions good in my setup

## Enable/Disable external thermostat/relays:
If you are not using an external thermostat/pump/backup_heater choose the correct packages in esp-lg-control.yaml (lines 9-19).  
Comment the config that you don't use and uncomment the used file. Always make sure 1 file is commented and one is uncommented per set op files.  
Example without external thermostat and without external relays:  

```yaml
  #Choose correct version of below two files. Always enable (by uncommenting) only one of the two
  #*****
  #with_external_thermostat: !include lg-heatpump-control/sensors/lg-with-external-thermostat.yaml
  without_external_thermostat: !include lg-heatpump-control/sensors/lg-without-external-thermostat.yaml
  #*****
  
  #Again Choose correct version of below two files. Always enable (by uncommenting) only one of the two
  #*****
  #with_external_relays: !include lg-heatpump-control/sensors/lg-with-external-relays.yaml 
  without_external_relays: !include lg-heatpump-control/sensors/lg-without-external-relays.yaml 
  #*****
```

## esp-lg-control -> Hobby project!
ESP32 based controller for LG Therma V Monoblock Unit.
Connects the LG to HomeAssistant and tries to optimise output modulation.
Tested with 3-phase 14kW U34 version. So propably works at least with 12-14-16kW U34 3-phase models. May also work with 1-phase and U44 models (depending on modbus registers). But not tested.

This is a hobby project. I share this to allow other people to pick up ideas and contribute. All assistance is welcome. I will not provide support, so use at your own risk. And make sure you know what you are doing, by using this script it is possible that your unit stops functioning or breaks and you could electrocute yourself.

## UPDATES
March 2025
*  Redesigned breakout logic. It now acts on compressor frequency. When > 85 % (adjustable) silent mode is switched on for xx (adjustable) minutes.
*  Added a defrost-boost routine. Routine increases the heating curve with x °C (adjustable) to compensate for loss of energy transfer when time between defrosts < 50 mins (adjustable). The boost is disabled when time between defrosts > 5 min above boost threshold. Changes in boost value take effect on the next defrost.
* Several small enhancements and bug fixes

## What is different
This version is not up to date withn the original version (yet).
Compared to the orginal several functions are added/modified (Status January 28, 2025):
* The modbus yaml inludes a code to read an Eastron SDM72 KWh meter
* The LG RMC is used as temperature sensor for an ESPhome thermostat to control the heatpump. Heatpump is switched on/off via modbus, no wires thermostat connection is needed. Beware of the poor resolutionof the RMC sensore via modbus (0.5 °C). Is a temporary solution, pending design of dedicated LVGL based thermostat.
* Windchill (feels like) temperature is read from Home Assistant (HA reads value from API of local weather station). Value is used with adjustable factor to influence heaatcurve value of HP water setting (Ta).
* Routine added to increase boost heat curve with adjustable value when time between defrosts is rather short (<50 min), boost is disableb when time between defrosts > 55 min (needs testing). 
* When silent after defrost is selected, option added to step up to target setting in 4 steps. Reason, attempt to minimize overshoot after defrost.
* Few routines to deal with known firmware bugs:
  * Break out logic to deal with the EEV valve issue. Rountine increases targets for a few minutes when compressor T > 73 °C. (Not tested, error occured only during freezing weather)
  * Degree-minutes control, when Ta stays below target for adjustible degree-minutes value, STALL state enters and Stall is allowed to adjust target setting (enabling STALL under other conditions is not recommended).Original target restored once STALL managed to raise Ta to heatcurve target. Routine is added as solution whenduring freezing waether target is not reached while HP is not running on full power (waiting for the right winter weather to test this routine)
* Experimental routine to switch Silent mode automatically when outside temperatures < -5, hence RH is lower. Uses outside temperature, dewpoint, RH and evaporator temperature. Reason is surplus capacity of HP and very low noise level in silent mode. Waiting for a cold winter to test. Anyhow switches silent of when approaching typical defrost circumstances.
Added software is tested with HP controlled at water outlet temperature and without backup heater (Full Electric only)

## Whats new 

~~Experimental (simple) PID controller. It works, but I am not sure if it really addes value compared to the simple algoritm. It fixes some issues (with HP not starting, because target is set too low). But there are other ways to fix that.
But it is a lot of fun to experiment with.~~

## Known limitations/ToDo:
* Further testing on different models needed

## Hardware
Works with any ESP chip/board supported by ESPHome.

**Recommend to use the board designed by johanneswilkens:** [Github Link](https://github.com/johanneswilkens/Heat-Pump-Controller-PCB)

In this specific config four relays are connected to GPIO Pins. These relays actuate the external thermostat connections on the LG. This config is optional.

A Nest external thermostat is connectend to a GPIO Pin (with pulldown resistor). The Nest Ketel Module connects this pin to a 3.3V supply to create a digital 'high' when the thermostat requests heat. This config is optional. It can be replaced by a template switch. The switch is nesscesary as this start the control algoritm.

Modbus communication is done through a rs485 to TTL converter based on the max485 chip. I use a board with automatic flow control. You can use other boards if you like. 

## Installation
* Clone repo
* Copy config/config-example.yaml to config/config.yaml
* Update config.yaml
* Update secrets.yaml
* Adapt to your own setup als needed
* Build

## Disclaimer
This is an experimental script and in early alpha stage. This is in no way finished software and should only be used by professionals. You are using this on your own riks. Do not use if you don't have any soldering/programming experience. Pull request are welcome! 
