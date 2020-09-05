/*
 * UserInterface.cpp
 *
 *  Created on: 7 Jan 2017
 *      Author: David
 */

#ifndef OEM_LAYOUT

#include "UserInterface.hpp"

#include "UserInterfaceConstants.hpp"
#include "PanelDue.hpp"
#include "FileManager.hpp"
#include "MessageLog.hpp"
#include "Library/Misc.hpp"
#include "Library/Vector.hpp"
#include "Icons/Icons.hpp"
#include "Hardware/Buzzer.hpp"
#include "Hardware/Reset.hpp"
#include "Hardware/SerialIo.hpp"
#include "Hardware/SysTick.hpp"
#include "Strings.hpp"
#include "Version.hpp"

// Public fields
TextField *fwVersionField, *userCommandField;
IntegerField *freeMem;
StaticTextField *touchCalibInstruction, *debugField;
StaticTextField *messageTextFields[numMessageRows], *messageTimeFields[numMessageRows];

// Private fields
class AlertPopup;
class AlertPopupP;

const size_t machineNameLength = 30;
const size_t printingFileLength = 40;
const size_t zprobeBufLength = 12;
const size_t generatedByTextLength = 50;
const size_t lastModifiedTextLength = 20;
const size_t printTimeTextLength = 12;		// e.g. 11h 55m
const size_t controlPageMacroTextLength = 50;

struct FileListButtons
{
	SingleButton *scrollLeftButton, *scrollRightButton, *folderUpButton;
	IntegerField *errorField;
};

static FileListButtons filesListButtons, macrosListButtons, macrosListButtonsP;
static SingleButton *changeCardButton;

static TextButton *filenameButtons[NumDisplayedFiles];
static TextButton *macroButtons[NumDisplayedMacros];
static TextButton *macroButtonsP[NumDisplayedMacrosP];
static TextButton *controlPageMacroButtons[NumControlPageMacroButtons];
static String<controlPageMacroTextLength> controlPageMacroText[NumControlPageMacroButtons];

static PopupWindow *setTempPopup, *setRPMPopup, *movePopup, *extrudePopup, *fileListPopup, *macrosPopup, *fileDetailPopup, *baudPopup,
		*volumePopup, *infoTimeoutPopup, *areYouSurePopup, *keyboardPopup, *languagePopup, *coloursPopup;
#ifdef SUPPORT_ENCODER
static PopupWindow *setTempPopupEncoder, *macrosPopupP;
#endif
static StaticTextField *areYouSureTextField, *areYouSureQueryField;
static PopupWindow *areYouSurePopupP, *extrudePopupP;
static StaticTextField *areYouSureTextFieldP, *areYouSureQueryFieldP;
static DisplayField *baseRoot, *commonRoot, *controlRoot, *printRoot, *messageRoot, *setupRoot,
		*pendantBaseRoot, *pendantJogRoot, *pendantOffsetRoot, *pendantJobRoot;
static SingleButton *homeAllButton, *bedCompButton;
static IconButtonWithText *homeButtons[MaxDisplayableAxes], *toolButtons[MaxHeaters];
static SingleButton *pHomeAllButton, *pHomeButtons[MaxTotalAxes], *measureZButton;
static FloatField *controlTabAxisPos[MaxDisplayableAxes];
static FloatField *printTabAxisPos[MaxDisplayableAxes];
static FloatField *movePopupAxisPos[MaxDisplayableAxes];
static FloatField *jogTabAxisPos[MaxDisplayableAxesP], *jobTabAxisPos[MaxDisplayableAxesP];
static IconButtonWithText *toolSelectButtons[MaxPendantTools], *toolButtonsPJob[MaxPendantTools];
static FloatField *currentTemps[MaxHeaters];
static FloatField *currentTempPJog;
static FloatField *currentTempsPJob[MaxPendantTools];
static FloatField *fpHeightField, *fpLayerHeightField, *babystepOffsetField;
static IntegerField *fpSizeField, *fpFilamentField, *filePopupTitleField;
static ProgressBar *printProgressBar, *printProgressBarP;
static SingleButton *tabControl, *tabPrint, *tabMsg, *tabSetup, *tabPendant, *tabJog, *tabOffset, *tabJob;
static ButtonBase *pResumeButton, *pPauseButton, *pResetButton;
static StaticTextField *jobTextField;
static IntegerButton *pFeedrateButton, *pExtruderPercentButton, *pSpindleRPMButton;
static ButtonBase *filesButton, *pauseButton, *resumeButton, *resetButton, *babystepButton;
static TextField *timeLeftField, *zProbe;
static TextField *fpNameField, *fpGeneratedByField, *fpLastModifiedField, *fpPrintTimeField;
static StaticTextField *moveAxisRows[MaxDisplayableAxes];
static StaticTextField *nameField, *statusField;
static StaticTextField *pNameField, *pStatusField;
static IntegerButton *activeTemps[MaxHeaters], *standbyTemps[MaxHeaters];
static IntegerButton *activeTempPJog, *standbyTempPJog;
static IntegerButton *activeTempsPJob[MaxPendantTools], *standbyTempsPJob[MaxPendantTools];
static IntegerField *currentToolField;
static IntegerButton *spd, *extrusionFactors[MaxHeaters], *fanSpeed, *baudRateButton, *volumeButton, *infoTimeoutButton;
static TextButton *languageButton, *coloursButton, *dimmingTypeButton;
static SingleButton *moveButton, *extrudeButton, *macroButton;
static PopupWindow *babystepPopup;
static AlertPopup *alertPopup;
static AlertPopupP *alertPopupP;
static CharButtonRow *keyboardRows[4];
static const char* array const * array currentKeyboard;

static ButtonBase * null currentTab = nullptr, *lastRegularTab = nullptr, *lastPendantTab = nullptr;

static ButtonPress currentButton;
static ButtonPress fieldBeingAdjusted;
static ButtonPress currentExtrudeRatePress, currentExtrudeAmountPress;
static ButtonPress currentExtrudeRatePressP, currentExtrudeAmountPressP;

static String<machineNameLength> machineName;
static String<printingFileLength> printingFile;
static String<zprobeBufLength> zprobeBuf;
static String<generatedByTextLength> generatedByText;
static String<lastModifiedTextLength> lastModifiedText;
static String<printTimeTextLength> printTimeText;

const size_t maxUserCommandLength = 40;					// max length of a user gcode command
const size_t numUserCommandBuffers = 6;					// number of command history buffers plus one

static String<maxUserCommandLength> userCommandBuffers[numUserCommandBuffers];
static size_t currentUserCommandBuffer = 0, currentHistoryBuffer = 0;

static unsigned int numToolColsUsed = 0;
static unsigned int numHeaterAndToolColumns = 0;
static int oldIntValue;
HeaterStatus heaterStatus[MaxHeaters];
static Event eventToConfirm = evNull;
static uint8_t numVisibleAxes = 0;						// initialise to 0 so we refresh the macros list when we receive the number of axes
static uint8_t numDisplayedAxes = 0;
static bool isDelta = false;

const char* array null currentFile = nullptr;			// file whose info is displayed in the file info popup
const StringTable * strings = &LanguageTables[0];
static bool keyboardIsDisplayed = false;
static bool keyboardShifted = false;

int32_t alertMode = -1;									// the mode of the current alert, or -1 if no alert displayed
uint32_t alertTicks = 0;
uint32_t infoTimeout = DefaultInfoTimeout;				// info timeout in seconds, 0 means don't display into messages at all
uint32_t whenAlertReceived;
bool displayingResponse = false;						// true if displaying a response

enum Workplaces
{
	G54,
	G55,
	G56,
	G57,
	G58,
	G59,
	G59_1,
	G59_2,
	G59_3,
	MaxTotalWorkplaces
};

struct Axis
{
	float babystep;
	char letter[2];
	float workplaceOffsets[9];
	uint16_t homed : 1,
		visible : 1,
		slot : 6,
		slotP : 6;
};

static Axis CreateAxis(char letter)
{
	return (Axis)
			{
				0.0f,							// babystep
				{letter, '\0'},					// letter
				{0, 0, 0, 0, 0, 0, 0, 0, 0},	// workplaceOffsets
				false,							// homed
				false,							// visible
				MaxTotalAxes,					// slot
				MaxTotalAxes					// slotP
			};
}

static Axis axes[] = {
		CreateAxis('X'),
		CreateAxis('Y'),
		CreateAxis('Z'),
		CreateAxis('U'),
		CreateAxis('V'),
		CreateAxis('W'),
		CreateAxis('A'),
		CreateAxis('B'),
		CreateAxis('C'),
		CreateAxis('D'),
};

static bool allAxesHomed = false;

struct Spindle
{
	// Index within configured spindles
	uint8_t index = 0;
	float active = 0.0;
	float max = 0.0;
	int8_t tool = -1;
	Spindle* next = nullptr;
};

static Spindle* spindles;

struct Tool
{
	// tool number
	uint8_t index = 0;
	int8_t heater = -1;				// only look at the first heater as we only display one
	int8_t extruder = -1;			// only look at the first extruder as we only display one
	Spindle* spindle = nullptr;		// only look at the first spindle as we only display one
	float offsets[MaxTotalAxes];
	ToolStatus status = ToolStatus::off;
	uint8_t slot = MaxHeaters;
	uint8_t slotP = MaxPendantTools;
	Tool* next = nullptr;
};

static Tool* tools;
static int32_t currentTool = -1;

struct BedOrChamber
{
	// Index within configured heaters
	uint8_t index = 0;
	// Id of heater
	int8_t heater = -1;
	// Slot for display on panel
	uint8_t slot = MaxHeaters;
	uint8_t slotP = MaxPendantTools;
} bedHeater, chamberHeater;

#ifdef SUPPORT_ENCODER

# include "Hardware/RotaryEncoder.hpp"

static RotaryEncoder *encoder;
static uint32_t lastEncoderCommandSentAt = 0;
static ButtonPress currentJogAxis, currentJogAmount;
static bool isLandscape = false;

#endif

class StandardPopupWindow : public PopupWindow
{
public:
	StandardPopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder, Colour textColour, Colour imageBackColour,
			const char * null title, PixelNumber topMargin = popupTopMargin);

protected:
	StaticTextField *titleField;
	IconButton *closeButton;
};

class AlertPopup : public StandardPopupWindow
{
public:
	AlertPopup(const ColourScheme& colours);
	void Set(const char *title, const char *text, int32_t mode, uint32_t controls);

private:
	TextButton *okButton, *cancelButton, *zUpCourseButton, *zUpMedButton, *zUpFineButton, *zDownCourseButton, *zDownMedButton, *zDownFineButton;
	String<alertTextLength/3> alertText1, alertText2, alertText3;
	String<alertTitleLength> alertTitle;
};

class AlertPopupP : public StandardPopupWindow
{
public:
	AlertPopupP(const ColourScheme& colours);
	void Set(const char *title, const char *text, int32_t mode, uint32_t controls);

private:
	TextButton *zUpCourseButton, *zUpMedButton, *zUpFineButton, *zDownCourseButton, *zDownMedButton, *zDownFineButton;
	IconButton *okButton, *cancelButton;
	String<145/5> alertText1, alertText2, alertText3, alertText4, alertText5;
	String<alertTitleLength> alertTitle;
};

// Create a standard popup window with a title and a close button at the top right
StandardPopupWindow::StandardPopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder, Colour textColour, Colour imageBackColour, const char * null title, PixelNumber topMargin)
	: PopupWindow(ph, pw, pb, pBorder), titleField(nullptr)
{
	DisplayField::SetDefaultColours(textColour, pb);
	if (title != nullptr)
	{
		AddField(titleField = new StaticTextField(topMargin + labelRowAdjust, popupSideMargin + closeButtonWidth + popupFieldSpacing,
							pw - 2 * (popupSideMargin + closeButtonWidth + popupFieldSpacing), TextAlignment::Centre, title));
	}
	else
	{
		titleField = nullptr;
	}
	DisplayField::SetDefaultColours(textColour, imageBackColour);
	AddField(closeButton = new IconButton(popupTopMargin, pw - (closeButtonWidth + popupSideMargin), closeButtonWidth, IconCancel, evCancel));
}

AlertPopup::AlertPopup(const ColourScheme& colours)
	: StandardPopupWindow(alertPopupHeight, alertPopupWidth,
			colours.alertPopupBackColour, colours.popupBorderColour, colours.alertPopupTextColour, colours.buttonImageBackColour, "", popupTopMargin)		// title is present, but empty for now
{
	DisplayField::SetDefaultColours(colours.alertPopupTextColour, colours.alertPopupBackColour);
	titleField->SetValue(alertTitle.c_str());
	AddField(new StaticTextField(popupTopMargin + 2 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, alertText1.c_str()));
	AddField(new StaticTextField(popupTopMargin + 3 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, alertText2.c_str()));
	AddField(new StaticTextField(popupTopMargin + 4 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, alertText3.c_str()));

	// Calculate the button positions
	constexpr unsigned int numButtons = 6;
	constexpr PixelNumber buttonWidthUnits = 5;
	constexpr PixelNumber buttonSpacingUnits = 1;
	constexpr PixelNumber totalUnits = (numButtons * buttonWidthUnits) + ((numButtons - 1) * buttonSpacingUnits);
	constexpr PixelNumber unitWidth = (alertPopupWidth - 2 * popupSideMargin)/totalUnits;
	constexpr PixelNumber buttonWidth = buttonWidthUnits * unitWidth;
	constexpr PixelNumber buttonStep = (buttonWidthUnits + buttonSpacingUnits) * unitWidth;
	constexpr PixelNumber hOffset = popupSideMargin + (alertPopupWidth - 2 * popupSideMargin - totalUnits * unitWidth)/2;

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	AddField(zUpCourseButton =   new TextButton(popupTopMargin + 6 * rowTextHeight, hOffset,				  buttonWidth, LESS_ARROW "2.0", evMoveZ, "-2.0"));
	AddField(zUpMedButton =      new TextButton(popupTopMargin + 6 * rowTextHeight, hOffset + buttonStep,     buttonWidth, LESS_ARROW "0.2", evMoveZ, "-0.2"));
	AddField(zUpFineButton =     new TextButton(popupTopMargin + 6 * rowTextHeight, hOffset + 2 * buttonStep, buttonWidth, LESS_ARROW "0.02", evMoveZ, "-0.02"));
	AddField(zDownFineButton =   new TextButton(popupTopMargin + 6 * rowTextHeight, hOffset + 3 * buttonStep, buttonWidth, MORE_ARROW "0.02", evMoveZ, "0.02"));
	AddField(zDownMedButton =    new TextButton(popupTopMargin + 6 * rowTextHeight, hOffset + 4 * buttonStep, buttonWidth, MORE_ARROW "0.2", evMoveZ, "0.2"));
	AddField(zDownCourseButton = new TextButton(popupTopMargin + 6 * rowTextHeight, hOffset + 5 * buttonStep, buttonWidth, MORE_ARROW "2.0", evMoveZ, "2.0"));

	AddField(okButton =          new TextButton(popupTopMargin + 6 * rowTextHeight + buttonHeight + moveButtonRowSpacing, hOffset + buttonStep,     buttonWidth + buttonStep, "OK", evCloseAlert, "M292 P0"));
	AddField(cancelButton =      new TextButton(popupTopMargin + 6 * rowTextHeight + buttonHeight + moveButtonRowSpacing, hOffset + 3 * buttonStep, buttonWidth + buttonStep, "Cancel", evCloseAlert, "M292 P1"));
}

void AlertPopup::Set(const char *title, const char *text, int32_t mode, uint32_t controls)
{
	alertTitle.copy(title);

	// Split the alert text into 3 lines
	size_t splitPoint = MessageLog::FindSplitPoint(text, alertText1.capacity(), (PixelNumber)(GetWidth() - 2 * popupSideMargin));
	alertText1.copy(text);
	alertText1.truncate(splitPoint);
	text += splitPoint;
	splitPoint = MessageLog::FindSplitPoint(text, alertText2.capacity(), GetWidth() - 2 * popupSideMargin);
	alertText2.copy(text);
	alertText2.truncate(splitPoint);
	text += splitPoint;
	alertText3.copy(text);

	closeButton->Show(mode == 1);
	okButton->Show(mode >= 2);
	cancelButton->Show(mode == 3);
	const bool showZbuttons = (controls & (1u << 2)) != 0;
	zUpCourseButton->Show(showZbuttons);
	zUpMedButton->Show(showZbuttons);
	zUpFineButton->Show(showZbuttons);
	zDownCourseButton->Show(showZbuttons);
	zDownMedButton->Show(showZbuttons);
	zDownFineButton->Show(showZbuttons);
}
AlertPopupP::AlertPopupP(const ColourScheme& colours)
	: StandardPopupWindow(alertPopupHeightP, alertPopupWidthP,
			colours.alertPopupBackColour, colours.popupBorderColour, colours.alertPopupTextColour, colours.buttonImageBackColour, "", popupTopMargin)		// title is present, but empty for now
{
	DisplayField::SetDefaultColours(colours.alertPopupTextColour, colours.alertPopupBackColour);
	titleField->SetValue(alertTitle.c_str());
	AddField(new StaticTextField(popupTopMargin + 2 * rowTextHeight, popupSideMargin/2, GetWidth() - popupSideMargin, TextAlignment::Centre, alertText1.c_str()));
	AddField(new StaticTextField(popupTopMargin + 3 * rowTextHeight, popupSideMargin/2, GetWidth() - popupSideMargin, TextAlignment::Centre, alertText2.c_str()));
	AddField(new StaticTextField(popupTopMargin + 4 * rowTextHeight, popupSideMargin/2, GetWidth() - popupSideMargin, TextAlignment::Centre, alertText3.c_str()));
	AddField(new StaticTextField(popupTopMargin + 5 * rowTextHeight, popupSideMargin/2, GetWidth() - popupSideMargin, TextAlignment::Centre, alertText4.c_str()));
	AddField(new StaticTextField(popupTopMargin + 6 * rowTextHeight, popupSideMargin/2, GetWidth() - popupSideMargin, TextAlignment::Centre, alertText5.c_str()));

	// Calculate the button positions
	constexpr unsigned int numButtons = 3;
	constexpr PixelNumber buttonWidthUnits = 6;
	constexpr PixelNumber buttonSpacingUnits = 1;
	constexpr PixelNumber totalUnits = (numButtons * buttonWidthUnits) + ((numButtons - 1) * buttonSpacingUnits);
	constexpr PixelNumber unitWidth = (alertPopupWidthP - popupSideMargin)/totalUnits;
	constexpr PixelNumber buttonWidth = buttonWidthUnits * unitWidth;
	constexpr PixelNumber buttonStep = (buttonWidthUnits + buttonSpacingUnits) * unitWidth;
	constexpr PixelNumber hOffset = popupSideMargin/2 + (alertPopupWidthP - popupSideMargin - totalUnits * unitWidth)/2;

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	AddField(zUpCourseButton =   new TextButton(popupTopMargin + 8 * rowTextHeight, hOffset + 2 * buttonStep, buttonWidth, LESS_ARROW "2.0", evMoveZ, "-2.0"));
	AddField(zUpMedButton =      new TextButton(popupTopMargin + 8 * rowTextHeight, hOffset + 1 * buttonStep, buttonWidth, LESS_ARROW "0.2", evMoveZ, "-0.2"));
	AddField(zUpFineButton =     new TextButton(popupTopMargin + 8 * rowTextHeight, hOffset + 0 * buttonStep, buttonWidth, LESS_ARROW "0.02", evMoveZ, "-0.02"));
	AddField(zDownFineButton =   new TextButton(popupTopMargin + 8 * rowTextHeight + buttonHeight + 2 * margin, hOffset + 0 * buttonStep, buttonWidth, MORE_ARROW "0.02", evMoveZ, "0.02"));
	AddField(zDownMedButton =    new TextButton(popupTopMargin + 8 * rowTextHeight + buttonHeight + 2 * margin, hOffset + 1 * buttonStep, buttonWidth, MORE_ARROW "0.2", evMoveZ, "0.2"));
	AddField(zDownCourseButton = new TextButton(popupTopMargin + 8 * rowTextHeight + buttonHeight + 2 * margin, hOffset + 2 * buttonStep, buttonWidth, MORE_ARROW "2.0", evMoveZ, "2.0"));

	AddField(okButton =          new IconButton(popupTopMargin + 8 * rowTextHeight + 2 * buttonHeight + moveButtonRowSpacing + 2 * margin, hOffset + buttonStep/4,	   buttonWidth + buttonStep/4, IconOk, evCloseAlert, "M292 P0"));
	AddField(cancelButton =      new IconButton(popupTopMargin + 8 * rowTextHeight + 2 * buttonHeight + moveButtonRowSpacing + 2 * margin, hOffset + 3 * buttonStep/2, buttonWidth + buttonStep/4, IconCancel, evCloseAlert, "M292 P1"));
}

