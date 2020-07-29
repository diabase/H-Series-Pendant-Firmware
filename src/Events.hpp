/*
 * Events.hpp
 *
 *  Created on: 6 Jan 2017
 *      Author: David
 */

#ifndef SRC_EVENTS_HPP_
#define SRC_EVENTS_HPP_

// Event numbers, used to say what we need to do when a field is touched
// *** MUST leave value 0 free to mean "no event"
enum Event : uint8_t
{
	evNull = 0,						// value must match nullEvent declared in Display.hpp

	evDefaultRoot, evPendantRoot,

	// Page selection
	evTabControl, evTabPrint, evTabMsg, evTabSetup,

	// Pendant-mode page selection
	evTabJog, evTabOffset, evTabJob,

	// Pendant jog buttons
	evPJogAxis, evPJogAmount,

	// Pendant tool selection
	evToolSelect,

	evMeasureZ, evProbeWorkpiece, evTouchoff, evSetToolOffset, evFindCenterOfCavity, evZeroAxisInWCS,

	// Heater control
	evSelectHead, evAdjustActiveTemp, evAdjustStandbyTemp,

	// Control functions
	evMovePopup, evExtrudePopup, evFan, evListMacros,
	evMoveX, evMoveY, evMoveZ, evMoveU, evMoveV, evMoveW,	// these 6 must be contiguous and in this order
	evExtrudeAmount, evExtrudeRate, evExtrude, evRetract,
	evExtrudeAmountP, evExtrudeRateP, evExtrudeP, evRetractP,

	// Print functions
	evExtrusionFactor,
	evAdjustFan,
	evAdjustInt,
	evSetInt,
	evListFiles,

	evFile, evMacro, evMacroControlPage,
	evPrintFile,
	evSendCommand,
	evFactoryReset,
	evAdjustSpeed,
	evPAdjustExtrusionPercent, // TODO: remove as soon as we have extruder number
	evPAdjustSpindleRPM,

	evScrollFiles, evScrollMacros, evFilesUp, evMacrosUp, evChangeCard,

	evKeyboard,

	// Setup functions
	evCalTouch, evSetBaudRate, evInvertX, evInvertY, evAdjustBaudRate, evSetVolume, evAdjustVolume, evSetInfoTimeout, evAdjustInfoTimeout, evReset,

	evYes,
	evCancel,
	evDeleteFile,
	evSimulateFile,
	evPausePrint,
	evResumePrint,
	evBabyStepPopup, evBabyStepAmount,

	evKey, evShift, evBackspace, evSendKeyboardCommand, evUp, evDown,

	evAdjustLanguage, evSetLanguage,
	evAdjustColours, evSetColours,
	evBrighter, evDimmer,
	evSetDimmingType,

	evEmergencyStop,

	evJogZ, evCloseAlert

#ifdef OEM_LAYOUT
	, evOemJogPage, evOemSetIncrement, evOemAdjustUpDown, evOemDone, evOemSetBrightness, evOemAxisSelect, evOemDistanceSelect, evOemCardSelect
#endif
};

#endif /* SRC_EVENTS_HPP_ */
