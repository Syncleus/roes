#ifndef _SWR_STRINGS_H_
#define _SWR_STRINGS_H_

#include <avr/pgmspace.h>

#define FS(s) reinterpret_cast<const char *>F(s)

#define CORRUPT_EEPROM FS("EEPROM is corrupt")
#define CRC_CHECK_FAILED FS("CRC check failed")
#define SWR_LABEL FS("SWR")
#define FWD_LABEL FS("Fwd")
#define RVR_LABEL FS("Rvr")
#define RL_LABEL FS("RL")
#define PHS_LABEL FS("Phs")
#define WATTS_UNIT_LABEL FS("w")
#define DECIBEL_UNIT_LABEL FS("dB")
#define STOP_WARNING_LABEL FS("STOP")
#define TRANSMITTING_LABEL FS("transmitting")
#define CALIBRATE_LABEL FS("Calibrate")
#define CALIBRATE_LINE_1A FS("Apply ")
#define CALIBRATE_LINE_1B FS(" into")
#define CALIBRATE_LINE_2_OPEN FS("an open load")
#define CALIBRATE_LINE_2_DUMMY FS("a dummy load")
#define ERROR_WARNING_LABEL FS("ERROR!")
#define COMMANDS_OVERVIEW_HELP FS("Availible commands:")
#define CALIBRATEONBOOT_INVALID_ARGUMENT FS("Invalid argument, argument to calibrateonboot command must be either 'on' or 'off'")
#define DEMO_INVALID_ARGUMENT FS("Invalid argument, argument to demo command must be either 'on' or 'off'")
#define CALIBRATIONPOINTS_DUMMY_LABEL FS("calibration points [dummy]: ")
#define CALIBRATIONPOINTS_OPEN_LABEL FS("calibration points [open]: ")
#define ONE_SPACE   FS(" ")
#define TWO_SPACE   FS("  ")
#define THREE_SPACE FS("   ")
#define FOUR_SPACE  FS("    ")
#define OPEN_LABEL FS("open")
#define CALIBRATIONPOINTS_SET FS("calibration points set")
#define EEPROM_CLEARED FS("Eeprom cleared.")
#define CALIBRATIONDATA_HEADER_1 FS("Calibration data for ")
#define CALIBRATIONDATA_HEADER_2_DUMMY FS("w into a dummy load")
#define CALIBRATIONDATA_HEADER_2_OPEN FS("w into an open load")
#define CALIBRATIONDATA_FWD FS("        fwd: ")
#define CALIBRATIONDATA_RVR FS("        refl: ")
#define CALIBRATIONDATA_MAGNITUDE FS("  magnitude: ")
#define CALIBRATIONDATA_PHASE FS("      phase: ")
#define CALIBRATIONDATA_VREF FS("       vref: ")
#define READINPUTS_FWD FS("POWER_FWD_PIN: ")
#define READINPUTS_RVR FS("POWER_RVR_PIN: ")
#define READINPUTS_VREF FS("POWER_VREF_PIN: ")
#define READINPUTS_PHASE FS("POWER_PHASE_PIN: ")
#define READINPUTS_MAGNITUDE FS("POWER_MAGNITUDE_PIN: ")
#define CALIBRATEONBOOT_LABEL FS("calibrateonboot: ")
#define CALIBRATEONBOOT_ON FS("on")
#define CALIBRATEONBOOT_OFF FS("off")
#define CALIBRATEONBOOT_ACTIVATING FS("Activating calibrateonboot.")
#define CALIBRATEONBOOT_DEACTIVATING FS("Deactivating calibrateonboot.")
#define PONG_LABEL FS("Pong!")
#define DEMO_ON CALIBRATEONBOOT_ON
#define DEMO_OFF CALIBRATEONBOOT_OFF
#define DEMO_LABEL FS("demo: ")
#define DEMO_ACTIVATING FS("Activating demo.")
#define DEMO_DEACTIVATING FS("Deactivating demo.")
#define CALIBRATIONPOINTS_TOO_MANY FS("Too many calibration points! max: ")
#define CALIBRATIONPOINTS_TOO_FEW FS("Must provide at least one calibration point!")
#define HELP_COMMANDS_1 FS("  help")
#define HELP_COMMANDS_2 FS("  calibrationpoints")
#define HELP_COMMANDS_3 FS("  cleareeprom")
#define HELP_COMMANDS_4 FS("  readinputs")
#define HELP_COMMANDS_5 FS("  calibrationdata")
#define HELP_COMMANDS_6 FS("  calibrateonboot")
#define HELP_COMMANDS_7 FS("  demo")
#define HELP_COMMANDS_8 FS("  ping")
#define POWER_LABEL FS("Pwr")
#define POWER_FWD_LABEL FS("Fwd:")
#define DBM_UNIT_LABEL FS("dBm")
#define POWER_RVR_LABEL FS("Rvr:")
#define SWR_SOURCE_INFO FS("SWR Source: ")
#define SWR_SOURCE_DIFFERENTIAL FS("differential")
#define SWR_SOURCE_ENVELOPE FS("envelope")
#define SWR_SOURCE_DIFFERENTIAL_SET FS("SWR source is now the differential AD8302.")
#define SWR_SOURCE_ENVELOPE_SET FS("SWR source is now the envelope detector.")
#define SWR_SOURCE_INVALID FS("Invalid arguments, accepts either 'differential' or 'envelope' as arguments.")
#define HELP_COMMANDS_9 FS("  swrsource")
#define CALIBRATIONDATA_PHASE_SHIFTED FS("      phase shifted: ")
#define CALIBRATIONDATA_VREF_SHIFTED FS("       vref shifted: ")
#define COMPLEX_PLUS_STRING FS(" + ")
#define COMPLEX_I_STRING FS("i")

void uint32toa(uint32_t value, char *buffer, uint8_t radix);
char* splitString(char* data, char separator);

#endif /* _SWR_STRINGS_H_ */