void AlertPopupP::Set(const char *title, const char *text, int32_t mode, uint32_t controls)
{
	alertTitle.copy(title);

	// Split the alert text into 3 lines
	size_t splitPoint = MessageLog::FindSplitPoint(text, alertText1.capacity(), (PixelNumber)(GetWidth() - 2 * popupSideMargin));
	alertText1.copy(text);
	alertText1.truncate(splitPoint);
	text += splitPoint;
	splitPoint = MessageLog::FindSplitPoint(text, alertText2.capacity(), GetWidth() - 2 * popupSideMargin);
	alertText2.copy(text);
	alertText2.truncate(splitPoint);
	text += splitPoint;
	splitPoint = MessageLog::FindSplitPoint(text, alertText3.capacity(), GetWidth() - 2 * popupSideMargin);
	alertText3.copy(text);
	alertText3.truncate(splitPoint);
	text += splitPoint;
	splitPoint = MessageLog::FindSplitPoint(text, alertText4.capacity(), GetWidth() - 2 * popupSideMargin);
	alertText4.copy(text);
	alertText4.truncate(splitPoint);
	text += splitPoint;
	alertText5.copy(text);

	closeButton->Show(mode == 1);
	okButton->Show(mode >= 2);
	cancelButton->Show(mode == 3);
	const bool showZbuttons = (controls & (1u << 2)) != 0;
	zUpCourseButton->Show(showZbuttons);
	zUpMedButton->Show(showZbuttons);
	zUpFineButton->Show(showZbuttons);
	zDownCourseButton->Show(showZbuttons);
	zDownMedButton->Show(showZbuttons);
	zDownFineButton->Show(showZbuttons);
}

inline PixelNumber CalcWidth(unsigned int numCols, PixelNumber displayWidth = DisplayX)
{
	return (displayWidth - 2 * margin + fieldSpacing)/numCols - fieldSpacing;
}

inline PixelNumber CalcXPos(unsigned int col, PixelNumber width, int offset = 0)
{
	return col * (width + fieldSpacing) + margin + offset;
}

// Add a text button with a string parameter
TextButton *AddTextButton(PixelNumber row, unsigned int col, unsigned int numCols, const char* array text, Event evt, const char* param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	TextButton *f = new TextButton(row - 2, xpos, width, text, evt, param);
	mgr.AddField(f);
	return f;
}

// Add a text button with an int parameter
TextButton *AddTextButton(PixelNumber row, unsigned int col, unsigned int numCols, const char* array text, Event evt, int param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	TextButton *f = new TextButton(row - 2, xpos, width, text, evt, param);
	mgr.AddField(f);
	return f;
}

// Add an integer button
IntegerButton *AddIntegerButton(PixelNumber row, unsigned int col, unsigned int numCols, const char * array null label, const char * array null units, Event evt, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IntegerButton *f = new IntegerButton(row - 2, xpos, width, label, units);
	f->SetEvent(evt, 0);
	mgr.AddField(f);
	return f;
}

// Add an icon button with a string parameter
IconButton *AddIconButton(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, const char* param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButton *f = new IconButton(row - 2, xpos, width, icon, evt, param);
	mgr.AddField(f);
	return f;
}

// Add an icon button with an int parameter
IconButton *AddIconButton(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, int param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButton *f = new IconButton(row - 2, xpos, width, icon, evt, param);
	mgr.AddField(f);
	return f;
}

// Add an icon button with a string parameter
IconButtonWithText *AddIconButtonWithText(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, const char * text, const char* param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButtonWithText *f = new IconButtonWithText(row - 2, xpos, width, icon, evt, text, param);
	mgr.AddField(f);
	return f;
}

// Add an icon button with an int parameter
IconButtonWithText *AddIconButtonWithText(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, int intVal, const int param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButtonWithText *f = new IconButtonWithText(row - 2, xpos, width, icon, evt, intVal, param);
	mgr.AddField(f);
	return f;
}

// Add an icon button with an int parameter
IconButtonWithText *AddIconButtonWithText(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, const char * text, const int param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButtonWithText *f = new IconButtonWithText(row - 2, xpos, width, icon, evt, text, param);
	mgr.AddField(f);
	return f;
}

// Create a row of text buttons.
// Optionally, set one to 'pressed' and return that one.
// Set the colours before calling this
ButtonPress CreateStringButtonRow(Window * pf, PixelNumber top, PixelNumber left, PixelNumber totalWidth, PixelNumber spacing, unsigned int numButtons,
									const char* array const text[], const char* array const params[], Event evt, int selected = -1)
{
	const PixelNumber step = (totalWidth + spacing)/numButtons;
	ButtonPress bp;
	for (unsigned int i = 0; i < numButtons; ++i)
	{
		TextButton *tp = new TextButton(top, left + i * step, step - spacing, text[i], evt, params[i]);
		pf->AddField(tp);
		if ((int)i == selected)
		{
			tp->Press(true, 0);
			bp = ButtonPress(tp, 0);
		}
	}
	return bp;
}

// Create a row of text buttons.
// Optionally, set one to 'pressed' and return that one.
// Set the colours before calling this
ButtonPress CreateStringButtonRowVertical(Window * pf, PixelNumber top, PixelNumber left, PixelNumber totalHeight, PixelNumber spacing, PixelNumber buttonWidth, unsigned int numButtons,
									const char* array const text[], const char* array const params[], Event evt, int selected = -1)
{
	const PixelNumber step = (totalHeight + spacing)/numButtons;
	ButtonPress bp;
	for (unsigned int i = 0; i < numButtons; ++i)
	{
		TextButton *tp = new TextButton(top + i * step, left, buttonWidth, text[i], evt, params[i]);
		pf->AddField(tp);
		if ((int)i == selected)
		{
			tp->Press(true, 0);
			bp = ButtonPress(tp, 0);
		}
	}
	return bp;
}

// Create a row of text buttons.
// Optionally, set one to 'pressed' and return that one.
// Set the colours before calling this
ButtonPress CreateFloatButtonRowVertical(Window * pf, PixelNumber top, PixelNumber left, PixelNumber totalHeight, PixelNumber spacing, PixelNumber buttonWidth, unsigned int numButtons,
									const char* unit, const unsigned short int decimals, const float params[], Event evt, int selected = -1)
{
	const PixelNumber step = (totalHeight + spacing)/numButtons;
	ButtonPress bp;
	for (unsigned int i = 0; i < numButtons; ++i)
	{
		FloatButton *tp = new FloatButton(top + i * step, left, buttonWidth, decimals, unit);
		tp->SetEvent(evt, params[i]);
		tp->SetValue(params[i]);
		pf->AddField(tp);
		if ((int)i == selected)
		{
			tp->Press(true, 0);
			bp = ButtonPress(tp, 0);
		}
	}
	return bp;
}

#if 0	// currently unused
// Create a row of icon buttons.
// Set the colours before calling this
void CreateIconButtonRow(Window * pf, PixelNumber top, PixelNumber left, PixelNumber totalWidth, PixelNumber spacing, unsigned int numButtons,
									const Icon icons[], const char* array const params[], Event evt)
{
	const PixelNumber step = (totalWidth + spacing)/numButtons;
	for (unsigned int i = 0; i < numButtons; ++i)
	{
		pf->AddField(new IconButton(top, left + i * step, step - spacing, icons[i], evt, params[i]));
	}
}
#endif

// Create a popup bar with string parameters
PopupWindow *CreateStringPopupBar(const ColourScheme& colours, PixelNumber width, unsigned int numEntries, const char* const text[], const char* const params[], Event ev)
{
	PopupWindow *pf = new PopupWindow(popupBarHeight, width, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	PixelNumber step = (width - 2 * popupSideMargin + popupFieldSpacing)/numEntries;
	for (unsigned int i = 0; i < numEntries; ++i)
	{
		pf->AddField(new TextButton(popupTopMargin, popupSideMargin + i * step, step - popupFieldSpacing, text[i], ev, params[i]));
	}
	return pf;
}

// Create a popup bar with integer parameters
// If the 'params' parameter is null then we use 0, 1, 2.. at the parameters
PopupWindow *CreateIntPopupBar(const ColourScheme& colours, PixelNumber width, unsigned int numEntries, const char* const text[], const int * null params, Event ev, Event zeroEv)
{
	PopupWindow *pf = new PopupWindow(popupBarHeight, width, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	PixelNumber step = (width - 2 * popupSideMargin + popupFieldSpacing)/numEntries;
	for (unsigned int i = 0; i < numEntries; ++i)
	{
		const int iParam = (params == nullptr) ? (int)i : params[i];
		pf->AddField(new TextButton(popupSideMargin, popupSideMargin + i * step, step - popupFieldSpacing, text[i], (params[i] == 0) ? zeroEv : ev, iParam));
	}
	return pf;
}

// Nasty hack to work around bug in RepRapFirmware 1.09k and earlier
// The M23 and M30 commands don't work if we send the full path, because "0:/gcodes/" gets prepended regardless.
const char * array StripPrefix(const char * array dir)
{
	if ((GetFirmwareFeatures() && noGcodesFolder) == 0)			// if running RepRapFirmware
	{
		const size_t len = strlen(dir);
		if (len >= 8 && memcmp(dir, "/gcodes/", 8) == 0)
		{
			dir += 8;
		}
		else if (len >= 10 && memcmp(dir, "0:/gcodes/", 10) == 0)
		{
			dir += 10;
		}
		else if (strcmp(dir, "/gcodes") == 0 || strcmp(dir, "0:/gcodes") == 0)
		{
			dir += len;
		}
	}
	return dir;
}

// Adjust the brightness
void ChangeBrightness(bool up)
{
	int adjust = max<int>(1, GetBrightness()/5);
	if (!up)
	{
		adjust = -adjust;
	}
	SetBrightness(GetBrightness() + adjust);
}

// Cycle through available display dimmer types
void ChangeDisplayDimmerType()
{
	DisplayDimmerType newType = (DisplayDimmerType) ((uint8_t)GetDisplayDimmerType() + 1);
	if (newType == DisplayDimmerType::NumTypes)
	{
		newType = (DisplayDimmerType)0;
	}
	SetDisplayDimmerType(newType);
}

// Update an integer field, provided it isn't the one being adjusted
// Don't update it if the value hasn't changed, because that makes the display flicker unnecessarily
void UpdateField(IntegerButton *f, int val)
{
	if (f != fieldBeingAdjusted.GetButton())
	{
		f->SetValue(val);
	}
}

void PopupAreYouSure(Event ev, const char* text, const char* query = strings->areYouSure)
{
	eventToConfirm = ev;
	if (isLandscape)
	{
		areYouSureTextField->SetValue(text);
		areYouSureQueryField->SetValue(query);
		mgr.SetPopup(areYouSurePopup, AutoPlace, AutoPlace);
	}
	else {
		areYouSureTextFieldP->SetValue(text);
		areYouSureQueryFieldP->SetValue(query);
		mgr.SetPopupP(areYouSurePopupP, AutoPlace, AutoPlace);
	}
}

void CreateIntegerAdjustPopup(const ColourScheme& colours)
{
	// Create the popup window used to adjust temperatures, fan speed, extrusion factor etc.
	static const char* const tempPopupText[] = {"-5", "-1", strings->set, "+1", "+5"};
	static const int tempPopupParams[] = { -5, -1, 0, 1, 5 };
	setTempPopup = CreateIntPopupBar(colours, tempPopupBarWidth, 5, tempPopupText, tempPopupParams, evAdjustInt, evSetInt);
}

void CreateIntegerRPMAdjustPopup(const ColourScheme& colours)
{
	// Create the popup window used to adjust temperatures, fan speed, extrusion factor etc.
	static const char* const rpmPopupText[] = {"-1000", "-100", "-10", strings->set, "+10", "+100", "+1000"};
	static const int rpmPopupParams[] = { -1000, -100, -10, 0, 10, 100, 1000 };
	setRPMPopup = CreateIntPopupBar(colours, rpmPopupBarWidth, 7, rpmPopupText, rpmPopupParams, evAdjustInt, evSetInt);
}

#ifdef SUPPORT_ENCODER
void CreateIntegerAdjustWithEncoderPopup(const ColourScheme& colours)
{
	// Create the popup window used to adjust temperatures, fan speed, extrusion factor etc.
	static const char* const tempPopupText[] = {strings->set};
	static const int tempPopupParams[] = { 0 };
	setTempPopupEncoder = CreateIntPopupBar(colours, tempPopupBarWidthEncoder, 1, tempPopupText, tempPopupParams, evAdjustInt, evSetInt);
}
#endif

// Create the movement popup window
void CreateMovePopup(const ColourScheme& colours)
{
	static const char * array const xyJogValues[] = { "-100", "-10", "-1", "-0.1", "0.1",  "1", "10", "100" };
	static const char * array const zJogValues[] = { "-50", "-5", "-0.5", "-0.05", "0.05",  "0.5", "5", "50" };

	movePopup = new StandardPopupWindow(movePopupHeight, movePopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, strings->moveHead);
	PixelNumber ypos = popupTopMargin + buttonHeight + moveButtonRowSpacing;
	const PixelNumber axisPosYpos = ypos + (MaxDisplayableAxes - 1) * (buttonHeight + moveButtonRowSpacing);
	const PixelNumber xpos = popupSideMargin + axisLabelWidth;
	Event e = evMoveX;
	PixelNumber column = popupSideMargin + margin;
	PixelNumber xyFieldWidth = (DISPLAY_X - (2 * margin) - (MaxDisplayableAxes * fieldSpacing))/(MaxDisplayableAxes + 1);

	for (size_t i = 0; i < MaxDisplayableAxes; ++i)
	{
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		const char * array const * array values = (axisNames[i][0] == 'Z') ? zJogValues : xyJogValues;
		CreateStringButtonRow(movePopup, ypos, xpos, movePopupWidth - xpos - popupSideMargin, fieldSpacing, 8, values, values, e);

		// We create the label after the button row, so that the buttons follow it in the field order, which makes it easier to hide them
		DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
		StaticTextField * const tf = new StaticTextField(ypos + labelRowAdjust, popupSideMargin, axisLabelWidth, TextAlignment::Left, axisNames[i]);
		movePopup->AddField(tf);
		moveAxisRows[i] = tf;
		UI::ShowAxis(i, i < MIN_AXES);

		DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupInfoBackColour);
		FloatField *f = new FloatField(axisPosYpos, column, xyFieldWidth, TextAlignment::Left, (i == 2) ? 2 : 1, axisNames[i]);
		movePopupAxisPos[i] = f;
		f->SetValue(0.0);
		movePopup->AddField(f);
		f->Show(i < MIN_AXES);
		column += xyFieldWidth + fieldSpacing;

		ypos += buttonHeight + moveButtonRowSpacing;
		e = (Event)((uint8_t)e + 1);
	}
}

// Create the extrusion controls popup
void CreateExtrudePopup(const ColourScheme& colours)
{
	static const char * array extrudeAmountValues[] = { "100", "50", "20", "10", "5",  "1" };
	static const char * array extrudeSpeedValues[] = { "50", "20", "10", "5", "2" };
	static const char * array extrudeSpeedParams[] = { "3000", "1200", "600", "300", "120" };		// must be extrudeSpeedValues * 60

	extrudePopup = new StandardPopupWindow(extrudePopupHeight, extrudePopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, strings->extrusionAmount);
	PixelNumber ypos = popupTopMargin + buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	currentExtrudeAmountPress = CreateStringButtonRow(extrudePopup, ypos, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, fieldSpacing, 6, extrudeAmountValues, extrudeAmountValues, evExtrudeAmount, 3);
	ypos += buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	extrudePopup->AddField(new StaticTextField(ypos + labelRowAdjust, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, TextAlignment::Centre, strings->extrusionSpeed));
	ypos += buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	currentExtrudeRatePress = CreateStringButtonRow(extrudePopup, ypos, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, fieldSpacing, 5, extrudeSpeedValues, extrudeSpeedParams, evExtrudeRate, 4);
	ypos += buttonHeight + extrudeButtonRowSpacing;
	extrudePopup->AddField(new TextButton(ypos, popupSideMargin, extrudePopupWidth/3 - 2 * popupSideMargin, strings->extrude, evExtrude));
	extrudePopup->AddField(new TextButton(ypos, (2 * extrudePopupWidth)/3 + popupSideMargin, extrudePopupWidth/3 - 2 * popupSideMargin, strings->retract, evRetract));
}

// Create the extrusion controls popup
void CreateExtrudePopupP(const ColourScheme& colours)
{
	static const char * array extrudeAmountValues[] = { "100", "50", "20", "10", "5",  "1" };
	static const char * array extrudeSpeedValues[] = { "50", "20", "10", "5", "2", "1" };
	static const char * array extrudeSpeedParams[] = { "3000", "1200", "600", "300", "120", "60" };		// must be extrudeSpeedValues * 60

	extrudePopupP = new StandardPopupWindow(extrudePopupHeightP, extrudePopupWidthP, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, strings->extrusion);
	const PixelNumber colWidth = CalcWidth(3, extrudePopupWidthP - 2 * popupSideMargin);
	PixelNumber ypos = popupTopMargin + buttonHeight + extrudeButtonRowSpacing;

	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	extrudePopupP->AddField(new StaticTextField(ypos + labelRowAdjust, CalcXPos(0, colWidth, popupSideMargin), colWidth, TextAlignment::Centre, "Amount"));
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	extrudePopupP->AddField(new StaticTextField(ypos + labelRowAdjust, CalcXPos(1, colWidth, popupSideMargin), colWidth, TextAlignment::Centre, "Speed"));

	ypos += buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	currentExtrudeAmountPressP = CreateStringButtonRowVertical(
			extrudePopupP,
			ypos,
			CalcXPos(0, colWidth, popupSideMargin),
			ARRAY_SIZE(extrudeAmountValues) * buttonHeight + (ARRAY_SIZE(extrudeAmountValues) - 1) * fieldSpacing,
			fieldSpacing,
			colWidth,
			ARRAY_SIZE(extrudeAmountValues),
			extrudeAmountValues,
			extrudeAmountValues,
			evExtrudeAmountP,
			3);
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	currentExtrudeRatePressP = CreateStringButtonRowVertical(
			extrudePopupP,
			ypos,
			CalcXPos(1, colWidth, popupSideMargin),
			ARRAY_SIZE(extrudeSpeedValues) * buttonHeight + (ARRAY_SIZE(extrudeSpeedValues) - 1) * fieldSpacing,
			fieldSpacing,
			colWidth,
			ARRAY_SIZE(extrudeSpeedValues),
			extrudeSpeedValues,
			extrudeSpeedParams,
			evExtrudeRateP,
			5);
	ypos += 2 * buttonHeight + extrudeButtonRowSpacing;
	extrudePopupP->AddField(new TextButton(ypos, CalcXPos(2, colWidth, popupSideMargin), colWidth, strings->extrude, evExtrude));
	ypos += buttonHeight + extrudeButtonRowSpacing;
	extrudePopupP->AddField(new TextButton(ypos, CalcXPos(2, colWidth, popupSideMargin), colWidth, strings->retract, evRetract));
}


// Create a popup used to list files pr macros
PopupWindow *CreateFileListPopup(FileListButtons& controlButtons, TextButton ** array fileButtons, unsigned int numRows, unsigned int numCols, const ColourScheme& colours, bool filesNotMacros,
		PixelNumber popupHeight = fileListPopupHeight, PixelNumber popupWidth = fileListPopupWidth)
pre(fileButtons.lim == numRows * numCols)
{
	PopupWindow * const popup = new StandardPopupWindow(popupHeight, popupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, nullptr);
	const PixelNumber closeButtonPos = popupWidth - closeButtonWidth - popupSideMargin;
	const PixelNumber navButtonWidth = (closeButtonPos - popupSideMargin)/7;
	const PixelNumber upButtonPos = closeButtonPos - navButtonWidth - fieldSpacing;
	const PixelNumber rightButtonPos = upButtonPos - navButtonWidth - fieldSpacing;
	const PixelNumber leftButtonPos = rightButtonPos - navButtonWidth - fieldSpacing;
	const PixelNumber textPos = popupSideMargin + navButtonWidth;
	const PixelNumber changeButtonPos = popupSideMargin;

	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	if (filesNotMacros)
	{
		popup->AddField(filePopupTitleField = new IntegerField(popupTopMargin + labelRowAdjust, textPos, leftButtonPos - textPos, TextAlignment::Centre, strings->filesOnCard, nullptr));
	}
	else
	{
		popup->AddField(new StaticTextField(popupTopMargin + labelRowAdjust, textPos, leftButtonPos - textPos, TextAlignment::Centre, strings->macros));
	}

	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.buttonImageBackColour);
	if (filesNotMacros)
	{
		popup->AddField(changeCardButton = new IconButton(popupTopMargin, changeButtonPos, navButtonWidth, IconFiles, evChangeCard, 0));
	}

	const Event scrollEvent = (filesNotMacros) ? evScrollFiles : evScrollMacros;

	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	popup->AddField(controlButtons.scrollLeftButton = new TextButton(popupTopMargin, leftButtonPos, navButtonWidth, LEFT_ARROW, scrollEvent, -1));
	controlButtons.scrollLeftButton->Show(false);
	popup->AddField(controlButtons.scrollRightButton = new TextButton(popupTopMargin, rightButtonPos, navButtonWidth, RIGHT_ARROW, scrollEvent, 1));
	controlButtons.scrollRightButton->Show(false);
	popup->AddField(controlButtons.folderUpButton = new TextButton(popupTopMargin, upButtonPos, navButtonWidth, UP_ARROW, (filesNotMacros) ? evFilesUp : evMacrosUp));
	controlButtons.folderUpButton->Show(false);

	const PixelNumber fileFieldWidth = (popupWidth + fieldSpacing - (2 * popupSideMargin))/numCols;
	for (unsigned int c = 0; c < numCols; ++c)
	{
		PixelNumber row = popupTopMargin;
		for (unsigned int r = 0; r < numRows; ++r)
		{
			row += buttonHeight + fileButtonRowSpacing;
			TextButton *t = new TextButton(row, (fileFieldWidth * c) + popupSideMargin, fileFieldWidth - fieldSpacing, nullptr, evNull);
			t->Show(false);
			popup->AddField(t);
			*fileButtons = t;
			++fileButtons;
		}
	}

	controlButtons.errorField = new IntegerField(popupTopMargin + 2 * (buttonHeight + fileButtonRowSpacing), popupSideMargin, popupWidth - (2 * popupSideMargin),
							TextAlignment::Centre, strings->error, strings->accessingSdCard);
	controlButtons.errorField->Show(false);
	popup->AddField(controlButtons.errorField);
	return popup;
}

