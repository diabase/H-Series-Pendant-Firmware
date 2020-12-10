/*
 * UserInterfaceConstants.hpp
 *
 *  Created on: 10 Jan 2017
 *      Author: David
 */

#ifndef SRC_USERINTERFACECONSTANTS_HPP_
#define SRC_USERINTERFACECONSTANTS_HPP_

#include "Configuration.hpp"
#include "DisplaySize.hpp"
#include "Library/Misc.hpp"

const size_t NumColourSchemes = 3;

#ifdef OEM_LAYOUT

#include "OemUserInterfaceConstants.hpp"

#else

static const char* _ecv_array const axisNames[] = { "X", "Y", "Z", "U", "V", "W", "A", "B", "C", "D" };
constexpr size_t MaxTotalAxes = ARRAY_SIZE(axisNames);
static const char* _ecv_array const jogAxes[]  = { "X", "Y", "Z", "A", "C" };
static const char* _ecv_array const wcsNames[] = { "G54", "G55", "G56", "G57", "G58", "G59", "G59.1", "G59.2", "G59.3" };
static const uint8_t ProbeToolIndex = 10;
static const char* _ecv_array const babystepAmounts[] = { "0.01", "0.02", "0.05", "0.1" };
static const float _ecv_array babystepAmountsF[] = { 0.01, 0.02, 0.05, 0.1 };

#if DISPLAY_X == 480

const unsigned int MaxHeaters = 5;
#define MaxDisplayableAxes	(4)

const PixelNumber margin = 2;
const PixelNumber textButtonMargin = 1;
const PixelNumber iconButtonMargin = 1;
const PixelNumber outlinePixels = 2;
const PixelNumber fieldSpacing = 6;
const PixelNumber statusFieldWidth = 156;
const PixelNumber bedColumn = 114;

const PixelNumber rowTextHeight = 21;	// height of the font we use
const PixelNumber rowHeight = 28;
const PixelNumber rowHeightP = rowHeight; // set them equal for now
const PixelNumber moveButtonRowSpacing = 12;
const PixelNumber extrudeButtonRowSpacing = 12;
const PixelNumber fileButtonRowSpacing = 8;
const PixelNumber keyboardButtonRowSpacing = 6;		// small enough to show 2 lines of messages

const PixelNumber speedTextWidth = 70;
const PixelNumber efactorTextWidth = 30;
const PixelNumber percentageWidth = 60;
const PixelNumber e1FactorXpos = 140, e2FactorXpos = 250;

const PixelNumber messageTimeWidth = 60;

const PixelNumber popupY = 192;
const PixelNumber popupSideMargin = 10;
const PixelNumber popupTopMargin = 10;
const PixelNumber keyboardTopMargin = 8;

const PixelNumber popupFieldSpacing = 10;

const PixelNumber axisLabelWidth = 26;
const PixelNumber firstMessageRow = margin + rowHeight + 3;		// adjust this to get a whole number of message rows below the keyboard

const PixelNumber progressBarHeight = 10;
const PixelNumber closeButtonWidth = 40;

const PixelNumber touchCalibMargin = 15;

const PixelNumber ColourGradientWidth = 2 * 128;

extern uint8_t glcd19x21[];				// declare which fonts we will be using
#define DEFAULT_FONT	glcd19x21

#elif DISPLAY_X == 800
const unsigned int MaxHeaters = 7;
#define MaxDisplayableAxes (6)
#define MaxDisplayableAxesP (5)
#define MaxAxesPendant (9)
constexpr size_t MaxPendantTools = 6;

const PixelNumber margin = 4;
const PixelNumber textButtonMargin = 1;
const PixelNumber iconButtonMargin = 2;
const PixelNumber outlinePixels = 3;
const PixelNumber fieldSpacing = 12;
const PixelNumber statusFieldWidth = 228;
const PixelNumber bedColumn = 160;

const PixelNumber rowTextHeight = 32;	// height of the font we use
const PixelNumber rowHeight = 48;
const PixelNumber rowHeightP = rowHeight; // set them equal for now
const PixelNumber moveButtonRowSpacing = 20;
const PixelNumber extrudeButtonRowSpacing = 20;
const PixelNumber fileButtonRowSpacing = 12;
const PixelNumber keyboardButtonRowSpacing = 12;

const PixelNumber speedTextWidth = 105;
const PixelNumber efactorTextWidth = 45;
const PixelNumber percentageWidth = 90;
const PixelNumber e1FactorXpos = 220, e2FactorXpos = 375;

const PixelNumber messageTimeWidth = 90;

const PixelNumber popupY = 345;
const PixelNumber popupSideMargin = 20;
const PixelNumber popupTopMargin = 20;
const PixelNumber keyboardTopMargin = 20;
const PixelNumber popupFieldSpacing = 20;

const PixelNumber axisLabelWidth = 40;
const PixelNumber firstMessageRow = margin + rowHeight;		// adjust this to get a whole number of message rows below the keyboard

const PixelNumber progressBarHeight = 16;
const PixelNumber closeButtonWidth = 66;

const PixelNumber touchCalibMargin = 22;

const PixelNumber ColourGradientWidth = 3 * 128;

extern uint8_t glcd28x32[];				// declare which fonts we will be using
#define DEFAULT_FONT	glcd28x32

#else

#error Unsupported DISPLAY_X value

#endif

const PixelNumber buttonHeight = rowTextHeight + 4;
const PixelNumber tempButtonWidth = (DISPLAY_X + fieldSpacing - bedColumn)/MaxHeaters - fieldSpacing;

const PixelNumber row1 = 0;										// we don't need a top margin
const PixelNumber row2 = row1 + rowHeight - 2;					// the top row never has buttons so it can be shorter
const PixelNumber row3 = row2 + rowHeight;
const PixelNumber row4 = row3 + rowHeight;
const PixelNumber row5 = row4 + rowHeight;
const PixelNumber row6 = row5 + rowHeight;
const PixelNumber row6p3 = row6 + (rowHeight/3);
const PixelNumber row7 = row6 + rowHeight;
const PixelNumber row7p7 = row7 + ((2 * rowHeight)/3);
const PixelNumber row8 = row7 + rowHeight;
const PixelNumber row8p7 = row8 + ((2 * rowHeight)/3);
const PixelNumber row9 = row8 + rowHeight;
const PixelNumber rowTabs = DisplayY - rowTextHeight;			// place at bottom of screen with no margin
const PixelNumber labelRowAdjust = 2;							// how much to drop non-button fields to line up with buttons

const PixelNumber row1P = margin/2;										// we don't need a top margin
const PixelNumber row2P = row1P + rowHeightP;						// the top row never has buttons so it can be shorter
const PixelNumber row3P = row2P + rowHeightP;
const PixelNumber row4P = row3P + rowHeightP;
const PixelNumber row5P = row4P + rowHeightP;
const PixelNumber row6P = row5P + rowHeightP;
const PixelNumber row7P = row6P + rowHeightP;
const PixelNumber row8P = row7P + rowHeightP;
const PixelNumber row9P = row8P + rowHeightP;
const PixelNumber row10P = row9P + rowHeightP;
const PixelNumber row11P = row10P + rowHeightP;
const PixelNumber row12P = row11P + rowHeightP;
const PixelNumber row13P = row12P + rowHeightP;
const PixelNumber row14P = row13P + rowHeightP;
const PixelNumber row15P = row14P + rowHeightP;
const PixelNumber rowTabsP = DisplayYP - rowTextHeight - 2;			// place at bottom of screen with no margin

const PixelNumber ColourGradientLeftPos = DISPLAY_X - ColourGradientWidth - margin;
const PixelNumber ColourGradientTopPos = row2;
const PixelNumber ColourGradientHeight = rowTextHeight;

const PixelNumber speedColumn = margin;
const PixelNumber fanColumn = DISPLAY_X/4 + 20;

const PixelNumber pauseColumn = DISPLAY_X/2 + 10 + fieldSpacing;
const PixelNumber resumeColumn = pauseColumn;
const PixelNumber cancelColumn = pauseColumn + (DISPLAY_X - pauseColumn - fieldSpacing - margin)/2 + fieldSpacing;
const PixelNumber babystepColumn = cancelColumn;

const PixelNumber fullPopupWidth = DisplayX - (2 * margin);
const PixelNumber fullPopupHeight = DisplayY - (2 * margin);
const PixelNumber popupBarHeight = buttonHeight + (2 * popupTopMargin);
const PixelNumber fullPopupWidthP = DisplayXP - (2 * margin);
const PixelNumber fullPopupHeightP = DisplayYP - (2 * margin);

const PixelNumber tempPopupBarWidth = (3 * fullPopupWidth)/4;
const PixelNumber tempPopupBarWidthEncoder = 150;
const PixelNumber rpmPopupBarWidth = fullPopupWidth;
const PixelNumber fileInfoPopupWidth = fullPopupWidth - (4 * margin),
				  fileInfoPopupHeight = (10 * rowTextHeight) + buttonHeight + (2 * popupTopMargin);
const PixelNumber areYouSurePopupWidth = DisplayX - 80,
				  areYouSurePopupHeight = (3 * rowHeight) + (2 * popupTopMargin);
const PixelNumber areYouSurePopupWidthP = DisplayXP - 80,
				  areYouSurePopupHeightP = (3 * rowHeightP) + (2 * popupTopMargin);