// Create the popup window used to display the file dialog
void CreateFileActionPopup(const ColourScheme& colours)
{
	fileDetailPopup = new StandardPopupWindow(fileInfoPopupHeight, fileInfoPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, "File information");
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	PixelNumber ypos = popupTopMargin + (3 * rowTextHeight)/2;
	fpNameField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->fileName);
	ypos += rowTextHeight;
	fpSizeField = new IntegerField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->fileSize, " b");
	ypos += rowTextHeight;
	fpLayerHeightField = new FloatField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, 2, strings->layerHeight, "mm");
	ypos += rowTextHeight;
	fpHeightField = new FloatField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, 1, strings->objectHeight, "mm");
	ypos += rowTextHeight;
	fpFilamentField = new IntegerField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->filamentNeeded, "mm");
	ypos += rowTextHeight;
	fpGeneratedByField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->generatedBy, generatedByText.c_str());
	ypos += rowTextHeight;
	fpLastModifiedField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->lastModified, lastModifiedText.c_str());
	ypos += rowTextHeight;
	fpPrintTimeField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->estimatedPrintTime, printTimeText.c_str());

	fileDetailPopup->AddField(fpNameField);
	fileDetailPopup->AddField(fpSizeField);
	fileDetailPopup->AddField(fpLayerHeightField);
	fileDetailPopup->AddField(fpHeightField);
	fileDetailPopup->AddField(fpFilamentField);
	fileDetailPopup->AddField(fpGeneratedByField);
	fileDetailPopup->AddField(fpLastModifiedField);
	fileDetailPopup->AddField(fpPrintTimeField);

	// Add the buttons
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	fileDetailPopup->AddField(new TextButton(popupTopMargin + 10 * rowTextHeight, popupSideMargin, fileInfoPopupWidth/3 - 2 * popupSideMargin, strings->print, evPrintFile));
	fileDetailPopup->AddField(new TextButton(popupTopMargin + 10 * rowTextHeight, fileInfoPopupWidth/3 + popupSideMargin, fileInfoPopupWidth/3 - 2 * popupSideMargin, strings->simulate, evSimulateFile));
	fileDetailPopup->AddField(new IconButton(popupTopMargin + 10 * rowTextHeight, (2 * fileInfoPopupWidth)/3 + popupSideMargin, fileInfoPopupWidth/3 - 2 * popupSideMargin, IconTrash, evDeleteFile));
}

// Create the "Are you sure?" popup
void CreateAreYouSurePopup(const ColourScheme& colours)
{
	areYouSurePopup = new PopupWindow(areYouSurePopupHeight, areYouSurePopupWidth, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	areYouSurePopup->AddField(areYouSureTextField = new StaticTextField(popupSideMargin, margin, areYouSurePopupWidth - 2 * margin, TextAlignment::Centre, nullptr));
	areYouSurePopup->AddField(areYouSureQueryField = new StaticTextField(popupTopMargin + rowHeight, margin, areYouSurePopupWidth - 2 * margin, TextAlignment::Centre, nullptr));

	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	areYouSurePopup->AddField(new IconButton(popupTopMargin + 2 * rowHeight, popupSideMargin, areYouSurePopupWidth/2 - 2 * popupSideMargin, IconOk, evYes));
	areYouSurePopup->AddField(new IconButton(popupTopMargin + 2 * rowHeight, areYouSurePopupWidth/2 + 10, areYouSurePopupWidth/2 - 2 * popupSideMargin, IconCancel, evCancel));
}

// Create the "Are you sure?" popup for portrait orienttion
void CreateAreYouSurePopupPortrait(const ColourScheme& colours)
{
	areYouSurePopupP = new PopupWindow(areYouSurePopupHeightP, areYouSurePopupWidthP, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	areYouSurePopupP->AddField(areYouSureTextFieldP = new StaticTextField(popupSideMargin, margin, areYouSurePopupWidthP - 2 * margin, TextAlignment::Centre, nullptr));
	areYouSurePopupP->AddField(areYouSureQueryFieldP = new StaticTextField(popupTopMargin + rowHeight, margin, areYouSurePopupWidthP - 2 * margin, TextAlignment::Centre, nullptr));

	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	areYouSurePopupP->AddField(new IconButton(popupTopMargin + 2 * rowHeight, popupSideMargin, areYouSurePopupWidthP/2 - 2 * popupSideMargin, IconOk, evYes));
	areYouSurePopupP->AddField(new IconButton(popupTopMargin + 2 * rowHeight, areYouSurePopupWidthP/2 + 10, areYouSurePopupWidthP/2 - 2 * popupSideMargin, IconCancel, evCancel));
}


// Create the baud rate adjustment popup
void CreateBaudRatePopup(const ColourScheme& colours)
{
	static const char* const baudPopupText[] = { "9600", "19200", "38400", "57600", "115200" };
	static const int baudPopupParams[] = { 9600, 19200, 38400, 57600, 115200 };
	baudPopup = CreateIntPopupBar(colours, fullPopupWidth, 5, baudPopupText, baudPopupParams, evAdjustBaudRate, evAdjustBaudRate);
}

// Create the volume adjustment popup
void CreateVolumePopup(const ColourScheme& colours)
{
	static_assert(Buzzer::MaxVolume == 5, "MaxVolume assumed to be 5 here");
	static const char* const volumePopupText[Buzzer::MaxVolume + 1] = { "0", "1", "2", "3", "4", "5" };
	volumePopup = CreateIntPopupBar(colours, fullPopupWidth, ARRAY_SIZE(volumePopupText), volumePopupText, nullptr, evAdjustVolume, evAdjustVolume);
}

// Create the volume adjustment popup
void CreateInfoTimeoutPopup(const ColourScheme& colours)
{
	static const char* const infoTimeoutPopupText[Buzzer::MaxVolume + 1] = { "0", "2", "5", "10" };
	static const int values[] = { 0, 2, 5, 10 };
	infoTimeoutPopup = CreateIntPopupBar(colours, fullPopupWidth, ARRAY_SIZE(infoTimeoutPopupText), infoTimeoutPopupText, values, evAdjustInfoTimeout, evAdjustInfoTimeout);
}

// Create the colour scheme change popup
void CreateColoursPopup(const ColourScheme& colours)
{
	if (NumColourSchemes >= 2)
	{
		// Put all the colour scheme names in a single array for the call to CreateIntPopupBar
		const char* coloursPopupText[NumColourSchemes];
		for (size_t i = 0; i < NumColourSchemes; ++i)
		{
			coloursPopupText[i] = strings->colourSchemeNames[i];
		}
		coloursPopup = CreateIntPopupBar(colours, fullPopupWidth, NumColourSchemes, coloursPopupText, nullptr, evAdjustColours, evAdjustColours);
	}
	else
	{
		coloursPopup = nullptr;
	}
}

// Create the language popup (currently only affects the keyboard layout)
void CreateLanguagePopup(const ColourScheme& colours)
{
	languagePopup = new PopupWindow(popupBarHeight, fullPopupWidth, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	PixelNumber step = (fullPopupWidth - 2 * popupSideMargin + popupFieldSpacing)/NumLanguages;
	for (unsigned int i = 0; i < NumLanguages; ++i)
	{
		languagePopup->AddField(new TextButton(popupSideMargin, popupSideMargin + i * step, step - popupFieldSpacing, LanguageTables[i].languageName, evAdjustLanguage, i));
	}
}

// Create the pop-up keyboard
void CreateKeyboardPopup(uint32_t language, ColourScheme colours)
{
	static const char* array const keysEN[8] = { "1234567890-+", "QWERTYUIOP[]", "ASDFGHJKL:@", "ZXCVBNM,./", "!\"#$%^&*()_=", "qwertyuiop{}", "asdfghjkl;'", "zxcvbnm<>?" };
	static const char* array const keysDE[8] = { "1234567890-+", "QWERTZUIOP[]", "ASDFGHJKL:@", "YXCVBNM,./", "!\"#$%^&*()_=", "qwertzuiop{}", "asdfghjkl;'", "yxcvbnm<>?" };
	static const char* array const keysFR[8] = { "1234567890-+", "AZERTWUIOP[]", "QSDFGHJKLM@", "YXCVBN.,:/", "!\"#$%^&*()_=", "azertwuiop{}", "qsdfghjklm'", "yxcvbn<>;?" };
	static const char* array const * const keyboards[] = { keysEN, keysDE, keysFR, keysEN, keysEN };		// Spain and Czech keyboard layout is same as English

	static_assert(ARRAY_SIZE(keyboards) >= NumLanguages, "Wrong number of keyboard entries");

	keyboardPopup = new StandardPopupWindow(keyboardPopupHeight, keyboardPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupInfoTextColour, colours.buttonImageBackColour, nullptr, keyboardTopMargin);

	// Add the text area in which the command is built
	DisplayField::SetDefaultColours(colours.popupInfoTextColour, colours.popupInfoBackColour);		// need a different background colour
	userCommandField = new TextField(keyboardTopMargin + labelRowAdjust, popupSideMargin, keyboardPopupWidth - 2 * popupSideMargin - closeButtonWidth - popupFieldSpacing, TextAlignment::Left, nullptr, "_");
	userCommandField->SetLabel(userCommandBuffers[currentUserCommandBuffer].c_str());	// set up to display the current user command
	keyboardPopup->AddField(userCommandField);

	if (language >= NumLanguages)
	{
		language = 0;
	}

	currentKeyboard = keyboards[language];
	PixelNumber row = keyboardTopMargin + keyButtonVStep;

	for (size_t i = 0; i < 4; ++i)
	{
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		// New code using CharButtonRow to economise on RAM at the expense of more flash memory usage
		const PixelNumber column = popupSideMargin + (i * keyButtonHStep)/3;
		keyboardRows[i] = new CharButtonRow(row, column, keyButtonWidth, keyButtonHStep, currentKeyboard[i], evKey);
		keyboardPopup->AddField(keyboardRows[i]);
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.buttonImageBackColour);
		switch (i)
		{
		case 0:
			keyboardPopup->AddField(new IconButton(row, keyboardPopupWidth - popupSideMargin - (5 * keyButtonWidth)/4, (5 * keyButtonWidth)/4, IconBackspace, evBackspace));
			break;

		case 2:
			keyboardPopup->AddField(new TextButton(row, keyboardPopupWidth - popupSideMargin - (3 * keyButtonWidth)/2, (3 * keyButtonWidth)/2, UP_ARROW, evUp));
			break;

		case 3:
			keyboardPopup->AddField(new TextButton(row, keyboardPopupWidth - popupSideMargin - (3 * keyButtonWidth)/2, (3 * keyButtonWidth)/2, DOWN_ARROW, evDown));
			break;

		default:
			break;
		}
		row += keyButtonVStep;
	}

	// Add the shift, space and enter keys
	const PixelNumber keyButtonHSpace = keyButtonHStep - keyButtonWidth;
	const PixelNumber wideKeyButtonWidth = (keyboardPopupWidth - 2 * popupSideMargin - 2 * keyButtonHSpace)/5;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	keyboardPopup->AddField(new TextButton(row, popupSideMargin, wideKeyButtonWidth, "Shift", evShift, 0));
	keyboardPopup->AddField(new TextButton(row, popupSideMargin + wideKeyButtonWidth + keyButtonHSpace, 2 * wideKeyButtonWidth, "", evKey, (int)' '));
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.buttonImageBackColour);
	keyboardPopup->AddField(new IconButton(row, popupSideMargin + 3 * wideKeyButtonWidth + 2 * keyButtonHSpace, wideKeyButtonWidth, IconEnter, evSendKeyboardCommand));
}

// Create the babystep popup
void CreateBabystepPopup(const ColourScheme& colours)
{
	static const char * array const babystepStrings[2] = { LESS_ARROW " 0.02", MORE_ARROW " 0.02" };
	static const char * array const babystepAmounts[2] = { "-0.02", "+0.02" };
	babystepPopup = new StandardPopupWindow(babystepPopupHeight, babystepPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour,
			strings->babyStepping);
	PixelNumber ypos = popupTopMargin + babystepRowSpacing;
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	babystepPopup->AddField(babystepOffsetField = new FloatField(ypos, popupSideMargin, babystepPopupWidth - 2 * popupSideMargin, TextAlignment::Left, 3, strings->currentZoffset, "mm"));
	ypos += babystepRowSpacing;
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.buttonImageBackColour);
	CreateStringButtonRow(babystepPopup, ypos, popupSideMargin, babystepPopupWidth - 2 * popupSideMargin, fieldSpacing, 2, babystepStrings, babystepAmounts, evBabyStepAmount);
}

// Create the grid of heater icons and temperatures
void CreateTemperatureGrid(const ColourScheme& colours)
{
	// Add the emergency stop button
	DisplayField::SetDefaultColours(colours.stopButtonTextColour, colours.stopButtonBackColour);
	mgr.AddField(new TextButton(row2, margin, bedColumn - fieldSpacing - margin - 16, strings->stop, evEmergencyStop));

	// Add the labels and the debug field
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(debugField = new StaticTextField(row1 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Left, "debug"));
	mgr.AddField(new StaticTextField(row3 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, strings->current));
	mgr.AddField(new StaticTextField(row4 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, strings->active));
	mgr.AddField(new StaticTextField(row5 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, strings->standby));

	// Add the grid
	for (unsigned int i = 0; i < MaxHeaters; ++i)
	{
		const PixelNumber column = ((tempButtonWidth + fieldSpacing) * i) + bedColumn;

		// Add the icon button
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
		IconButtonWithText * const b = new IconButtonWithText(row2, column, tempButtonWidth, i == 0 ? IconBed : IconNozzle, evSelectHead, i, i);
		b->Show(false);
		toolButtons[i] = b;
		mgr.AddField(b);

		// Add the current temperature field
		DisplayField::SetDefaultColours(colours.infoTextColour, colours.defaultBackColour);
		FloatField * const f = new FloatField(row3 + labelRowAdjust, column, tempButtonWidth, TextAlignment::Centre, 1);
		f->SetValue(0.0);
		f->Show(false);
		currentTemps[i] = f;
		mgr.AddField(f);

		// Add the active temperature button
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
		IntegerButton *ib = new IntegerButton(row4, column, tempButtonWidth);
		ib->SetEvent(evAdjustActiveTemp, (int)i);
		ib->SetValue(0);
		ib->Show(false);
		activeTemps[i] = ib;
		mgr.AddField(ib);

		// Add the standby temperature button
		ib = new IntegerButton(row5, column, tempButtonWidth);
		ib->SetEvent(evAdjustStandbyTemp, (int)i);
		ib->SetValue(0);
		ib->Show(false);
		standbyTemps[i] = ib;
		mgr.AddField(ib);
	}
}

// Create the extra fields for the Control tab
void CreateControlTabFields(const ColourScheme& colours)
{
	mgr.SetRoot(commonRoot);

	DisplayField::SetDefaultColours(colours.infoTextColour, colours.infoBackColour);
	PixelNumber column = margin;
	PixelNumber xyFieldWidth = (DISPLAY_X - (2 * margin) - (MaxDisplayableAxes * fieldSpacing))/(MaxDisplayableAxes + 1);
	for (size_t i = 0; i < MaxDisplayableAxes; ++i)
	{
		FloatField * const f = new FloatField(row6p3 + labelRowAdjust, column, xyFieldWidth, TextAlignment::Left, (i == 2) ? 2 : 1, axisNames[i]);
		controlTabAxisPos[i] = f;
		f->SetValue(0.0);
		mgr.AddField(f);
		f->Show(i < MIN_AXES);
		column += xyFieldWidth + fieldSpacing;
	}
	zprobeBuf[0] = 0;
	mgr.AddField(zProbe = new TextField(row6p3 + labelRowAdjust, column, DISPLAY_X - column - margin, TextAlignment::Left, "P", zprobeBuf.c_str()));

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.notHomedButtonBackColour);
	homeAllButton = AddIconButton(row7p7, 0, MaxDisplayableAxes + 2, IconHomeAll, evSendCommand, "G28");
	homeButtons[0] = AddIconButtonWithText(row7p7, 1, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[0], axisNames[0]);
	homeButtons[1] = AddIconButtonWithText(row7p7, 2, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[1], axisNames[1]);
	homeButtons[2] = AddIconButtonWithText(row7p7, 3, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[2], axisNames[2]);
#if MaxDisplayableAxes > 3
	homeButtons[3] = AddIconButtonWithText(row7p7, 4, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[3], axisNames[3]);
	homeButtons[3]->Show(false);
#endif
#if MaxDisplayableAxes > 4
	homeButtons[4] = AddIconButtonWithText(row7p7, 5, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[4], axisNames[4]);
	homeButtons[4]->Show(false);
#endif
#if MaxDisplayableAxes > 5
	homeButtons[5] = AddIconButtonWithText(row7p7, 6, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[5], axisNames[5]);
	homeButtons[5]->Show(false);
#endif
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
	bedCompButton = AddIconButton(row7p7, MaxDisplayableAxes + 1, MaxDisplayableAxes + 2, IconBedComp, evSendCommand, "G32");

	filesButton = AddIconButton(row8p7, 0, 4, IconFiles, evListFiles, nullptr);
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	moveButton = AddTextButton(row8p7, 1, 4, strings->move, evMovePopup, nullptr);
	extrudeButton = AddTextButton(row8p7, 2, 4, strings->extrusion, evExtrudePopup, nullptr);
	macroButton = AddTextButton(row8p7, 3, 4, strings->macro, evListMacros, nullptr);

	// When there is room, we also display a few macro buttons on the right hand side
	for (size_t i = 0; i < NumControlPageMacroButtons; ++i)
	{
		// The position and width of the buttons will get corrected when we know how many tools we have
		TextButton * const b = controlPageMacroButtons[i] = new TextButton(row2 + i * rowHeight, 999, 99, nullptr, evNull);
		b->Show(false);			// hide them until we have loaded the macros
		mgr.AddField(b);
	}

	controlRoot = mgr.GetRoot();
}

// Create the fields for the Printing tab
void CreatePrintingTabFields(const ColourScheme& colours)
{
	mgr.SetRoot(commonRoot);

	// Labels
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(new StaticTextField(row6 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, strings->extruderPercent));

	// Extrusion factor buttons
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	for (unsigned int i = 0; i < MaxHeaters; ++i)
	{
		const PixelNumber column = ((tempButtonWidth + fieldSpacing) * i) + bedColumn;

		IntegerButton * const ib = new IntegerButton(row6, column, tempButtonWidth);
		ib->SetValue(100);
		ib->SetEvent(evExtrusionFactor, (int)i);
		ib->Show(false);
		extrusionFactors[i] = ib;
		mgr.AddField(ib);
	}

	// Speed button
	mgr.AddField(spd = new IntegerButton(row7, speedColumn, fanColumn - speedColumn - fieldSpacing, strings->speed, "%"));
	spd->SetValue(100);
	spd->SetEvent(evAdjustSpeed, "M220 S");

	// Fan button
	mgr.AddField(fanSpeed = new IntegerButton(row7, fanColumn, pauseColumn - fanColumn - fieldSpacing, strings->fan, "%"));
	fanSpeed->SetEvent(evAdjustFan, 0);
	fanSpeed->SetValue(0);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.pauseButtonBackColour);
	pauseButton = new TextButton(row7, pauseColumn, babystepColumn - pauseColumn - fieldSpacing, strings->pause, evPausePrint, "M25");
	mgr.AddField(pauseButton);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	babystepButton = new TextButton(row7, babystepColumn, DisplayX - babystepColumn - fieldSpacing, strings->babystep, evBabyStepPopup);
	mgr.AddField(babystepButton);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resumeButtonBackColour);
	resumeButton = new TextButton(row7, resumeColumn, cancelColumn - resumeColumn - fieldSpacing, strings->resume, evResumePrint, "M24");
	mgr.AddField(resumeButton);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resetButtonBackColour);
	resetButton = new TextButton(row7, cancelColumn, DisplayX - cancelColumn - margin, strings->cancel, evReset, "M0");
	mgr.AddField(resetButton);

#if DISPLAY_X == 800
	// On 5" and 7" screens there is room to show the current position on the Print page
	const PixelNumber offset = rowHeight - 20;
	DisplayField::SetDefaultColours(colours.infoTextColour, colours.infoBackColour);
	PixelNumber column = margin;
	PixelNumber xyFieldWidth = (DISPLAY_X - (2 * margin) - (MaxDisplayableAxes * fieldSpacing))/(MaxDisplayableAxes + 1);
	for (size_t i = 0; i < MaxDisplayableAxes; ++i)
	{
		FloatField * const f = new FloatField(row8 + labelRowAdjust - 4, column, xyFieldWidth, TextAlignment::Left, (i == 2) ? 2 : 1, axisNames[i]);
		printTabAxisPos[i] = f;
		f->SetValue(0.0);
		mgr.AddField(f);
		f->Show(i < MIN_AXES);
		column += xyFieldWidth + fieldSpacing;
	}
#else
	const PixelNumber offset = 0;
#endif

	DisplayField::SetDefaultColours(colours.progressBarColour,colours. progressBarBackColour);
	mgr.AddField(printProgressBar = new ProgressBar(row8 + offset + (rowHeight - progressBarHeight)/2, margin, progressBarHeight, DisplayX - 2 * margin));
	mgr.Show(printProgressBar, false);

	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(timeLeftField = new TextField(row9 + offset, margin, DisplayX - 2 * margin, TextAlignment::Left, strings->timeRemaining));
	mgr.Show(timeLeftField, false);

	printRoot = mgr.GetRoot();
}

// Create the fields for the Message tab
void CreateMessageTabFields(const ColourScheme& colours)
{
	mgr.SetRoot(baseRoot);
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
	mgr.AddField(new IconButton(margin,  DisplayX - margin - keyboardButtonWidth, keyboardButtonWidth, IconKeyboard, evKeyboard));
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(new StaticTextField(margin + labelRowAdjust, margin, DisplayX - 2 * margin - keyboardButtonWidth, TextAlignment::Centre, strings->messages));
	PixelNumber row = firstMessageRow;
	for (unsigned int r = 0; r < numMessageRows; ++r)
	{
		StaticTextField *t = new StaticTextField(row, margin, messageTimeWidth, TextAlignment::Left, nullptr);
		mgr.AddField(t);
		messageTimeFields[r] = t;
		t = new StaticTextField(row, messageTextX, messageTextWidth, TextAlignment::Left, nullptr);
		mgr.AddField(t);
		messageTextFields[r] = t;
		row += rowTextHeight;
	}
	messageRoot = mgr.GetRoot();
}

// Create the fields for the Setup tab
void CreateSetupTabFields(uint32_t language, const ColourScheme& colours)
{
	mgr.SetRoot(baseRoot);
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	// The firmware version field doubles up as an area for displaying debug messages, so make it the full width of the display
	mgr.AddField(fwVersionField = new TextField(row1, margin, DisplayX, TextAlignment::Left, strings->firmwareVersion, VERSION_TEXT));
	mgr.AddField(freeMem = new IntegerField(row2, margin, DisplayX/2 - margin, TextAlignment::Left, "Free RAM: "));
	mgr.AddField(new ColourGradientField(ColourGradientTopPos, ColourGradientLeftPos, ColourGradientWidth, ColourGradientHeight));

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	baudRateButton = AddIntegerButton(row3, 0, 3, nullptr, " baud", evSetBaudRate);
	baudRateButton->SetValue(GetBaudRate());
	volumeButton = AddIntegerButton(row3, 1, 3, strings->volume, nullptr, evSetVolume);
	volumeButton->SetValue(GetVolume());
	languageButton = AddTextButton(row3, 2, 3, LanguageTables[language].languageName, evSetLanguage, nullptr);
	AddTextButton(row4, 0, 3, strings->calibrateTouch, evCalTouch, nullptr);
	AddTextButton(row4, 1, 3, strings->mirrorDisplay, evInvertX, nullptr);
	AddTextButton(row4, 2, 3, strings->invertDisplay, evInvertY, nullptr);
	coloursButton = AddTextButton(row5, 0, 3, strings->colourSchemeNames[colours.index], evSetColours, nullptr);
	AddTextButton(row5, 1, 3, strings->brightnessDown, evDimmer, nullptr);
	AddTextButton(row5, 2, 3, strings->brightnessUp, evBrighter, nullptr);
	dimmingTypeButton = AddTextButton(row6, 0, 3, strings->displayDimmingNames[(unsigned int)GetDisplayDimmerType()], evSetDimmingType, nullptr);
	infoTimeoutButton = AddIntegerButton(row6, 1, 3, strings->infoTimeout, nullptr, evSetInfoTimeout);
	infoTimeoutButton->SetValue(infoTimeout);
	AddTextButton(row6, 2, 3, strings->clearSettings, evFactoryReset, nullptr);
	setupRoot = mgr.GetRoot();
}

void CreateCommonPendantFields(const ColourScheme &colours) {
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour, colours.buttonBorderColour, colours.buttonGradColour,
									colours.buttonPressedBackColour, colours.buttonPressedGradColour, colours.pal);
	tabJog = AddTextButton(rowTabsP, 0, 4, strings->jog, evTabJog, nullptr, DisplayXP);
	tabOffset = AddTextButton(rowTabsP, 1, 4, strings->offset, evTabOffset, nullptr, DisplayXP);
	tabJob = AddTextButton(rowTabsP, 2, 4, strings->job, evTabJob, nullptr, DisplayXP);
	AddTextButton(rowTabsP, 3, 4, strings->backToNormal, evDefaultRoot, nullptr, DisplayXP);

	// Add title bar
	DisplayField::SetDefaultColours(colours.titleBarTextColour, colours.titleBarBackColour);
	const PixelNumber width = CalcWidth(3, DisplayXP) + (2*margin);
	mgr.AddField(pNameField   = new StaticTextField(row1P, 0, width, TextAlignment::Left, machineName.c_str()));
	mgr.AddField(pStatusField = new StaticTextField(row1P, width, width, TextAlignment::Right, nullptr));

	// Add the emergency stop button
	DisplayField::SetDefaultColours(colours.stopButtonTextColour, colours.stopButtonBackColour);
	AddTextButton(row1P, 2, 3, strings->stop, evEmergencyStop, nullptr, DisplayXP);
}

void CreatePendantJogTabFields(const ColourScheme& colours) {
	mgr.SetRoot(pendantBaseRoot);

	DisplayField::SetDefaultColours(colours.titleBarTextColour, colours.titleBarBackColour);
	const PixelNumber colWidth = CalcWidth(3, DisplayXP);

	const PixelNumber jogBlock = row2P;
	const PixelNumber secondBlock = row9P;

	const unsigned int axisCol = 1;
	const unsigned int movementCol = 0;
	const unsigned int currentPosCol = 2;
	const unsigned int homingCol = 0;
	const unsigned int toolsCol = 1;
	const unsigned int extrudeCol = 2;

	mgr.AddField(new StaticTextField(jogBlock, CalcXPos(axisCol, colWidth),			colWidth, TextAlignment::Centre, strings->axis));
	mgr.AddField(new StaticTextField(jogBlock, CalcXPos(movementCol, colWidth),		colWidth, TextAlignment::Centre, strings->movement));
	mgr.AddField(new StaticTextField(jogBlock, CalcXPos(currentPosCol, colWidth),	colWidth, TextAlignment::Centre, strings->currentLocation));

	mgr.AddField(new StaticTextField(secondBlock, CalcXPos(homingCol, colWidth),	colWidth, TextAlignment::Centre, strings->homing));
	mgr.AddField(new StaticTextField(secondBlock, CalcXPos(toolsCol, colWidth),		colWidth, TextAlignment::Centre, strings->tools));
//	mgr.AddField(new StaticTextField(secondBlock, CalcXPos(extrudeCol, labelWidth),	 labelWidth, TextAlignment::Centre, strings->extrusion));
	mgr.AddField(new StaticTextField(secondBlock, CalcXPos(extrudeCol, colWidth),	colWidth, TextAlignment::Right, strings->current));
	mgr.AddField(new StaticTextField(secondBlock + 2 * rowHeightP, CalcXPos(extrudeCol, colWidth), colWidth, TextAlignment::Right, strings->active));
	mgr.AddField(new StaticTextField(secondBlock + 4 * rowHeightP, CalcXPos(extrudeCol, colWidth), colWidth, TextAlignment::Right, strings->standby));

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);

	static const float jogAmountValues[] = { 0.01, 0.10, 1.00 /*, 5.00 */ };

	// Axis selection
	currentJogAxis = CreateStringButtonRowVertical(&mgr, jogBlock + rowHeightP, CalcXPos(axisCol, colWidth), ARRAY_SIZE(jogAxes) * buttonHeight + (ARRAY_SIZE(jogAxes) - 1) * fieldSpacing, fieldSpacing, colWidth, ARRAY_SIZE(jogAxes), jogAxes, jogAxes, evPJogAxis, 0);

	// Distance per click
	currentJogAmount = CreateFloatButtonRowVertical(&mgr, jogBlock + rowHeightP, CalcXPos(movementCol, colWidth), ARRAY_SIZE(jogAmountValues) * buttonHeight + (ARRAY_SIZE(jogAmountValues) - 1) * fieldSpacing, fieldSpacing, colWidth, ARRAY_SIZE(jogAmountValues), "mm", 2, jogAmountValues, evPJogAmount, 2);

	// Axis position fields
	DisplayField::SetDefaultColours(colours.infoTextColour, colours.infoBackColour);
	PixelNumber row = jogBlock + rowHeightP;
	PixelNumber width = CalcWidth(3, DisplayXP);
	for (size_t i = 0; i < MaxDisplayableAxesP; ++i)
	{
		FloatField * const f = new FloatField(row + labelRowAdjust, CalcXPos(currentPosCol, width), width, TextAlignment::Right, 3);
		jogTabAxisPos[i] = f;
		f->SetValue(0.0);
		mgr.AddField(f);
		row += rowHeightP;
	}

	// Homing buttons
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.notHomedButtonBackColour);
	pHomeAllButton  = AddIconButton(secondBlock + 1 * rowHeightP, homingCol, 3, IconHomeAll,			evSendCommand,	"G28", DisplayXP);
	pHomeButtons[0] = AddIconButtonWithText(secondBlock + 2 * rowHeightP, homingCol, 3, IconHomeAll,	evHomeAxis, axisNames[0], axisNames[0], DisplayXP);
	pHomeButtons[1] = AddIconButtonWithText(secondBlock + 3 * rowHeightP, homingCol, 3, IconHomeAll,	evHomeAxis, axisNames[1], axisNames[1], DisplayXP);
	pHomeButtons[2] = AddIconButtonWithText(secondBlock + 4 * rowHeightP, homingCol, 3, IconHomeAll,	evHomeAxis, axisNames[2], axisNames[2], DisplayXP);
	measureZButton  = AddTextButton(secondBlock + 5 * rowHeightP, homingCol, 3, strings->measureZ,  evMeasureZ, 	"M98 P\"measureZ.g\"", DisplayXP);
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	                  AddTextButton(secondBlock + 6 * rowHeightP, homingCol, 3, strings->macro,     evListMacros,    nullptr, DisplayXP);

	// Tool selection buttons
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
	for (size_t i = 0; i < MaxPendantTools; ++i)
	{
		toolSelectButtons[i]  = AddIconButtonWithText(secondBlock + (i + 1) * rowHeightP, toolsCol, 3, IconNozzle, evToolSelect, i, i, DisplayXP);
	}

	// Extrusion/Heating
	// Add the current temperature field
	DisplayField::SetDefaultColours(colours.infoTextColour, colours.defaultBackColour);
	currentTempPJog = new FloatField(secondBlock + 1 * rowHeightP, CalcXPos(extrudeCol, colWidth), colWidth, TextAlignment::Centre, 1);
	currentTempPJog->SetValue(0.0);
	mgr.AddField(currentTempPJog);

	// Add the active temperature button
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	activeTempPJog = AddIntegerButton(secondBlock + 3 * rowHeightP, extrudeCol, 3, nullptr, nullptr, evAdjustActiveTemp, DisplayXP);
	activeTempPJog->SetValue(0);
	activeTempPJog->SetEvent(evAdjustActiveTemp, (int)-1);

	// Add the standby temperature button
	standbyTempPJog = AddIntegerButton(secondBlock + 5 * rowHeightP, extrudeCol, 3, nullptr, nullptr, evAdjustStandbyTemp, DisplayXP);
	standbyTempPJog->SetValue(0);
	standbyTempPJog->SetEvent(evAdjustStandbyTemp, (int)-1);

	// Add the Extrude popup button
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	AddTextButton(secondBlock + 6 * rowHeightP, extrudeCol, 3, strings->extrusion, evExtrudePopup, nullptr, DisplayXP);


	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	pendantJogRoot = mgr.GetRoot();
}

void CreatePendantOffsetTabFields(const ColourScheme& colours) {
	mgr.SetRoot(pendantBaseRoot);

	DisplayField::SetDefaultColours(colours.titleBarTextColour, colours.titleBarBackColour);
	const PixelNumber fullWidth = CalcWidth(1, DisplayXP);
	const PixelNumber xPos = CalcXPos(0, fullWidth);
	mgr.AddField(new StaticTextField(row2P, xPos, fullWidth, TextAlignment::Centre, strings->probeWorkpiece));
	mgr.AddField(new StaticTextField(row8P, xPos, fullWidth, TextAlignment::Centre, strings->touchOff));
	mgr.AddField(new StaticTextField(row11P, xPos, fullWidth, TextAlignment::Centre, strings->toolOffset));
	mgr.AddField(new StaticTextField(row14P, xPos, fullWidth, TextAlignment::Centre, strings->zeroWorkpiece));

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
	AddIconButton(row3P, 1, 4, Ymax2min, evProbeWorkpiece, "Ymin", DisplayXP);
	AddIconButton(row4P, 0, 4, Xmin2max, evProbeWorkpiece, "Xmax", DisplayXP);
	AddIconButton(row4P, 2, 4, Xmax2min, evProbeWorkpiece, "Xmin", DisplayXP);
	AddIconButton(row5P, 1, 4, Ymin2max, evProbeWorkpiece, "Ymax", DisplayXP);
	AddIconButton(row5P, 3, 4, Zmax2min, evProbeWorkpiece, "Zmin", DisplayXP);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	AddTextButton(row6P, 0, 1, strings->findCenterOfCavity, evFindCenterOfCavity, nullptr, DisplayXP);

	AddTextButton(row9P, 0, 2, "X-Y", evTouchoff, "X-Y", DisplayXP);
	AddTextButton(row9P, 1, 2, "Z", evTouchoff, "Z", DisplayXP);

	DisplayField::SetDefaultColours(colours.infoTextColour, colours.infoBackColour);
	const PixelNumber w = CalcWidth(3, DisplayXP);
	mgr.AddField(currentToolField = new IntegerField(row12P, CalcXPos(0, w), w, TextAlignment::Centre));
	currentToolField->SetValue(currentTool);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
	AddIconButton(row12P, 1, 3, SetToolOffsetXY, evSetToolOffset, 0, DisplayXP);
	AddIconButton(row12P, 2, 3, SetToolOffsetZ, evSetToolOffset, 1, DisplayXP);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	AddTextButton(row15P, 0, 5, "X", evZeroAxisInWCS, "X", DisplayXP);
	AddTextButton(row15P, 1, 5, "Y", evZeroAxisInWCS, "Y", DisplayXP);
	AddTextButton(row15P, 2, 5, "Z", evZeroAxisInWCS, "Z", DisplayXP);
	AddTextButton(row15P, 3, 5, "A", evZeroAxisInWCS, "A", DisplayXP);
	AddTextButton(row15P, 4, 5, "C", evZeroAxisInWCS, "C", DisplayXP);

	pendantOffsetRoot = mgr.GetRoot();
}

void CreatePendantJobTabFields(const ColourScheme& colours) {
	mgr.SetRoot(pendantBaseRoot);

	const PixelNumber fullWidth = CalcWidth(1, DisplayXP);
	const PixelNumber xPos = CalcXPos(0, fullWidth);
	mgr.AddField(jobTextField = new StaticTextField(row2P, xPos, fullWidth, TextAlignment::Centre, strings->noJob));


	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.pauseButtonBackColour);
	pPauseButton = AddTextButton(row3P, 0, 1, strings->pause, evPausePrint, "M25", DisplayXP);
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resumeButtonBackColour);
	pResumeButton = AddTextButton(row3P, 0, 2, strings->resume, evResumePrint, "M24", DisplayXP);
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resetButtonBackColour);
	pResetButton = AddTextButton(row3P, 1, 2, strings->cancel, evReset, "M0", DisplayXP);

	DisplayField::SetDefaultColours(colours.progressBarColour, colours.progressBarBackColour);
	mgr.AddField(printProgressBarP = new ProgressBar(row4P + (rowHeightP - progressBarHeight)/2, margin, progressBarHeight, DisplayXP - 2 * margin));
	mgr.Show(printProgressBarP, false);

	DisplayField::SetDefaultColours(colours.infoTextColour, colours.infoBackColour);
	const PixelNumber colWidth = CalcWidth(5, DisplayXP);
	unsigned int actualCol = 0;
	for (size_t i = 0; i < MaxDisplayableAxesP; ++i)
	{
		FloatField * const f = new FloatField(row5P + labelRowAdjust - 4, CalcXPos(actualCol, colWidth), colWidth, TextAlignment::Left, (i == 2) ? 2 : 1);
		jobTabAxisPos[i] = f;
		f->SetValue(0.0);
		mgr.AddField(f);
		++actualCol;
	}

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	// Speed button
	pFeedrateButton = AddIntegerButton(row6P, 0, 2, strings->speed, "%", evAdjustSpeed, DisplayXP);
	pFeedrateButton->SetValue(100);
	pFeedrateButton->SetEvent(evAdjustSpeed, "M220 S");
	pExtruderPercentButton = AddIntegerButton(row6P, 1, 2, strings->extruderShort, "%", evPAdjustExtrusionPercent, DisplayXP);
	pExtruderPercentButton->SetValue(100);
	pExtruderPercentButton->SetEvent(evPAdjustExtrusionPercent, "M221 S");

	pSpindleRPMButton = AddIntegerButton(row7P, 0, 1, strings->spindleRPM, nullptr, evAdjustActiveRPM, DisplayXP);
	pSpindleRPMButton->SetValue(0);

	// Heating control
	DisplayField::SetDefaultColours(colours.titleBarTextColour, colours.titleBarBackColour);
	mgr.AddField(new StaticTextField(row8P + ((rowHeightP)/3), xPos, fullWidth, TextAlignment::Centre, strings->heatControl));

	const PixelNumber iconWidth = 50;
	const PixelNumber iconColWidth = margin + iconWidth;
	const PixelNumber currentColWidth = 140;
	const PixelNumber activeColWidth = 128;
	const PixelNumber standbyColWidth = 154;
	const PixelNumber iconCol = margin;
	const PixelNumber currentCol = iconCol + iconColWidth + margin - 8;
	const PixelNumber activeCol = currentCol + currentColWidth + margin;
	const PixelNumber standbyCol = activeCol + activeColWidth + margin;
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(new StaticTextField(row9P, currentCol, currentColWidth, TextAlignment::Centre, strings->current));
	mgr.AddField(new StaticTextField(row9P, activeCol, activeColWidth, TextAlignment::Centre, strings->active));
	mgr.AddField(new StaticTextField(row9P, standbyCol, standbyColWidth, TextAlignment::Centre, strings->standby));

	// Add the grid
	for (unsigned int i = 0; i < 6; ++i)
	{
		const PixelNumber row = row10P + i * rowHeightP;

		// Add the icon button
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
		IconButtonWithText * const b = new IconButtonWithText(row, iconCol, iconWidth, IconNozzle, evSelectHead, i, i);
		b->Show(false);
		toolButtonsPJob[i] = b;
		mgr.AddField(b);

		// Add the current temperature field
		DisplayField::SetDefaultColours(colours.infoTextColour, colours.defaultBackColour);
		FloatField * const f = new FloatField(row + labelRowAdjust, currentCol+25, tempButtonWidth, TextAlignment::Centre, 1);
		f->SetValue(0.0);
		f->Show(false);
		currentTempsPJob[i] = f;
		mgr.AddField(f);

		// Add the active temperature button
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
		IntegerButton *ib = new IntegerButton(row, activeCol+20, tempButtonWidth);
		ib->SetEvent(evAdjustActiveTemp, (int)i);
		ib->SetValue(0);
		ib->Show(false);
		activeTempsPJob[i] = ib;
		mgr.AddField(ib);

		// Add the standby temperature button
		ib = new IntegerButton(row, standbyCol+30, tempButtonWidth);
		ib->SetEvent(evAdjustStandbyTemp, (int)i);
		ib->SetValue(0);
		ib->Show(false);
		standbyTempsPJob[i] = ib;
		mgr.AddField(ib);
	}

	pendantJobRoot = mgr.GetRoot();
}