const PixelNumber movePopupWidth = fullPopupWidth;
const PixelNumber movePopupHeight = ((MaxDisplayableAxes + 1) * buttonHeight) + (MaxDisplayableAxes * moveButtonRowSpacing) + (2 * popupTopMargin);

const PixelNumber extrudePopupWidth = fullPopupWidth;
const PixelNumber extrudePopupHeight = (5 * buttonHeight) + (4 * extrudeButtonRowSpacing) + (2 * popupTopMargin);
const PixelNumber extrudePopupWidthP = fullPopupWidthP;
const PixelNumber extrudePopupHeightP = (8 * buttonHeight) + (4 * extrudeButtonRowSpacing) + (5 * fieldSpacing) + (2 * popupTopMargin);

const PixelNumber keyboardButtonWidth = DisplayX/5;
const PixelNumber keyboardPopupWidth = fullPopupWidth;
const PixelNumber keyButtonWidth = (keyboardPopupWidth - 2 * popupSideMargin)/16;
const PixelNumber keyButtonHStep = (keyboardPopupWidth - 2 * popupSideMargin - keyButtonWidth)/12;
const PixelNumber keyButtonVStep = buttonHeight + keyboardButtonRowSpacing;
const PixelNumber keyboardPopupHeight = (5 * keyButtonVStep) + (2 * keyboardTopMargin) + buttonHeight;
const PixelNumber keyboardPopupY = margin;

const unsigned int NumFileColumns = 1;
const unsigned int NumFileRows = (fullPopupHeight - (2 * popupTopMargin) + fileButtonRowSpacing)/(buttonHeight + fileButtonRowSpacing) - 1;
const unsigned int NumDisplayedFiles = NumFileColumns * NumFileRows;

const PixelNumber fileListPopupWidth = fullPopupWidth;
const PixelNumber fileListPopupHeight = ((NumFileRows + 1) * buttonHeight) + (NumFileRows * fileButtonRowSpacing) + (2 * popupTopMargin);

const unsigned int NumMacroColumns = 2;
const unsigned int NumMacroRows = (fullPopupHeight - (2 * popupTopMargin) + fileButtonRowSpacing)/(buttonHeight + fileButtonRowSpacing) - 1;
const unsigned int NumDisplayedMacros = NumMacroColumns * NumMacroRows;

const PixelNumber MacroListPopupWidth = fullPopupWidth;
const PixelNumber MacroListPopupHeight = ((NumMacroRows + 1) * buttonHeight) + (NumMacroRows * fileButtonRowSpacing) + (2 * popupTopMargin);

const unsigned int NumMacroColumnsP = 1;
const unsigned int NumMacroRowsP = (fullPopupHeightP - rowHeightP - (2 * popupTopMargin) + fileButtonRowSpacing)/(buttonHeight + fileButtonRowSpacing) - 1;
//const unsigned int NumMacroRowsP = NumMacroRows;
const unsigned int NumDisplayedMacrosP = NumMacroColumnsP * NumMacroRowsP;

const PixelNumber MacroListPopupWidthP = fullPopupWidthP;
const PixelNumber MacroListPopupHeightP = ((NumMacroRowsP + 1) * buttonHeight) + (NumMacroRowsP * fileButtonRowSpacing) + (2 * popupTopMargin);

const unsigned int numMessageRows = (rowTabs - margin - rowHeight)/rowTextHeight;
const PixelNumber messageTextX = margin + messageTimeWidth + 2;
const PixelNumber messageTextWidth = DisplayX - margin - messageTextX;

const unsigned int NumControlPageMacroButtons = 4;
const PixelNumber minControlPageMacroButtonsWidth = (tempButtonWidth * 3)/2;
const PixelNumber maxControlPageMacroButtonsWidth = DisplayX/2 - 2 * margin;

const PixelNumber alertPopupWidth = fullPopupWidth - 6 * margin;
const PixelNumber alertPopupHeight = 2 * popupTopMargin + 6 * rowTextHeight + 2 * buttonHeight + moveButtonRowSpacing;
const PixelNumber alertPopupWidthP = fullPopupWidthP - 2 * margin;
const PixelNumber alertPopupHeightP = 2 * popupTopMargin + 8 * rowTextHeight + 3 * buttonHeight + 2* moveButtonRowSpacing;

const PixelNumber babystepPopupWidth = (2 * fullPopupWidth)/3;
const PixelNumber babystepPopupHeight = 3 * rowHeight + 2 * popupTopMargin;
const PixelNumber babystepRowSpacing = rowHeight;

#endif

#endif /* SRC_USERINTERFACECONSTANTS_HPP_ */