// Create the fields for the Pendant root
void CreatePendantRoot(const ColourScheme& colours)
{
	PortraitDisplay(false);

	mgr.SetRoot(nullptr);
	CreateCommonPendantFields(colours);
	pendantBaseRoot = mgr.GetRoot();		// save the root of fields that we usually display in pendant mode

	CreatePendantJogTabFields(colours);
	CreatePendantOffsetTabFields(colours);
	CreatePendantJobTabFields(colours);

	// Pop-ups
	CreateAreYouSurePopupPortrait(colours);
	CreateExtrudePopupP(colours);
	macrosPopupP = CreateFileListPopup(macrosListButtonsP, macroButtonsP, NumMacroRowsP, NumMacroColumnsP, colours, false, MacroListPopupHeightP, MacroListPopupWidthP);
	alertPopupP = new AlertPopupP(colours);

	LandscapeDisplay(false);
}

// Create the fields that are displayed on all pages
void CreateCommonFields(const ColourScheme& colours)
{
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour, colours.buttonBorderColour, colours.buttonGradColour,
									colours.buttonPressedBackColour, colours.buttonPressedGradColour, colours.pal);
	tabControl = AddTextButton(rowTabs, 0, 5, strings->control, evTabControl, nullptr);
	tabPrint = AddTextButton(rowTabs, 1, 5, strings->print, evTabPrint, nullptr);
	tabMsg = AddTextButton(rowTabs, 2, 5, strings->console, evTabMsg, nullptr);
	tabPendant = AddTextButton(rowTabs, 3, 5, strings->pendant, evPendantRoot, nullptr);
	tabSetup = AddTextButton(rowTabs, 4, 5, strings->setup, evTabSetup, nullptr);
}

void CreateMainPages(uint32_t language, const ColourScheme& colours)
{
	if (language >= ARRAY_SIZE(LanguageTables))
	{
		language = 0;
	}
	strings = &LanguageTables[language];
	CreateCommonFields(colours);
	baseRoot = mgr.GetRoot();		// save the root of fields that we usually display

	// Create the fields that are common to the Control and Print pages
	DisplayField::SetDefaultColours(colours.titleBarTextColour, colours.titleBarBackColour);
	mgr.AddField(nameField = new StaticTextField(row1, 0, DisplayX - statusFieldWidth, TextAlignment::Centre, machineName.c_str()));
	mgr.AddField(statusField = new StaticTextField(row1, DisplayX - statusFieldWidth, statusFieldWidth, TextAlignment::Right, nullptr));
	CreateTemperatureGrid(colours);
	commonRoot = mgr.GetRoot();		// save the root of fields that we display on more than one page

	// Create the pages
	CreateControlTabFields(colours);
	CreatePrintingTabFields(colours);
	CreateMessageTabFields(colours);
	CreateSetupTabFields(language, colours);
}

namespace UI
{
	static void Adjusting(ButtonPress bp)
	{
		fieldBeingAdjusted = bp;
		if (bp == currentButton)
		{
			currentButton.Clear();		// to stop it being released
		}
	}

	static void StopAdjusting()
	{
		if (fieldBeingAdjusted.IsValid())
		{
			mgr.Press(fieldBeingAdjusted, false);
			fieldBeingAdjusted.Clear();
		}
	}

	static void CurrentButtonReleased()
	{
		if (currentButton.IsValid())
		{
			mgr.Press(currentButton, false);
			currentButton.Clear();
		}
	}

	static void ClearAlertOrResponse();

	// Return the number of supported languages
	extern unsigned int GetNumLanguages()
	{
		return NumLanguages;
	}

	// Create all the fields we ever display
	void CreateFields(uint32_t language, const ColourScheme& colours, uint32_t p_infoTimeout)
	{
		infoTimeout = p_infoTimeout;

		// Set up default colours and margins
		mgr.Init(colours.defaultBackColour);
		DisplayField::SetDefaultFont(DEFAULT_FONT);
		ButtonWithText::SetFont(DEFAULT_FONT);
		CharButtonRow::SetFont(DEFAULT_FONT);
		SingleButton::SetTextMargin(textButtonMargin);
		SingleButton::SetIconMargin(iconButtonMargin);

		// Create the pages
		CreateMainPages(language, colours);

		CreatePendantRoot(colours);

		// Create the popup fields
		CreateIntegerAdjustPopup(colours);
		CreateIntegerRPMAdjustPopup(colours);
#ifdef SUPPORT_ENCODER
		CreateIntegerAdjustWithEncoderPopup(colours);
#endif
		CreateMovePopup(colours);
		CreateExtrudePopup(colours);
		fileListPopup = CreateFileListPopup(filesListButtons, filenameButtons, NumFileRows, NumFileColumns, colours, true);
		macrosPopup = CreateFileListPopup(macrosListButtons, macroButtons, NumMacroRows, NumMacroColumns, colours, false);
		CreateFileActionPopup(colours);
		CreateVolumePopup(colours);
		CreateInfoTimeoutPopup(colours);
		CreateBaudRatePopup(colours);
		CreateColoursPopup(colours);
		CreateAreYouSurePopup(colours);
		CreateKeyboardPopup(language, colours);
		CreateLanguagePopup(colours);
		alertPopup = new AlertPopup(colours);
		CreateBabystepPopup(colours);

		DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
		touchCalibInstruction = new StaticTextField(DisplayY/2 - 10, 0, DisplayX, TextAlignment::Centre, strings->touchTheSpot);

		mgr.SetRoot(nullptr);

#ifdef SUPPORT_ENCODER
		encoder = new RotaryEncoder(2, 3, 32+6);			// PA2, PA3 and PB6
		encoder->Init(4);
#endif
	}

	void ShowFilesButton()
	{
		mgr.Show(resumeButton, false);
		mgr.Show(pResumeButton, false);
		mgr.Show(resetButton, false);
		mgr.Show(pResetButton, false);
		mgr.Show(pauseButton, false);
		mgr.Show(pPauseButton, false);
		mgr.Show(babystepButton, false);
		mgr.Show(filesButton, true);
	}

	void ShowPauseButton()
	{
		mgr.Show(resumeButton, false);
		mgr.Show(pResumeButton, false);
		mgr.Show(resetButton, false);
		mgr.Show(pResetButton, false);
		mgr.Show(filesButton, false);
		mgr.Show(pauseButton, true);
		mgr.Show(pPauseButton, true);
		mgr.Show(babystepButton, true);
	}

	void ShowResumeAndCancelButtons()
	{
		mgr.Show(pauseButton, false);
		mgr.Show(pPauseButton, false);
		mgr.Show(babystepButton, false);
		mgr.Show(filesButton, false);
		mgr.Show(resumeButton, true);
		mgr.Show(pResumeButton, true);
		mgr.Show(resetButton, true);
		mgr.Show(pResetButton, true);
	}

	bool IsVisibleAxisPendant(const char * axis)
	{
		for (size_t i = 0; i < MaxDisplayableAxesP; ++i)
		{
			if (jogAxes[i][0] == axis[0])
			{
				return true;
			}
		}
		return false;
	}

	// Show or hide an axis on the move button grid and on the axis display
	void ShowAxis(size_t slot, bool b)
	{
		if (slot >= MaxDisplayableAxes)
		{
			return;
		}
		// The table gives us a pointer to the label field, which is followed by 8 buttons. So we need to show or hide 9 fields.
		DisplayField *f = moveAxisRows[slot];
		for (int i = 0; i < 9 && f != nullptr; ++i)
		{
			f->Show(b);
			f = f->next;
		}
		controlTabAxisPos[slot]->Show(b);
		printTabAxisPos[slot]->Show(b);
		if (numDisplayedAxes < MaxDisplayableAxes)
		{
			movePopupAxisPos[slot]->Show(b);		// the move popup axis positions occupy the last axis row of the move popup
		}
		else
		{
			// This is incremental and we might end up that this row is no longer available
			for (size_t i = 0; i < MaxDisplayableAxes; ++i)
			{
				movePopupAxisPos[i]->Show(false);
			}
		}
	}

	// Show or hide an axis on the move button grid and on the axis display
	void ShowAxisP(size_t slot, bool b)
	{
		if (slot >= MaxDisplayableAxesP)
		{
			return;
		}
		jobTabAxisPos[slot]->Show(b);
	}

	void UpdateAxisPosition(size_t axis, float fval)
	{
		if (axis < MaxTotalAxes)
		{
			size_t slot = axes[axis].slot;
			if (slot < MaxDisplayableAxes)
			{
				if (controlTabAxisPos[slot] != nullptr)
				{
					controlTabAxisPos[slot]->SetValue(fval);
				}
				if (printTabAxisPos[slot] != nullptr)
				{
					printTabAxisPos[slot]->SetValue(fval);
				}
				if (movePopupAxisPos[slot] != nullptr)
				{
					movePopupAxisPos[slot]->SetValue(fval);
				}
			}
			size_t slotP = axes[axis].slotP;
			if (slotP < MaxDisplayableAxesP)
			{
				jogTabAxisPos[slotP]->SetValue(fval);
				jobTabAxisPos[slotP]->SetValue(fval);
			}
		}
	}

	template<class T>
	T* GetOrCreate(T*& start, size_t index, bool create)
	{
		T* ret = start;
		for (; ret != nullptr; ret = ret->next)
		{
			if (ret->index == index)
			{
				break;
			}
		}

		// Not found, create new
		if (ret == nullptr && create)
		{
			ret = new T;
			ret->index = index;
			if (start == nullptr || start->index > index)
			{
				ret->next = start;
				start = ret;
			}
			else
			{
				// Insert it at the correct spot
				auto current = start;
				while (current->next != nullptr && current->next->index < index)
				{
					current = current->next;
				}
				ret->next = current->next;
				current->next = ret;
			}
		}
		return ret;
	}

	Spindle* GetOrCreateSpindle(size_t index, bool create = false)
	{
		return GetOrCreate(spindles, index, create);
	}

	Tool* GetOrCreateTool(size_t index, bool create = false)
	{
		return GetOrCreate(tools, index, create);
	}

	Tool* GetToolForExtruder(size_t extruder)
	{
		for (auto tool = tools; tool != nullptr; tool = tool->next)
		{
			if (tool->extruder == (int)extruder)
			{
				return tool;
			}
		}
		return nullptr;
	}

	Tool* GetToolForHeater(size_t heater)
	{
		for (auto tool = tools; tool != nullptr; tool = tool->next)
		{
			if (tool->heater == (int)heater)
			{
				return tool;
			}
		}
		return nullptr;
	}

	int GetHeaterSlot(size_t heater)
		{
		if ((int)heater == bedHeater.heater)
		{
			return bedHeater.slot;
		}
		if ((int)heater == chamberHeater.heater)
		{
			return chamberHeater.slot;
		}
		auto tool = GetToolForHeater(heater);
		return tool != nullptr ? tool->slot : -1;
	}

	void UpdateCurrentTemperature(size_t heater, float fval)
	{
		const int heaterSlot = GetHeaterSlot(heater);
		if (heaterSlot < 0)
		{
			return;
		}
		currentTemps[heaterSlot]->SetValue(fval);

		auto tool = GetToolForHeater(heater);
		if (tool != nullptr)
		{
			if (tool->index == currentTool)
			{
				currentTempPJog->SetValue(fval);
			}

			if (tool->slotP < MaxPendantTools)
			{
				currentTempsPJob[tool->slotP]->SetValue(fval);
			}
		}
	}

	void UpdateHeaterStatus(const size_t heater, const HeaterStatus status)
	{
		const int heaterSlot = GetHeaterSlot(heater);
		if (heaterSlot < 0)
		{
			return;
		}
		heaterStatus[heaterSlot] = status;
		Colour c = (status == HeaterStatus::standby) ? colours->standbyBackColour
						: (status == HeaterStatus::active) ? colours->activeBackColour
						: (status == HeaterStatus::fault) ? colours->errorBackColour
						: (status == HeaterStatus::tuning) ? colours->tuningBackColour
						: colours->defaultBackColour;
		if (currentTemps[heaterSlot] != nullptr)
		{
			currentTemps[heaterSlot]->SetColours((status == HeaterStatus::fault) ? colours->errorTextColour : colours->infoTextColour, c);
		}
		auto tool = GetToolForHeater(heater);
		if (tool != nullptr)
		{
			if (tool->index == currentTool)
			{
				currentTempPJog->SetColours((status == HeaterStatus::fault) ? colours->errorTextColour : colours->infoTextColour, c);
			}
			if (tool->slotP < MaxPendantTools)
			{
				currentTempsPJob[tool->slotP]->SetColours((status == HeaterStatus::fault) ? colours->errorTextColour : colours->infoTextColour, c);
			}
		}
	}

	void SetCurrentTool(int32_t ival)
	{
		if (ival == currentTool)
		{
			return;
		}
		currentTool = ival;
		if (currentTool < 0)
		{
			currentTempPJog->SetValue(0);
			currentTempPJog->SetColours(colours->infoTextColour, colours->defaultBackColour);
			UpdateField(activeTempPJog, 0);
			UpdateField(standbyTempPJog, 0);
			UpdateField(pSpindleRPMButton, 0);
		}
		else
		{
			auto tool = GetOrCreateTool(currentTool);
			if (tool != nullptr)
			{
				const bool hasHeater = tool->heater > -1;
				const bool hasSpindle = tool->spindle != nullptr;
				mgr.Show(currentTempPJog, hasHeater || hasSpindle);
				mgr.Show(activeTempPJog, hasHeater || hasSpindle);
				mgr.Show(standbyTempPJog, hasHeater);
				standbyTempPJog->SetEvent(standbyTempPJog->GetEvent(), tool->index);

				// We rarely see updates of this value so update it here
				if (hasSpindle)
				{
					UpdateField(activeTempPJog, tool->spindle->active);
					activeTempPJog->SetEvent(evAdjustActiveRPM, tool->spindle->index);
				}
				else
				{
					UpdateField(pSpindleRPMButton, 0);
				}
			}
		}
		currentToolField->SetValue(currentTool);
	}

	static int timesLeft[3];
	static String<50> timesLeftText;

	void ChangeStatus(PrinterStatus oldStatus, PrinterStatus newStatus)
	{
		switch (newStatus)
		{
		case PrinterStatus::printing:
		case PrinterStatus::simulating:
			if (oldStatus != PrinterStatus::paused && oldStatus != PrinterStatus::resuming)
			{
				// Starting a new print, so clear the times
				timesLeft[0] = timesLeft[1] = timesLeft[2] = 0;
			}
			[[fallthrough]];
		case PrinterStatus::paused:
		case PrinterStatus::pausing:
		case PrinterStatus::resuming:
			if (oldStatus == PrinterStatus::connecting || oldStatus == PrinterStatus::idle)
			{
				PrintStarted();
			}
			else if (currentTab == tabPrint)
			{
				nameField->SetValue(printingFile.c_str());
			}
			break;

		case PrinterStatus::idle:
			printingFile.clear();
			nameField->SetValue(machineName.c_str());		// if we are on the print tab then it may still be set to the file that was being printed
			__attribute__ ((fallthrough));
			// no break
		case PrinterStatus::configuring:
			if (oldStatus == PrinterStatus::flashing)
			{
				mgr.ClearAllPopups();						// clear the firmware update message
			}
			break;

		case PrinterStatus::connecting:
			printingFile.clear();
			// We no longer clear the machine name here
			mgr.ClearAllPopups();
			break;

		default:
			nameField->SetValue(machineName.c_str());
			break;
		}
	}

	// Append an amount of time to timesLeftText
	static void AppendTimeLeft(int t)
	{
		if (t <= 0)
		{
			timesLeftText.cat(strings->notAvailable);
		}
		else if (t < 60)
		{
			timesLeftText.catf("%ds", t);
		}
		else if (t < 60 * 60)
		{
			timesLeftText.catf("%dm %02ds", t/60, t%60);
		}
		else
		{
			t /= 60;
			timesLeftText.catf("%dh %02dm", t/60, t%60);
		}
	}

	void UpdateTimesLeft(size_t index, unsigned int seconds)
	{
		if (index < (int)ARRAY_SIZE(timesLeft))
		{
			timesLeft[index] = seconds;
			timesLeftText.copy(strings->file);
			AppendTimeLeft(timesLeft[0]);
			timesLeftText.cat(strings->filament);
			AppendTimeLeft(timesLeft[1]);
			if (DisplayX >= 800)
			{
				timesLeftText.cat(strings->layer);
				AppendTimeLeft(timesLeft[2]);
			}
			timeLeftField->SetValue(timesLeftText.c_str());
			mgr.Show(timeLeftField, true);
		}
	}

	void ChangeRoot(ButtonBase *newRoot)
	{
		if (newRoot == currentTab)
		{
			mgr.ClearAllPopups();						// if already on the correct page, just clear popups
		}
		else
		{
			switch (newRoot->GetEvent())
			{
			case evDefaultRoot:
				lastPendantTab = currentTab;
				LandscapeDisplay();
				isLandscape = true;
				ChangePage(lastRegularTab != nullptr ? lastRegularTab : tabControl);
				break;
			case evPendantRoot:
				lastRegularTab = currentTab;
				PortraitDisplay();
				isLandscape = false;
				ChangePage(lastPendantTab != nullptr ? lastPendantTab : tabJog);
				break;
			}
		}
	}

	// Change to the page indicated. Return true if the page has a permanently-visible button.
	bool ChangePage(ButtonBase *newTab)
	{
		if (newTab == currentTab)
		{
			mgr.ClearAllPopups();						// if already on the correct page, just clear popups
		}
		else
		{
			if (currentTab != nullptr)
			{
				currentTab->Press(false, 0);			// remove highlighting from the old tab
				if (currentTab->GetEvent() == evTabSetup && IsSaveNeeded())
				{
					SaveSettings();						// leaving the Control tab and we have changed settings, so save them
				}
			}
			newTab->Press(true, 0);						// highlight the new tab
			currentTab = newTab;
			mgr.ClearAllPopups();
			switch(newTab->GetEvent())
			{
			case evTabControl:
				mgr.SetRoot(controlRoot);
				nameField->SetValue(machineName.c_str());
				break;
			case evTabPrint:
				mgr.SetRoot(printRoot);
				nameField->SetValue(PrintInProgress() ? printingFile.c_str() : machineName.c_str());
				break;
			case evTabMsg:
				mgr.SetRoot(messageRoot);
				if (keyboardIsDisplayed)
				{
					mgr.SetPopup(keyboardPopup, AutoPlace, keyboardPopupY, false);
				}
				break;
			case evTabSetup:
				mgr.SetRoot(setupRoot);
				break;
			case evTabJog:
				mgr.SetRoot(pendantJogRoot);
				break;
			case evTabOffset:
				mgr.SetRoot(pendantOffsetRoot);
				break;
			case evTabJob:
				mgr.SetRoot(pendantJobRoot);
				jobTextField->SetValue(PrintInProgress() ? printingFile.c_str() : strings->noJob);
				break;
			default:
				mgr.SetRoot(commonRoot);
				break;
			}
			mgr.Refresh(true);
		}
		return true;
	}

	// Pop up the keyboard
	void ShowKeyboard()
	{
		mgr.SetPopup(keyboardPopup, AutoPlace, keyboardPopupY);
		keyboardIsDisplayed = true;
	}

	// This is called when the Cancel button on a popup is pressed
	void PopupCancelled()
	{
		if (mgr.GetPopup() == keyboardPopup)
		{
			keyboardIsDisplayed = false;
		}
	}

	// Return true if polling should be performed
	bool DoPolling()
	{
		return currentTab != tabSetup;			// don't poll while we are on the Setup page
	}

	void Tick()
	{
#ifdef SUPPORT_ENCODER
		encoder->Poll();
#endif
	}

#ifdef SUPPORT_ENCODER
	void HandleEncoderChange(const int change)
	{
		auto status = GetStatus();

		// In some cases we just ignore user input
		switch (status)
		{
		case PrinterStatus::configuring:
		case PrinterStatus::connecting:
		case PrinterStatus::flashing:
		case PrinterStatus::panelInitializing:
			return;
		default:
			break;
		}
		const PopupWindow *popup = mgr.GetPopup();
		// There is a pop-up - we will exit here after possibly doing something
		if (popup != nullptr)
		{
			if (popup == setTempPopupEncoder)
			{
				if (fieldBeingAdjusted.IsValid())
				{
					IntegerButton *ib = static_cast<IntegerButton*>(fieldBeingAdjusted.GetButton());
					int newValue = ib->GetValue() + change;
					switch(fieldBeingAdjusted.GetEvent())
					{
					case evAdjustActiveTemp:
					case evAdjustStandbyTemp:
						newValue = constrain<int>(newValue, 0, 1600);		// some users want to print at high temperatures
						break;

					case evAdjustSpeed:
						newValue = constrain<int>(newValue, 1, 1000);
						break;

					case evPAdjustExtrusionPercent:
						newValue = constrain<int>(newValue, 1, 1000);
						break;

					case evAdjustActiveRPM:
						{
							auto spindle = GetOrCreateSpindle(fieldBeingAdjusted.GetIParam());
							static const int spindleRpmMultiplier = 100;
							static const int maxSpindleRpm = spindle->max;
							newValue = constrain<int>((newValue - change) + (change * spindleRpmMultiplier), -maxSpindleRpm, maxSpindleRpm);
						}
						break;

					default:
						break;
					}
					ib->SetValue(newValue);
				}
			}
			return;
		}

		bool sent = false;
		// Jog axis around
		if (currentTab == tabJog && status != PrinterStatus::printing)
		{
			if (currentJogAxis.IsValid() && currentJogAmount.IsValid())
			{
				const float jogAmount = currentJogAmount.GetFParam();
				const unsigned int feedRate = jogAmount < 5.0f ? 6000 : 12000;
				String<MaxEncoderCommandLength> cmd;
				cmd.printf("G91 G0 %s%.3f F%d G90\n", currentJogAxis.GetSParam(), change * jogAmount, feedRate);
				SerialIo::SendString(cmd.c_str());
				sent = true;
			}
		}

		if (sent) {
			lastEncoderCommandSentAt = SystemTick::GetTickCount();
		}
	}
#endif

	// This is called in the main spin loop
	void Spin()
	{
#ifdef SUPPORT_ENCODER
		if (SystemTick::GetTickCount() - lastEncoderCommandSentAt >= MinimumEncoderCommandInterval)
		{
			// Check encoder and command movement
			const int ch = encoder->GetChange();
			if (ch != 0)
			{
				HandleEncoderChange(ch);
			}
		}
#endif

		if (currentTab == tabMsg)
		{
			MessageLog::UpdateMessages(false);
		}
		if (alertTicks != 0 && SystemTick::GetTickCount() - whenAlertReceived >= alertTicks)
		{
			ClearAlertOrResponse();
		}
	}

	// This is called when we have just started a file print
	void PrintStarted()
	{
		if (isLandscape)
		{
		ChangePage(tabPrint);
	}
		else
		{
			ChangePage(tabJob);
		}
	}

	// This is called when we have just received the name of the file being printed
	void PrintingFilenameChanged(const char data[])
	{
		if (!printingFile.similar(data))
		{
			printingFile.copy(data);
			if ((currentTab == tabPrint || currentTab == tabJob) && PrintInProgress())
			{
				nameField->SetChanged();
				jobTextField->SetChanged();
			}
		}
	}

	// This is called just before the main polling loop starts. Display the default page.
	void ShowDefaultPage()
	{
		PortraitDisplay();
		isLandscape = false;
		ChangePage(tabJog);
//		ChangePage(tabControl);
	}

	// Update the fields that are to do with the printing status
	void UpdatePrintingFields()
	{
		if (GetStatus() == PrinterStatus::printing || GetStatus() == PrinterStatus::simulating)
		{
			ShowPauseButton();
		}
		else if (GetStatus() == PrinterStatus::paused)
		{
			ShowResumeAndCancelButtons();
		}
		else
		{
			ShowFilesButton();
		}

		mgr.Show(printProgressBar, PrintInProgress());
		mgr.Show(printProgressBarP, PrintInProgress());
	//	mgr.Show(printingField, PrintInProgress());

		// Don't enable the time left field when we start printing, instead this will get enabled when we receive a suitable message
		if (!PrintInProgress())
		{
			mgr.Show(timeLeftField, false);
		}

		const unsigned int stat = (unsigned int)GetStatus();
		statusField->SetValue((stat < NumStatusStrings) ? strings->statusValues[stat] : "unknown status");
		pStatusField->SetValue((stat < NumStatusStrings) ? strings->statusValues[stat] : "unknown status");
	}

	// Set the percentage of print completed
	void SetPrintProgressPercent(unsigned int percent)
	{
		printProgressBar->SetPercent((uint8_t)percent);
		printProgressBarP->SetPercent((uint8_t)percent);
	}

	// Update the geometry or the number of axes
	void UpdateGeometry(unsigned int p_numAxes, bool p_isDelta)
	{
		if (p_numAxes != numVisibleAxes || p_isDelta != isDelta)
		{
			numVisibleAxes = p_numAxes;
			isDelta = p_isDelta;
			FileManager::RefreshMacrosList();
			numDisplayedAxes = 0;
			size_t slotP = 0;
			for (size_t i = 0; i < MaxTotalAxes; ++i)
			{
				Axis& axis = axes[i];
				axis.slot = MaxTotalAxes;
				axis.slotP = MaxTotalAxes;
				if (!axis.visible)
				{
					continue;
				}
				const char * letter = axis.letter;
				if (numDisplayedAxes < MaxDisplayableAxes)
				{
					axis.slot = numDisplayedAxes;
					++numDisplayedAxes;

					// Update axis letter everywhere we display it
					const uint8_t slot = axis.slot;
					controlTabAxisPos	[slot]->SetLabel(letter);
					moveAxisRows		[slot]->SetValue(letter);
					printTabAxisPos		[slot]->SetLabel(letter);
					movePopupAxisPos	[slot]->SetLabel(letter);
					homeButtons			[slot]->SetText(letter);

					// Update axis letter to be sent for homing commands
					homeButtons[slot]->SetEvent(homeButtons[slot]->GetEvent(), letter);

					mgr.Show(homeButtons[slot], !isDelta);
					ShowAxis(slot, true);
				}
				if (IsVisibleAxisPendant(letter))
				{
					axis.slotP = slotP;
					jobTabAxisPos[slotP]->SetLabel(letter);
					ShowAxisP(slotP, slotP < MaxTotalAxes);
					++slotP;
				}
			}
			// Hide axes possibly shown before
			for (size_t i = numDisplayedAxes; i < MaxDisplayableAxes; ++i)
			{
				mgr.Show(homeButtons[i], false);
				ShowAxis(i, false);
			}
			for (size_t i = slotP; i < MaxDisplayableAxesP; ++i)
			{
				ShowAxisP(i, false);
			}
		}
	}

	// Update the homed status of the specified axis. If the axis is -1 then it represents the "all homed" status.
	void UpdateHomedStatus(size_t axis, bool isHomed)
	{
		if (axis >= MaxTotalAxes)
		{
			return;
		}
		axes[axis].homed = isHomed;
		const size_t slot = axes[axis].slot;
		const size_t slotP = axes[axis].slotP;
		if (slot < MaxDisplayableAxes)
		{
			homeButtons[slot]->SetColours(colours->buttonTextColour, (isHomed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);
		}
		if (slotP < MaxDisplayableAxesP)
		{
			pHomeButtons[slotP]->SetColours(colours->buttonTextColour, (isHomed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);
		}

		bool allHomed = true;
		for (size_t i = 0; i < MaxTotalAxes; ++i)
		{
			if (axes[i].visible && !axes[i].homed)
			{
				allHomed = false;
				break;
			}
		}
		if (allHomed != allAxesHomed)
		{
			allAxesHomed = allHomed;
			homeAllButton->SetColours(colours->buttonTextColour, (allAxesHomed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);
			pHomeAllButton->SetColours(colours->buttonTextColour, (allAxesHomed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);
		}
	}

	// UIpdate the Z probe text
	void UpdateZProbe(const char data[])
	{
		zprobeBuf.copy(data);
		zProbe->SetChanged();
	}

	// Update the machine name
	void UpdateMachineName(const char data[])
	{
		machineName.copy(data);
		nameField->SetChanged();
		pNameField->SetChanged();
	}

	// Update the fan RPM
	void UpdateFanPercent(int rpm)
	{
		UpdateField(fanSpeed, rpm);
	}

	void UpdateTemperature(size_t index, int ival, IntegerButton** fields, IntegerButton* fieldPJog, IntegerButton** fieldsPJob)
	{
		const int heaterSlot = GetHeaterSlot(index);
		if (heaterSlot >= 0)
		{
			UpdateField(fields[heaterSlot], ival);
		}

		auto tool = GetToolForHeater(index);
		if (tool != nullptr)
		{
			if (tool->index == currentTool)
			{
				UpdateField(fieldPJog, ival);
			}
			if (tool->slotP < MaxPendantTools)
			{
				UpdateField(fieldsPJob[tool->slotP], ival);
			}
		}
		else if ((int)index == bedHeater.heater && bedHeater.slotP < MaxPendantTools)
		{
			UpdateField(activeTempsPJob[bedHeater.slotP], ival);
		}
		else if ((int)index == chamberHeater.heater && chamberHeater.slotP < MaxPendantTools)
		{
			UpdateField(activeTempsPJob[chamberHeater.slotP], ival);
		}
	}

	// Update an active temperature
	void UpdateActiveTemperature(size_t index, int ival)
	{
		UpdateTemperature(index, ival, activeTemps, activeTempPJog, activeTempsPJob);
	}

	// Update a standby temperature
	void UpdateStandbyTemperature(size_t index, int ival)
	{
		UpdateTemperature(index, ival, standbyTemps, standbyTempPJog, standbyTempsPJob);
	}

	// Update an extrusion factor
	void UpdateExtrusionFactor(size_t index, int ival)
	{
		auto tool = GetToolForExtruder(index);
		if (tool != nullptr)
		{
			if (tool->slot < MaxHeaters)
			{
				UpdateField(extrusionFactors[tool->slot], ival);
			}
			if (tool->index == currentTool)
			{
				UpdateField(pExtruderPercentButton, ival);
			}
		}
	}

	// Update the print speed factor
	void UpdateSpeedPercent(int ival)
	{
		UpdateField(spd, ival);
		UpdateField(pFeedrateButton, ival);
	}

	// Process a new message box alert, clearing any existing one
	void ProcessAlert(const Alert& alert)
	{
		if (isLandscape)
		{
			alertPopup->Set(alert.title.c_str(), alert.text.c_str(), alert.mode, alert.controls);
			mgr.SetPopup(alertPopup, AutoPlace, AutoPlace);
		}
		else
		{
			alertPopupP->Set(alert.title.c_str(), alert.text.c_str(), alert.mode, alert.controls);
			mgr.SetPopupP(alertPopupP, AutoPlace, AutoPlace);
		}
		RestoreBrightness();
		alertMode = alert.mode;
		displayingResponse = false;
		whenAlertReceived = SystemTick::GetTickCount();
		alertTicks = (alertMode < 2) ? (uint32_t)(alert.timeout * 1000.0) : 0;
	}

	// Process a command to clear a message box alert
	void ClearAlert()
	{
		if (alertMode >= 0)
		{
			alertTicks = 0;
			mgr.ClearPopup(true, isLandscape ? static_cast<PopupWindow*>(alertPopup) : static_cast<PopupWindow*>(alertPopupP));
			alertMode = -1;
		}
	}

	// Clear a message box alert or response. Called when the user presses the close button or the alert or response times out.
	void ClearAlertOrResponse()
	{
		if (alertMode >= 0 || displayingResponse)
		{
			alertTicks = 0;
			mgr.ClearPopup(true, isLandscape ? static_cast<PopupWindow*>(alertPopup) : static_cast<PopupWindow*>(alertPopupP));
			alertMode = -1;
			displayingResponse = false;
		}
	}

	bool CanDimDisplay()
	{
		return alertMode < 2;
	}

	void ProcessSimpleAlert(const char* array text)
	{
		if (alertMode < 2)												// if the current alert doesn't require acknowledgement
		{
			if (isLandscape)
			{
				alertPopup->Set(strings->message, text, 1, 0);
				mgr.SetPopup(alertPopup, AutoPlace, AutoPlace);
			}
			else
			{
				alertPopupP->Set(strings->message, text, 1, 0);
				mgr.SetPopupP(alertPopupP, AutoPlace, AutoPlace);
			}
			alertMode = 1;												// a simple alert is like a mode 1 alert without a title
			displayingResponse = false;
			whenAlertReceived = SystemTick::GetTickCount();
			alertTicks = 0;												// no timeout
		}
		RestoreBrightness();
	}

	// Process a new response. This is treated like a simple alert except that it times out and isn't cleared by a "clear alert" command from the host.
	void NewResponseReceived(const char* array text)
	{
		const bool isErrorMessage = stringStartsWith(text, "Error");
		if (   alertMode < 2											// if the current alert doesn't require acknowledgement
			&& !(currentTab == tabSetup || currentTab == tabMsg)		// don't show on setup tab or on console tab
			&& (isErrorMessage || infoTimeout != 0)
		   )
		{
			if (isLandscape)
			{
				alertPopup->Set(strings->response, text, 1, 0);
				mgr.SetPopup(alertPopup, AutoPlace, AutoPlace);
			}
			else
			{
				alertPopupP->Set(strings->response, text, 1, 0);
				mgr.SetPopupP(alertPopupP, AutoPlace, AutoPlace);
			}
			alertMode = -1;												// make sure that a call to ClearAlert doesn't clear us
			displayingResponse = true;
			whenAlertReceived = SystemTick::GetTickCount();
			alertTicks = isErrorMessage ? 0 : infoTimeout * SystemTick::TicksPerSecond;				// time out if it isn't an error message
		}
	}

	// This is called when the user selects a new file from a list of SD card files
	void FileSelected(const char * array null fileName)
	{
		fpNameField->SetValue(fileName);
		// Clear out the old field values, they relate to the previous file we looked at until we process the response
		fpSizeField->SetValue(0);						// would be better to make it blank
		fpHeightField->SetValue(0.0);					// would be better to make it blank
		fpLayerHeightField->SetValue(0.0);				// would be better to make it blank
		fpFilamentField->SetValue(0);					// would be better to make it blank
		generatedByText.clear();
		fpGeneratedByField->SetChanged();
		lastModifiedText.clear();
		fpLastModifiedField->SetChanged();
		printTimeText.clear();
		fpPrintTimeField->SetChanged();
	}

	// This is called when the "generated by" file information has been received
	void UpdateFileGeneratedByText(const char data[])
	{
		generatedByText.copy(data);
		fpGeneratedByField->SetChanged();
	}

	// This is called when the "last modified" file information has been received
	void UpdateFileLastModifiedText(const char data[])
	{
		lastModifiedText.copy(data);
		lastModifiedText.replace('T', ' ');
		fpLastModifiedField->SetChanged();
	}

	// This is called when the "last modified" file information has been received
	void UpdatePrintTimeText(uint32_t seconds, bool isSimulated)
	{
		bool update = false;
		if (isSimulated)
		{
			printTimeText.clear();					// prefer simulated to estimated print time
			fpPrintTimeField->SetLabel(strings->simulatedPrintTime);
			update = true;
		}
		else if (printTimeText.isEmpty())
		{
			fpPrintTimeField->SetLabel(strings->estimatedPrintTime);
			update = true;
		}
		if (update)
		{
			unsigned int minutes = (seconds + 50)/60;
			printTimeText.printf("%dh %02dm", minutes / 60, minutes % 60);
			fpPrintTimeField->SetChanged();
		}
	}

	// This is called when the object height information for the file has been received
	void UpdateFileObjectHeight(float f)
	{
		fpHeightField->SetValue(f);
	}

	// This is called when the layer height information for the file has been received
	void UpdateFileLayerHeight(float f)
	{
		fpLayerHeightField->SetValue(f);
	}

	// This is called when the size of the file has been received
	void UpdateFileSize(int size)
	{
		fpSizeField->SetValue(size);
	}

	// This is called when the filament needed by the file has been received
	void UpdateFileFilament(int len)
	{
		fpFilamentField->SetValue(len);
	}

	// Return true if we are displaying file information
	bool IsDisplayingFileInfo()
	{
		return currentFile != nullptr;
	}

	// This is called when the host firmware changes
	void FirmwareFeaturesChanged(FirmwareFeatures newFeatures)
	{
		// Some firmwares don't support tool standby temperatures
		for (size_t i = 0; i < MaxHeaters; ++i)
		{
			mgr.Show(standbyTemps[i], (newFeatures & noStandbyTemps) == 0);
		}
	}

	static void DoEmergencyStop()
	{
		// We send M112 for the benefit of old firmware, and F0 0F (an invalid UTF8 sequence) for new firmware
		SerialIo::SendString("M112 ;" "\xF0" "\x0F" "\n");
		TouchBeep();											// needed when we are called from ProcessTouchOutsidePopup
		Delay(1000);
		SerialIo::SendString("M999\n");
		Reconnect();
	}

	void SendExtrusion(const bool retract, const char *amount, const char *rate) {
		SerialIo::SendString("M120\nM83\nG1 E");
		if (retract) {
			SerialIo::SendChar('-');
		}
		SerialIo::SendString(amount);
		SerialIo::SendString(" F");
		SerialIo::SendString(rate);
		SerialIo::SendString("\nM121\n");
	}

	// Process a touch event
	void ProcessTouch(ButtonPress bp)
	{
		if (bp.IsValid())
		{
			ButtonBase *f = bp.GetButton();
			currentButton = bp;
			mgr.Press(bp, true);
			Event ev = (Event)(f->GetEvent());
			switch(ev)
			{
			case evEmergencyStop:
				DoEmergencyStop();
				break;

			case evDefaultRoot:
			case evPendantRoot:
				ChangeRoot(f);
				break;

			case evTabControl:
			case evTabPrint:
			case evTabMsg:
			case evTabSetup:
			case evTabJog:
			case evTabOffset:
			case evTabJob:
				if (ChangePage(f))
				{
					currentButton.Clear();						// keep the button highlighted after it is released
				}
				break;

			case evPJogAxis:
				mgr.Press(currentJogAxis, false);
				mgr.Press(bp, true);
				currentJogAxis = bp;
				currentButton.Clear();						// stop it being released by the timer
				break;

			case evPJogAmount:
				mgr.Press(currentJogAmount, false);
				mgr.Press(bp, true);
				currentJogAmount = bp;
				currentButton.Clear();						// stop it being released by the timer
				break;

			case evMeasureZ:
				PopupAreYouSure(ev, strings->confirmMeasureZ);
				break;

			case evToolSelect:
			{
				const int head = bp.GetIParam();
				if (currentTool == head)					// if tool is selected
				{
					SerialIo::SendString("T-1\n");
				}
				else
				{
					SerialIo::SendChar('T');
					SerialIo::SendInt(head);
					SerialIo::SendChar('\n');
				}
				break;
			}

			case evProbeWorkpiece:
			{
				SerialIo::SendString("M98 P\"workpieceprobe_");
				SerialIo::SendString(bp.GetSParam());
				SerialIo::SendString(".g\"\n");
				break;
			}

			case evFindCenterOfCavity:
			{
				SerialIo::SendString("M98 P\"tcalibrate.g\"\n");
				break;
			}

			case evTouchoff:
			{
				SerialIo::SendString("M98 P\"touchoff_");
				SerialIo::SendString(bp.GetSParam());
				SerialIo::SendString(".g\"\n");
				break;
			}

			case evSetToolOffset:
			{
				// No tool or probe
				if (currentTool < 0 || currentTool == 10)
				{
					break;
				}
				String<120> cmd;
				cmd.printf("G10 L1 P%d %s M500 P10\n",
						currentTool,
						bp.GetIParam() == 0
							? "X{move.axes[0].userPosition} Y{move.axes[1].userPosition}"
							: "Z{move.axes[2].userPosition}");
				SerialIo::SendString(cmd.c_str());
				break;
			}

			case evZeroAxisInWCS:
			{
				String<120> cmd;
				cmd.printf("G10 L20 P{move.workspaceNumber} %s\n", bp.GetSParam());
				break;
			}

			case evAdjustActiveTemp:
			case evAdjustStandbyTemp:
				if (static_cast<IntegerButton*>(f)->GetValue() < 0)
				{
					static_cast<IntegerButton*>(f)->SetValue(0);
				}
				Adjusting(bp);
				if (isLandscape)
				{
				mgr.SetPopup(setTempPopup, AutoPlace, popupY);
				}
				else
				{
					const PixelNumber xPos = f->GetMinX();
					// Try to fit it below but fall back to above if not enough space
					const PixelNumber yPos =
							(f->GetMaxY() + margin + setTempPopupEncoder->GetHeight()) <= DisplayYP
								? f->GetMaxY() + margin
								: f->GetMinY() - margin - setTempPopupEncoder->GetHeight();
					mgr.SetPopupP(setTempPopupEncoder, xPos, yPos);
				}
				break;

			case evAdjustActiveRPM:
				Adjusting(bp);
				if (isLandscape)
				{
					mgr.SetPopup(setTempPopup, AutoPlace, popupY);
				}
				else
				{
					oldIntValue = static_cast<IntegerButton*>(bp.GetButton())->GetValue();
					const PixelNumber xPos = f->GetMinX();
					// Try to fit it below but fall back to above if not enough space
					const PixelNumber yPos =
							(f->GetMaxY() + margin + setTempPopupEncoder->GetHeight()) <= DisplayYP
								? f->GetMaxY() + margin
								: f->GetMinY() - margin - setTempPopupEncoder->GetHeight();
					mgr.SetPopupP(setTempPopupEncoder, xPos, yPos);
				}
				break;

			case evAdjustSpeed:
			case evExtrusionFactor:
			case evAdjustFan:
			case evPAdjustExtrusionPercent:
				oldIntValue = static_cast<IntegerButton*>(bp.GetButton())->GetValue();
				Adjusting(bp);
				if (isLandscape)
				{
					mgr.SetPopup(setTempPopup, AutoPlace, popupY);
				}
				else
				{
					const PixelNumber xPos = f->GetMinX();
					// Try to fit it below but fall back to above if not enough space
					const PixelNumber yPos =
							(f->GetMaxY() + margin + setTempPopupEncoder->GetHeight()) <= DisplayYP
								? f->GetMaxY() + margin
								: f->GetMinY() - margin - setTempPopupEncoder->GetHeight();
					mgr.SetPopupP(setTempPopupEncoder, xPos, yPos);
				}
				break;

			case evSetInt:
				if (fieldBeingAdjusted.IsValid())
				{
					int val = static_cast<const IntegerButton*>(fieldBeingAdjusted.GetButton())->GetValue();
					switch(fieldBeingAdjusted.GetEvent())
					{
					case evAdjustActiveTemp:
						{
							int heater = fieldBeingAdjusted.GetIParam();
							if (heater == bedHeater.heater || heater == chamberHeater.heater)
							{
								SerialIo::SendString("M14");
								SerialIo::SendInt(heater == bedHeater.heater ? 0 : 1);
								SerialIo::SendString(" P");
								SerialIo::SendInt(((heater == bedHeater.heater) ? bedHeater : chamberHeater).index);
								SerialIo::SendString(" S");
								SerialIo::SendInt(val);
								SerialIo::SendChar('\n');
							}
							else
							{
								SerialIo::SendString(((GetFirmwareFeatures() & noG10Temps) == 0) ? "G10 P" : "M104 T");
								SerialIo::SendInt(heater);
								SerialIo::SendString(" S");
								SerialIo::SendInt(val);
								SerialIo::SendChar('\n');
							}
						}
						break;

					case evAdjustStandbyTemp:
						{
							int heater = fieldBeingAdjusted.GetIParam();
							if (heater == bedHeater.heater || heater == chamberHeater.heater)
							{
								SerialIo::SendString("M14");
								SerialIo::SendInt(heater == bedHeater.heater ? 0 : 1);
								SerialIo::SendString(" P");
								SerialIo::SendInt(((heater == bedHeater.heater) ? bedHeater : chamberHeater).index);
								SerialIo::SendString(" R");
								SerialIo::SendInt(val);
								SerialIo::SendChar('\n');
							}
							else
 							{
								SerialIo::SendString("G10 P");
								SerialIo::SendInt(heater);
								SerialIo::SendString(" R");
								SerialIo::SendInt(val);
								SerialIo::SendChar('\n');
							}
						}
						break;

					case evAdjustActiveRPM:
						{
							auto spindle = GetOrCreateSpindle(fieldBeingAdjusted.GetIParam());
							if (val == 0)
							{
								SerialIo::SendString("M5 P");
								SerialIo::SendInt(spindle->index);
								SerialIo::SendChar('\n');
							}
							else
							{
								SerialIo::SendChar('M');
								SerialIo::SendInt(val < 0 ? 4 : 3);
								SerialIo::SendString(" P");
								SerialIo::SendInt(spindle->index);
								SerialIo::SendString(" S");
								SerialIo::SendInt(abs(val));
								SerialIo::SendChar('\n');
							}
						}
						break;

					case evExtrusionFactor:
						{
							const int extruder = fieldBeingAdjusted.GetIParam();
							SerialIo::SendString("M221 D");
							SerialIo::SendInt(extruder);
							SerialIo::SendString(" S");
							SerialIo::SendInt(val);
							SerialIo::SendChar('\n');
						}
						break;

					case evAdjustFan:
						SerialIo::SendString("M106 S");
						SerialIo::SendInt((256 * val)/100);
						SerialIo::SendChar('\n');
						break;

					default:
						{
							const char* null cmd = fieldBeingAdjusted.GetSParam();
							if (cmd != nullptr)
							{
								SerialIo::SendString(cmd);
								SerialIo::SendInt(val);
								SerialIo::SendChar('\n');
							}
						}
						break;
					}
					mgr.ClearPopup();
					StopAdjusting();
				}
				break;

			case evAdjustInt:
				if (fieldBeingAdjusted.IsValid())
				{
					IntegerButton *ib = static_cast<IntegerButton*>(fieldBeingAdjusted.GetButton());
					int newValue = ib->GetValue() + bp.GetIParam();
					switch(fieldBeingAdjusted.GetEvent())
					{
					case evAdjustActiveTemp:
					case evAdjustStandbyTemp:
						newValue = constrain<int>(newValue, 0, 1600);		// some users want to print at high temperatures
						break;

					case evAdjustFan:
						newValue = constrain<int>(newValue, 0, 100);
						break;

					case evAdjustActiveRPM:
						{
							auto spindle = GetOrCreateSpindle(fieldBeingAdjusted.GetIParam());
							newValue = constrain<int>(newValue, -spindle->max, spindle->max);
						}
						break;

					default:
						break;
					}
					ib->SetValue(newValue);
					ShortenTouchDelay();
				}
				break;

			case evMovePopup:
				mgr.SetPopup(movePopup, AutoPlace, AutoPlace);
				break;

			case evMoveX:
			case evMoveY:
			case evMoveZ:
			case evMoveU:
			case evMoveV:
			case evMoveW:
				{
					const uint8_t axis = ev - evMoveX;
					const char c = (axis < 3) ? 'X' + axis : ('U' - 3) + axis;
					SerialIo::SendString("G91\nG1 ");
					SerialIo::SendChar(c);
					SerialIo::SendString(bp.GetSParam());
					SerialIo::SendString(" F6000\nG90\n");
				}
				break;

			case evExtrudePopup:
				if (isLandscape)
				{
				mgr.SetPopup(extrudePopup, AutoPlace, AutoPlace);
				}
				else {

					mgr.SetPopupP(extrudePopupP, AutoPlace, AutoPlace);
				}
				break;

			case evExtrudeAmount:
				mgr.Press(currentExtrudeAmountPress, false);
				mgr.Press(bp, true);
				currentExtrudeAmountPress = bp;
				currentButton.Clear();						// stop it being released by the timer
				break;

			case evExtrudeRate:
				mgr.Press(currentExtrudeRatePress, false);
				mgr.Press(bp, true);
				currentExtrudeRatePress = bp;
				currentButton.Clear();						// stop it being released by the timer
				break;

			case evExtrudeAmountP:
				mgr.Press(currentExtrudeAmountPressP, false);
				mgr.Press(bp, true);
				currentExtrudeAmountPressP = bp;
				currentButton.Clear();						// stop it being released by the timer
				break;

			case evExtrudeRateP:
				mgr.Press(currentExtrudeRatePressP, false);
				mgr.Press(bp, true);
				currentExtrudeRatePressP = bp;
				currentButton.Clear();						// stop it being released by the timer
				break;

			case evExtrude:
			case evRetract:
				{
					const ButtonPress amount = isLandscape ? currentExtrudeAmountPress : currentExtrudeAmountPressP;
					const ButtonPress rate = isLandscape ? currentExtrudeRatePress : currentExtrudeRatePressP;
					if (amount.IsValid() && rate.IsValid())
					{
						SendExtrusion(ev == evRetract, amount.GetSParam(), rate.GetSParam());
					}
				}
				break;

			case evBabyStepPopup:
				mgr.SetPopup(babystepPopup, AutoPlace, AutoPlace);
				break;

			case evBabyStepAmount:
				SerialIo::SendString("M290 Z");
				SerialIo::SendString(bp.GetSParam());
				SerialIo::SendChar('\n');
				break;

			case evListFiles:
				FileManager::DisplayFilesList();
				break;

			case evListMacros:
				FileManager::DisplayMacrosList();
				break;

			case evCalTouch:
				CalibrateTouch();
				break;

			case evFactoryReset:
				PopupAreYouSure(ev, strings->confirmFactoryReset);
				break;

			case evSelectBed:
				{
					const auto slot = bedHeater.slot;
					if (slot >= MaxHeaters)
					{
						break;
					}
					if (heaterStatus[slot] == HeaterStatus::active)			// if bed is active
					{
						SerialIo::SendString("M144\n");
					}
					else
					{
						SerialIo::SendString("M140 P");
						SerialIo::SendInt(bedHeater.index);
						SerialIo::SendString(" S");
						SerialIo::SendInt(activeTemps[slot]->GetValue());
						SerialIo::SendChar('\n');
					}
				}
				break;

			case evSelectChamber:
				{
					const auto slot = chamberHeater.slot;
					if (slot >= MaxHeaters)
					{
						break;
					}
					SerialIo::SendString("M141 P");
					SerialIo::SendInt(chamberHeater.index);
					SerialIo::SendString(" S");
					if (heaterStatus[slot] == HeaterStatus::active)			// if chamber is active
					{
						SerialIo::SendString(" S-273.15\n");
					}
					else
					{
						SerialIo::SendInt(activeTemps[slot]->GetValue());
					}
					SerialIo::SendChar('\n');
				}
				break;

			case evSelectHead:
					{
						int head = bp.GetIParam();
						// pressing a evSeelctHead button in the middle of active printing is almost always accidental (and fatal to the print job)
						if (GetStatus() != PrinterStatus::printing && GetStatus() != PrinterStatus::simulating)
						{
							if (head == currentTool)		// if head is active
							{
								SerialIo::SendString("T-1\n");
							}
							else
							{
								SerialIo::SendChar('T');
								SerialIo::SendInt(head);
								SerialIo::SendChar('\n');
							}
						}
					}
				break;

			case evFile:
				{
					const char * array fileName = bp.GetSParam();
					if (fileName != nullptr)
					{
						if (fileName[0] == '*')
						{
							// It's a directory
							FileManager::RequestFilesSubdir(fileName + 1);
							//??? need to pop up a "wait" box here
						}
						else
						{
							// It's a regular file
							currentFile = fileName;
							SerialIo::SendString(((GetFirmwareFeatures() & noM20M36) == 0) ? "M36 " : "M408 S36 P");			// ask for the file info
							SerialIo::SendFilename(CondStripDrive(FileManager::GetFilesDir()), currentFile);
							SerialIo::SendChar('\n');
							FileSelected(currentFile);
							mgr.SetPopup(fileDetailPopup, AutoPlace, AutoPlace);
						}
					}
					else
					{
						ErrorBeep();
					}
				}
				break;

			case evFilesUp:
				FileManager::RequestFilesParentDir();
				break;

			case evMacrosUp:
				FileManager::RequestMacrosParentDir();
				break;

			case evMacro:
			case evMacroControlPage:
				{
					const char *fileName = bp.GetSParam();
					if (fileName != nullptr)
					{
						if (fileName[0] == '*')		// if it's a directory
						{
							FileManager::RequestMacrosSubdir(fileName + 1);
							//??? need to pop up a "wait" box here
						}
						else
						{
							SerialIo::SendString("M98 P");
							const char * array const dir = (ev == evMacroControlPage) ? FileManager::GetMacrosRootDir() : FileManager::GetMacrosDir();
							SerialIo::SendFilename(CondStripDrive(dir), fileName);
							SerialIo::SendChar('\n');
						}
					}
					else
					{
						ErrorBeep();
					}
				}
				break;

			case evPrintFile:
			case evSimulateFile:
				mgr.ClearPopup();			// clear the file info popup
				mgr.ClearPopup();			// clear the file list popup
				if (currentFile != nullptr)
				{
					SerialIo::SendString((ev == evSimulateFile) ? "M37 P" : "M32 ");
					SerialIo::SendFilename(CondStripDrive(StripPrefix(FileManager::GetFilesDir())), currentFile);
					SerialIo::SendChar('\n');
					PrintingFilenameChanged(currentFile);
					currentFile = nullptr;							// allow the file list to be updated
					CurrentButtonReleased();
					PrintStarted();
				}
				break;

			case evCancel:
				eventToConfirm = evNull;
				currentFile = nullptr;
				CurrentButtonReleased();
				PopupCancelled();
				mgr.ClearPopup();
				break;

			case evDeleteFile:
				CurrentButtonReleased();
				PopupAreYouSure(ev, strings->confirmFileDelete);
				break;

			case evSendCommand:
			case evPausePrint:
			case evResumePrint:
			case evReset:
				SerialIo::SendString(bp.GetSParam());
				SerialIo::SendChar('\n');
				break;

			case evHomeAxis:
				SerialIo::SendString("G28 ");
				SerialIo::SendString(bp.GetSParam());
				SerialIo::SendString("0\n");
				break;

			case evScrollFiles:
				FileManager::ScrollFiles(bp.GetIParam() * NumFileRows);
				ShortenTouchDelay();
				break;

			case evScrollMacros:
				FileManager::ScrollMacros(bp.GetIParam() * (isLandscape ? NumMacroRows : NumMacroRowsP));
				ShortenTouchDelay();
				break;

			case evChangeCard:
				(void)FileManager::NextCard();
				break;

			case evKeyboard:
				ShowKeyboard();
				break;

			case evInvertX:
				MirrorDisplay();
				CalibrateTouch();
				break;

			case evInvertY:
				InvertDisplay();
				CalibrateTouch();
				break;

			case evSetBaudRate:
				Adjusting(bp);
				mgr.SetPopup(baudPopup, AutoPlace, popupY);
				break;

			case evAdjustBaudRate:
				{
					const int rate = bp.GetIParam();
					SetBaudRate(rate);
					baudRateButton->SetValue(rate);
				}
				CurrentButtonReleased();
				mgr.ClearPopup();
				StopAdjusting();
				break;

			case evSetVolume:
				Adjusting(bp);
				mgr.SetPopup(volumePopup, AutoPlace, popupY);
				break;

			case evSetInfoTimeout:
				Adjusting(bp);
				mgr.SetPopup(infoTimeoutPopup, AutoPlace, popupY);
				break;

			case evSetColours:
				if (coloursPopup != nullptr)
				{
					Adjusting(bp);
					mgr.SetPopup(coloursPopup, AutoPlace, popupY);
				}
				break;

			case evBrighter:
			case evDimmer:
				ChangeBrightness(ev == evBrighter);
				ShortenTouchDelay();
				break;

			case evAdjustVolume:
				{
					const int newVolume = bp.GetIParam();
					SetVolume(newVolume);
					volumeButton->SetValue(newVolume);
				}
				TouchBeep();									// give audible feedback of the touch at the new volume level
				break;

			case evAdjustInfoTimeout:
				{
					infoTimeout = bp.GetIParam();
					SetInfoTimeout(infoTimeout);
					infoTimeoutButton->SetValue(infoTimeout);
				}
				TouchBeep();									// give audible feedback of the touch at the new volume level
				break;

			case evAdjustColours:
				{
					const uint8_t newColours = (uint8_t)bp.GetIParam();
					if (SetColourScheme(newColours))
					{
						SaveSettings();
						Restart();
					}
				}
				mgr.ClearPopup();
				break;

			case evSetLanguage:
				Adjusting(bp);
				mgr.SetPopup(languagePopup, AutoPlace, popupY);
				break;

			case evAdjustLanguage:
				{
					const uint8_t newLanguage = (uint8_t)bp.GetIParam();
					if (SetLanguage(newLanguage))
					{
						SaveSettings();
						Restart();
					}
				}
				mgr.ClearPopup();
				break;

			case evSetDimmingType:
				ChangeDisplayDimmerType();
				dimmingTypeButton->SetText(strings->displayDimmingNames[(unsigned int)GetDisplayDimmerType()]);
				break;

			case evYes:
				CurrentButtonReleased();
				mgr.ClearPopup();								// clear the yes/no popup
				switch (eventToConfirm)
				{
				case evFactoryReset:
					FactoryReset();
					break;

				case evDeleteFile:
					if (currentFile != nullptr)
					{
						mgr.ClearPopup();						// clear the file info popup
						SerialIo::SendString("M30 ");
						SerialIo::SendFilename(CondStripDrive(StripPrefix(FileManager::GetFilesDir())), currentFile);
						SerialIo::SendChar('\n');
						FileManager::RefreshFilesList();
						currentFile = nullptr;
					}
					break;

				case evMeasureZ:
					SerialIo::SendString(measureZButton->GetSParam(0));
					SerialIo::SendChar('\n');
					break;

				default:
					break;
				}
				eventToConfirm = evNull;
				currentFile = nullptr;
				break;

			case evKey:
				if (userCommandBuffers[currentUserCommandBuffer].add((char)bp.GetIParam()))
				{
					userCommandField->SetChanged();
				}
				break;

			case evShift:
				{
					size_t rowOffset;
					if (keyboardShifted)
					{
						bp.GetButton()->Press(false, 0);
						rowOffset = 0;
					}
					else
					{
						rowOffset = 4;
					}
					for (size_t i = 0; i < 4; ++i)
					{
						keyboardRows[i]->ChangeText(currentKeyboard[i + rowOffset]);
					}
				}
				keyboardShifted = !keyboardShifted;
				currentButton.Clear();				// make the key sticky
				break;

			case evBackspace:
				if (!userCommandBuffers[currentUserCommandBuffer].isEmpty())
				{
					userCommandBuffers[currentUserCommandBuffer].erase(userCommandBuffers[currentUserCommandBuffer].size() - 1);
					userCommandField->SetChanged();
					ShortenTouchDelay();
				}
				break;

			case evUp:
				currentHistoryBuffer = (currentHistoryBuffer + numUserCommandBuffers - 1) % numUserCommandBuffers;
				if (currentHistoryBuffer == currentUserCommandBuffer)
				{
					userCommandBuffers[currentUserCommandBuffer].clear();
				}
				else
				{
					userCommandBuffers[currentUserCommandBuffer].copy(userCommandBuffers[currentHistoryBuffer]);
				}
				userCommandField->SetChanged();
				break;

			case evDown:
				currentHistoryBuffer = (currentHistoryBuffer + 1) % numUserCommandBuffers;
				if (currentHistoryBuffer == currentUserCommandBuffer)
				{
					userCommandBuffers[currentUserCommandBuffer].clear();
				}
				else
				{
					userCommandBuffers[currentUserCommandBuffer].copy(userCommandBuffers[currentHistoryBuffer]);
				}
				userCommandField->SetChanged();
				break;

			case evSendKeyboardCommand:
				if (userCommandBuffers[currentUserCommandBuffer].size() != 0)
				{
					SerialIo::SendString(userCommandBuffers[currentUserCommandBuffer].c_str());
					SerialIo::SendChar('\n');

					// Add the command to the history if it was different frmo the previous command
					size_t prevBuffer = (currentUserCommandBuffer + numUserCommandBuffers - 1) % numUserCommandBuffers;
					if (strcmp(userCommandBuffers[currentUserCommandBuffer].c_str(), userCommandBuffers[prevBuffer].c_str()) != 0)
					{
						currentUserCommandBuffer = (currentUserCommandBuffer + 1) % numUserCommandBuffers;
					}
					currentHistoryBuffer = currentUserCommandBuffer;
					userCommandBuffers[currentUserCommandBuffer].clear();
					userCommandField->SetLabel(userCommandBuffers[currentUserCommandBuffer].c_str());
				}
				break;

			case evCloseAlert:
				SerialIo::SendString(bp.GetSParam());
				SerialIo::SendChar('\n');
				ClearAlertOrResponse();
				break;

			default:
				break;
			}
		}
	}

	// Process a touch event outside the popup on the field being adjusted
	void ProcessTouchOutsidePopup(ButtonPress bp)
	{
		if (bp == fieldBeingAdjusted)
		{
			DelayTouchLong();	// by default, ignore further touches for a long time
			TouchBeep();
			switch(fieldBeingAdjusted.GetEvent())
			{
			case evAdjustSpeed:
			case evExtrusionFactor:
			case evAdjustFan:
			case evPAdjustExtrusionPercent:
				static_cast<IntegerButton*>(fieldBeingAdjusted.GetButton())->SetValue(oldIntValue);
				mgr.ClearPopup();
				StopAdjusting();
				break;

			case evAdjustActiveTemp:
			case evAdjustStandbyTemp:
			case evAdjustActiveRPM:
			case evSetBaudRate:
			case evSetVolume:
			case evSetInfoTimeout:
			case evSetColours:
				if (fieldBeingAdjusted.GetEvent() == evAdjustActiveRPM && !isLandscape)
				{
					static_cast<IntegerButton*>(fieldBeingAdjusted.GetButton())->SetValue(oldIntValue);
				}
				mgr.ClearPopup();
				StopAdjusting();
				break;

			case evSetLanguage:
				mgr.ClearPopup();
				StopAdjusting();
				break;
			}
		}
		else
		{
			switch(bp.GetEvent())
			{
			case evEmergencyStop:
				mgr.Press(bp, true);
				DoEmergencyStop();
				mgr.Press(bp, false);
				break;

			case evTabControl:
			case evTabPrint:
			case evTabMsg:
			case evTabSetup:
			case evTabJog:
			case evTabOffset:
			case evTabJob:
				StopAdjusting();
				DelayTouchLong();	// by default, ignore further touches for a long time
				TouchBeep();
				{
					ButtonBase *btn = bp.GetButton();
					if (ChangePage(btn))
					{
						currentButton.Clear();						// keep the button highlighted after it is released
					}
				}
				break;

			case evSetBaudRate:
			case evSetVolume:
			case evSetInfoTimeout:
			case evSetColours:
			case evSetLanguage:
			case evCalTouch:
			case evInvertX:
			case evInvertY:
			case evFactoryReset:
				// On the Setup tab, we allow any other button to be pressed to exit the current popup
				StopAdjusting();
				DelayTouchLong();	// by default, ignore further touches for a long time
				TouchBeep();
				mgr.ClearPopup();
				ProcessTouch(bp);
				break;

			default:
				break;
			}
		}
	}

	// This is called when a button press times out
	void OnButtonPressTimeout()
	{
		if (currentButton.IsValid())
		{
			CurrentButtonReleased();
		}
	}

	void DisplayFilesOrMacrosList(bool filesNotMacros, int cardNumber, unsigned int numVolumes)
	{
		if (filesNotMacros)
		{
			filePopupTitleField->SetValue(cardNumber);
			changeCardButton->Show(numVolumes > 1);
		}
		if (isLandscape)
		{
			mgr.SetPopup((filesNotMacros) ? fileListPopup : macrosPopup, AutoPlace, AutoPlace);
		}
		else
		{
			mgr.SetPopupP((filesNotMacros) ? fileListPopup : macrosPopupP, AutoPlace, AutoPlace);
		}
	}

	void FileListLoaded(bool filesNotMacros, int errCode)
	{
		FileListButtons& buttons = (filesNotMacros) ? filesListButtons : macrosListButtons;
		if (errCode == 0)
		{
			mgr.Show(buttons.errorField, false);
			// Portrait mode
			mgr.Show(macrosListButtonsP.errorField, false);
		}
		else
		{
			buttons.errorField->SetValue(errCode);
			mgr.Show(buttons.errorField, true);
			// Portrait mode
			macrosListButtonsP.errorField->SetValue(errCode);
			mgr.Show(macrosListButtonsP.errorField, true);
		}
	}

	void EnableFileNavButtons(bool filesNotMacros, bool scrollEarlier, bool scrollLater, bool parentDir)
	{
		FileListButtons& buttons = (filesNotMacros) ? filesListButtons : macrosListButtons;
		mgr.Show(buttons.scrollLeftButton, scrollEarlier);
		mgr.Show(buttons.scrollRightButton, scrollLater);
		mgr.Show(buttons.folderUpButton, parentDir);

		// Portrait mode
		mgr.Show(macrosListButtonsP.scrollLeftButton, scrollEarlier);
		mgr.Show(macrosListButtonsP.scrollRightButton, scrollLater);
		mgr.Show(macrosListButtonsP.folderUpButton, parentDir);
	}

	// Update the specified button in the file or macro buttons list. If 'text' is nullptr then hide the button, else display it.
	void UpdateFileButton(bool filesNotMacros, unsigned int buttonIndex, const char * array null text, const char * array null param)
	{
		if (buttonIndex < ((filesNotMacros) ? NumDisplayedFiles : NumDisplayedMacros))
		{
			TextButton * const f = ((filesNotMacros) ? filenameButtons : macroButtons)[buttonIndex];
			f->SetText(text);
			f->SetEvent((text == nullptr) ? evNull : (filesNotMacros) ? evFile : evMacro, param);
			mgr.Show(f, text != nullptr);
		}

		// Portrait mode
		if (buttonIndex < NumDisplayedMacrosP)
		{
			TextButton * const fp = macroButtonsP[buttonIndex];
			fp->SetText(text);
			fp->SetEvent((text == nullptr) ? evNull : evMacro, param);
			mgr.Show(fp, text != nullptr);
		}
	}

	// Update the specified button in the macro short list. If 'fileName' is nullptr then hide the button, else display it.
	// Return true if this should be called again for the next button.
	bool UpdateMacroShortList(unsigned int buttonIndex, const char * array null fileName)
	{
		if (buttonIndex >= ARRAY_SIZE(controlPageMacroButtons) || controlPageMacroButtons[buttonIndex] == nullptr || numToolColsUsed == 0 || numToolColsUsed >= MaxHeaters - 2)
		{
			return false;
		}

		String<controlPageMacroTextLength>& str = controlPageMacroText[buttonIndex];
		str.clear();
		const bool isFile = (fileName != nullptr);
		if (isFile)
		{
			str.copy(fileName);
		}
		TextButton * const f = controlPageMacroButtons[buttonIndex];
		f->SetText(SkipDigitsAndUnderscore(str.c_str()));
		f->SetEvent((isFile) ? evMacroControlPage : evNull, str.c_str());
		mgr.Show(f, isFile);
		return true;
	}

	unsigned int GetNumScrolledFiles(bool filesNotMacros)
	{
		if (isLandscape)
		{
			return (filesNotMacros) ? NumFileRows : NumMacroRows;
		}
		else {
			return NumMacroRowsP;
		}
	}

	void AdjustControlPageMacroButtons()
	{
		const unsigned int n = numToolColsUsed;

		if (n != numHeaterAndToolColumns)
		{
			numHeaterAndToolColumns = n;

			// Adjust the width of the control page macro buttons, or hide them completely if insufficient room
			PixelNumber controlPageMacroButtonsColumn = (PixelNumber)(((tempButtonWidth + fieldSpacing) * n) + bedColumn + fieldSpacing);
			PixelNumber controlPageMacroButtonsWidth = (PixelNumber)((controlPageMacroButtonsColumn >= DisplayX - margin) ? 0 : DisplayX - margin - controlPageMacroButtonsColumn);
			if (controlPageMacroButtonsWidth > maxControlPageMacroButtonsWidth)
			{
				controlPageMacroButtonsColumn += controlPageMacroButtonsWidth - maxControlPageMacroButtonsWidth;
				controlPageMacroButtonsWidth = maxControlPageMacroButtonsWidth;
			}

			bool showControlPageMacroButtons = controlPageMacroButtonsWidth >= minControlPageMacroButtonsWidth;

			for (TextButton *& b : controlPageMacroButtons)
			{
				if (showControlPageMacroButtons)
				{
					b->SetPositionAndWidth(controlPageMacroButtonsColumn, controlPageMacroButtonsWidth);
				}
				mgr.Show(b, showControlPageMacroButtons);
			}

			if (currentTab == tabControl)
			{
				mgr.Refresh(true);
			}
		}
	}

	void AllToolsSeen()
	{
		size_t slot = 0;
		size_t slotP = 0;
		bedHeater.slot = MaxHeaters;
		bedHeater.slotP = MaxPendantTools;
		if (bedHeater.heater > -1)
		{
			bedHeater.slot = slot;
			mgr.Show(toolButtons[slot], true);
			mgr.Show(currentTemps[slot], true);
			mgr.Show(activeTemps[slot], true);
			mgr.Show(standbyTemps[slot], true);
			toolButtons[slot]->SetEvent(evSelectBed, bedHeater.index);
			toolButtons[slot]->SetIcon(IconBed);
			toolButtons[slot]->SetIntVal(bedHeater.index);	// Remove the line below if we want to show the bed number
			toolButtons[slot]->SetPrintText(false);
			activeTemps[slot]->SetEvent(evAdjustActiveTemp, bedHeater.heater);
			standbyTemps[slot]->SetEvent(standbyTemps[slot]->GetEvent(), bedHeater.heater);
			++slot;

			bedHeater.slotP = slotP;
			mgr.Show(toolButtonsPJob[slotP], true);
			mgr.Show(currentTempsPJob[slotP], true);
			mgr.Show(activeTempsPJob[slotP], true);
			mgr.Show(standbyTempsPJob[slotP], true);
			toolButtonsPJob[slotP]->SetEvent(evSelectChamber, bedHeater.index);
			toolButtonsPJob[slotP]->SetIcon(IconChamber);
			toolButtonsPJob[slotP]->SetIntVal(bedHeater.index);	// Remove the line below if we want to show the bed number
			toolButtonsPJob[slotP]->SetPrintText(false);
			activeTempsPJob[slotP]->SetEvent(evAdjustActiveTemp, bedHeater.heater);
			standbyTempsPJob[slotP]->SetEvent(standbyTempsPJob[slotP]->GetEvent(), bedHeater.heater);
			++slotP;
		}
		for (auto tool = tools; tool != nullptr; tool = tool->next)
		{
			tool->slot = slot;
			const bool hasHeater = tool->heater > -1;
			const bool hasSpindle = tool->spindle != nullptr;
			if (slot < MaxHeaters)
			{
				toolButtons[slot]->SetEvent(evSelectHead, tool->index);
				toolButtons[slot]->SetIntVal(tool->index);
				toolButtons[slot]->SetPrintText(true);
				toolButtons[slot]->SetIcon(hasSpindle ? IconSpindle : IconNozzle);
				mgr.Show(toolButtons[slot], true);
				mgr.Show(currentTemps[slot], hasHeater || hasSpindle);
				mgr.Show(activeTemps[slot], hasHeater || hasSpindle);
				mgr.Show(standbyTemps[slot], hasHeater);
				mgr.Show(extrusionFactors[slot], hasHeater);

				// Set tool number for change event
				const event_t evForActive = hasHeater ? evAdjustActiveTemp : hasSpindle ? evAdjustActiveRPM : evNull;
				const int evActiveParam = hasSpindle ? tool->spindle->index : tool->index;

				activeTemps[slot]->SetEvent(evForActive, evActiveParam);
				standbyTemps[slot]->SetEvent(standbyTemps[slot]->GetEvent(), tool->index);
				++slot;
			}
			tool->slotP = slotP;
			if (slotP < MaxPendantTools)
			{
				toolSelectButtons[slotP]->SetEvent(evSelectHead, tool->index);
				toolSelectButtons[slotP]->SetIntVal(tool->index);
				toolSelectButtons[slotP]->SetPrintText(true);

				toolButtonsPJob[slotP]->SetEvent(evSelectHead, tool->index);
				toolButtonsPJob[slotP]->SetIntVal(tool->index);
				toolButtonsPJob[slotP]->SetPrintText(true);

				// Special handling for T10 (Probe)
				if (tool->index == 10)
				{
					toolSelectButtons[slotP]->SetText(strings->probe);
					toolSelectButtons[slotP]->SetDrawIcon(false);

					toolButtonsPJob[slotP]->SetText(strings->probe);
					toolButtonsPJob[slotP]->SetDrawIcon(false);
				}
				else
				{
					toolSelectButtons[slotP]->SetText(nullptr);
					toolSelectButtons[slotP]->SetIcon(hasSpindle ? IconSpindle : IconNozzle);
					toolSelectButtons[slotP]->SetDrawIcon(true);

					toolButtonsPJob[slotP]->SetText(nullptr);
					toolButtonsPJob[slotP]->SetIcon(hasSpindle ? IconSpindle : IconNozzle);
					toolButtonsPJob[slotP]->SetDrawIcon(true);
				}
				mgr.Show(toolSelectButtons[slotP], true);

				// FIXME: this leads to empty rows if a tool has neither heater nor spindle
				mgr.Show(toolButtonsPJob[slotP], hasHeater || hasSpindle);
				mgr.Show(currentTempsPJob[slotP], hasHeater || hasSpindle);
				mgr.Show(activeTempsPJob[slotP], hasHeater || hasSpindle);
				mgr.Show(standbyTempsPJob[slotP], hasHeater);

				// Set tool/spindle number for change event
				const event_t evForActive = hasHeater ? evAdjustActiveTemp : hasSpindle ? evAdjustActiveRPM : evNull;
				const int evActiveParam = hasSpindle ? tool->spindle->index : tool->index;

				activeTempsPJob[slotP]->SetEvent(evForActive, evActiveParam);
				standbyTempsPJob[slotP]->SetEvent(standbyTempPJog->GetEvent(), tool->index);
				++slotP;
			}
		}
		chamberHeater.slot = MaxHeaters;
		if (slot < MaxHeaters && chamberHeater.heater > -1)
		{
			chamberHeater.slot = slot;
			mgr.Show(toolButtons[slot], true);
			mgr.Show(currentTemps[slot], true);
			mgr.Show(activeTemps[slot], true);
			mgr.Show(standbyTemps[slot], true);
			toolButtons[slot]->SetEvent(evSelectChamber, chamberHeater.index);
			toolButtons[slot]->SetIcon(IconChamber);
			toolButtons[slot]->SetIntVal(chamberHeater.index);	// Remove the line below if we want to show the chamber number
			toolButtons[slot]->SetPrintText(false);
			activeTemps[slot]->SetEvent(evAdjustActiveTemp, chamberHeater.heater);
			standbyTemps[slot]->SetEvent(standbyTemps[slot]->GetEvent(), chamberHeater.heater);
			++slot;
		}
		chamberHeater.slotP = MaxPendantTools;
		if (slotP < MaxPendantTools && chamberHeater.heater > -1)
		{
			chamberHeater.slotP = slotP;
			mgr.Show(toolButtonsPJob[slotP], true);
			mgr.Show(currentTempsPJob[slotP], true);
			mgr.Show(activeTempsPJob[slotP], true);
			mgr.Show(standbyTempsPJob[slotP], true);
			toolButtonsPJob[slotP]->SetEvent(evSelectChamber, chamberHeater.index);
			toolButtonsPJob[slotP]->SetIcon(IconChamber);
			toolButtonsPJob[slotP]->SetIntVal(chamberHeater.index);	// Remove the line below if we want to show the chamber number
			toolButtonsPJob[slotP]->SetPrintText(false);
			activeTempsPJob[slotP]->SetEvent(evAdjustActiveTemp, chamberHeater.heater);
			standbyTempsPJob[slotP]->SetEvent(standbyTempsPJob[slotP]->GetEvent(), chamberHeater.heater);
			++slotP;
		}
		numToolColsUsed = slot;
		for (size_t i = slot; i < MaxHeaters; ++i)
		{
			mgr.Show(toolButtons[i], false);
			mgr.Show(currentTemps[i], false);
			mgr.Show(activeTemps[i], false);
			mgr.Show(standbyTemps[i], false);
		}
		for (size_t i = slotP; i < MaxPendantTools; ++i)
		{
			mgr.Show(toolSelectButtons[i], false);
			mgr.Show(toolButtonsPJob[i], false);
			mgr.Show(currentTempsPJob[i], false);
			mgr.Show(activeTempsPJob[i], false);
			mgr.Show(standbyTempsPJob[i], false);
		}
		AdjustControlPageMacroButtons();
	}

	template<class T>
	void Remove(T*& start, size_t index, bool allFollowing)
	{
		// TODO: Unify if-else-block
		if (start != nullptr && start->index == index)
		{
			if (allFollowing)
			{
				for (auto toDelete = start; toDelete != nullptr; toDelete = start->next)
				{
					start = toDelete->next;
					delete toDelete;
				}
			}
			else
			{
				auto toDelete = start;
				start = toDelete->next;
				delete toDelete;
			}
		}
		else
		{
			for (auto s = start; s != nullptr; s = s->next)
			{
				if (s->next == nullptr || s->next->index != index)
				{
					continue;
				}
				if (allFollowing)
				{
					for (auto toDelete = s->next; toDelete != nullptr; toDelete = s->next)
					{
						s->next = toDelete->next;
						delete toDelete;
					}
				}
				else
				{
					auto toDelete = s->next;
					s->next = toDelete->next;
					delete toDelete;
				}
			}
		}
	}

	void RemoveSpindle(size_t index, bool allFollowing)
	{
		Remove(spindles, index, allFollowing);
	}

	void RemoveTool(size_t index, bool allFollowing)
	{
		Remove(tools, index, allFollowing);
	}

	void SetSpindleActive(size_t index, float active)
	{
		auto spindle = GetOrCreateSpindle(index, true);
		spindle->active = active;
		if (spindle->tool > -1)
		{
			auto tool = GetOrCreateTool(spindle->tool);
			if (tool != nullptr)
			{
				if (tool->slot < MaxHeaters)
				{
					UpdateField(activeTemps[tool->slot], (int)active);
				}
				if (tool->slotP < MaxPendantTools)
				{
					UpdateField(activeTempsPJob[tool->slotP], (int)active);
				}
				if (tool->index == currentTool)
				{
					UpdateField(activeTempPJog, (int)active);
				}
			}
		}
	}

	void SetSpindleCurrent(size_t index, float current)
	{
		auto spindle = GetOrCreateSpindle(index, true);
		if (spindle->tool > -1)
		{
			auto tool = GetOrCreateTool(spindle->tool);
			if (tool != nullptr)
			{
				if (tool->slot < MaxHeaters)
				{
					currentTemps[tool->slot]->SetValue(current);
				}
				if (tool->slotP < MaxPendantTools)
				{
					currentTempsPJob[tool->slotP]->SetValue(current);
				}
				if (tool->index == currentTool)
				{
					currentTempPJog->SetValue(current);
					UpdateField(pSpindleRPMButton, current);
				}
			}
		}
	}

	void SetSpindleMax(size_t index, float max)
	{
		GetOrCreateSpindle(index, true)->max = max;
	}

	void UpdateToolStatus(size_t toolIndex, ToolStatus status)
	{
		auto tool = GetOrCreateTool(toolIndex);
		tool->status = status;
		Colour c = (status == ToolStatus::standby) ? colours->standbyBackColour
					: (status == ToolStatus::active) ? colours->activeBackColour
					: colours->buttonImageBackColour;
		if (tool->slot < MaxHeaters)
		{
			toolButtons[tool->slot]->SetColours(colours->buttonTextColour, c);
		}
		if (tool->slotP < MaxPendantTools)
		{
			toolSelectButtons[tool->slot]->SetColours(colours->buttonTextColour, c);
			toolButtonsPJob[tool->slot]->SetColours(colours->buttonTextColour, c);
		}
	}

	void SetToolExtruder(size_t toolIndex, int8_t extruder)
	{
		GetOrCreateTool(toolIndex, true)->extruder = extruder;
	}

	void SetToolHeater(size_t toolIndex, int8_t heater)
	{
		GetOrCreateTool(toolIndex, true)->heater = heater;
	}

	void SetToolOffset(size_t toolIndex, size_t axisIndex, float offset)
	{
		if (axisIndex < MaxTotalAxes)
		{
			GetOrCreateTool(toolIndex, true)->offsets[axisIndex] = offset;
		}
	}

	void SetSpindleTool(int8_t spindle, int8_t toolIndex)
	{
		auto sp = GetOrCreateSpindle(spindle, true);
		sp->tool = toolIndex;
		if (toolIndex == -1)
		{
			for (auto tool = tools; tool != nullptr; tool = tool->next)
			{
				if (tool->spindle == sp)
				{
					tool->spindle = nullptr;
				}
			}
		}
		else
		{
			GetOrCreateTool(toolIndex, true)->spindle = sp;
		}
	}

	void SetBabystepOffset(size_t index, float f)
	{
		if (index < MaxTotalAxes)
		{
			axes[index].babystep = f;
			if (axes[index].letter[0] == 'Z')
			{
				babystepOffsetField->SetValue(f);
			}
		}
	}

	void SetAxisLetter(size_t index, char l)
	{
		if (index < MaxTotalAxes)
		{
			axes[index].letter[0] = l;
		}
	}

	void SetAxisVisible(size_t index, bool v)
	{
		if (index < MaxTotalAxes)
		{
			axes[index].visible = v;
		}
	}

	void SetAxisWorkplaceOffset(size_t axisIndex, size_t workplaceIndex, float offset)
	{
		if (axisIndex < MaxTotalAxes && workplaceIndex < Workplaces::MaxTotalWorkplaces)
		{
			axes[axisIndex].workplaceOffsets[workplaceIndex] = offset;
		}
	}

	void SetBedOrChamberHeater(int8_t heaterNumber, uint8_t heaterIndex, bool bed)
	{
		BedOrChamber* h = (bed) ? &bedHeater : &chamberHeater;
		h->heater = heaterNumber;
		h->index = heaterIndex;
	}
}

#endif

// End
