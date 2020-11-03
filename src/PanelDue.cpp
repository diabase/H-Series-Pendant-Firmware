// TFT panel controller to run on SAM3S2B/SAM3S4B/SAM4S4B
// Coding rules:
//
// 1. Must compile with no g++ warnings, when all warning are enabled.
// 2. Dynamic memory allocation using 'new' is permitted during the initialization phase only. No use of 'new' anywhere else,
//    and no use of 'delete', 'malloc' or 'free' anywhere.
// 3. No pure virtual functions. This is because in release builds, having pure virtual functions causes huge amounts of the C++ library to be linked in
//    (possibly because it wants to print a message if a pure virtual function is called).

#include "PanelDue.hpp"
#include "asf.h"

#include <cstring>
#include <cctype>
#include <cmath>

#include "Hardware/Mem.hpp"
#include "Display.hpp"
#include "Hardware/UTFT.hpp"
#include "Hardware/UTouch.hpp"
#include "Hardware/SerialIo.hpp"
#include "Hardware/Buzzer.hpp"
#include "Hardware/SysTick.hpp"
#include "Hardware/Reset.hpp"
#include "Library/Misc.hpp"
#include "General/SafeStrtod.h"

#if SAM4S
#include "flash_efc.h"
#else
#include "Hardware/FlashStorage.hpp"
#endif

#include "Configuration.hpp"
#include "UserInterfaceConstants.hpp"
#include "FileManager.hpp"
#include "RequestTimer.hpp"
#include "MessageLog.hpp"
#include "Events.hpp"
#include "HeaterStatus.hpp"
#include "PrinterStatus.hpp"
#include "ToolStatus.hpp"
#include "UserInterface.hpp"
#include "ObjectModel.hpp"

#ifdef OEM
# if DISPLAY_X == 800
#  include "OemSplashScreen_800_480.hpp"
# else
#  include "OemSplashScreen_480_272.hpp"
# endif
#endif

extern uint16_t _esplash[];							// defined in linker script

#define DEBUG	(0)

// Controlling constants
const uint32_t defaultPrinterPollInterval = 1000;			// poll interval in milliseconds
constexpr uint32_t slowPrinterPollInterval = defaultPrinterPollInterval * 4;	// poll interval in milliseconds when screensaver active
const uint32_t printerResponseInterval = 700;		// shortest time after a response that we send another poll (gives printer time to catch up)
const uint32_t printerPollTimeout = 4000;			// poll timeout in milliseconds
const uint32_t FileInfoRequestTimeout = 8000;		// file info request timeout in milliseconds
const uint32_t touchBeepLength = 20;				// beep length in ms
const uint32_t touchBeepFrequency = 4500;			// beep frequency in Hz. Resonant frequency of the piezo sounder is 4.5kHz.
const uint32_t errorBeepLength = 100;
const uint32_t errorBeepFrequency = 2250;
const uint32_t longTouchDelay = 250;				// how long we ignore new touches for after pressing Set
const uint32_t shortTouchDelay = 100;				// how long we ignore new touches while pressing up/down, to get a reasonable repeat rate

struct HostFirmwareType
{
	const char* _ecv_array const name;
	const FirmwareFeatures features;
};

const HostFirmwareType firmwareTypes[] =
{
	{ "RepRapFirmware", quoteFilenames },
	{ "Smoothie", 	noGcodesFolder | noStandbyTemps | noG10Temps | noDriveNumber | noM20M36 },
	{ "Repetier", 	noGcodesFolder | noStandbyTemps | noG10Temps },
	{ "Marlin",		noGcodesFolder | noStandbyTemps | noG10Temps }
};

// Variables

#if SAM4S

// Version 3.0
UTFT lcd(DISPLAY_CONTROLLER, 15, 14, 0, 39);
UTouch touch(8, 7, 6, 4, 1);

#elif SAM3S

// Version 1.0, 1.1, 2.0
UTFT lcd(DISPLAY_CONTROLLER, 16, 17, 18, 19);
UTouch touch(23, 24, 22, 21, 20);

#endif

// These defines control which detailed M409 requests will be sent
// If one of the fields in the disabled ones need to be fetched the
// corresponding define has to be set to (1)
#define FETCH_BOARDS		(1)
#define FETCH_DIRECTORIES	(0)
#define FETCH_FANS			(0)
#define FETCH_HEAT			(1)
#define FETCH_INPUTS		(0)
#define FETCH_JOB			(1)
#define FETCH_MOVE			(1)
#define FETCH_NETWORK		(1)
#define FETCH_SCANNER		(0)
#define FETCH_SENSORS		(0)
#define FETCH_SPINDLES		(1)
#define FETCH_STATE			(1)
#define FETCH_TOOLS			(1)
#define FETCH_VOLUMES		(1)

MainWindow mgr;

static uint32_t lastTouchTime;
static uint32_t ignoreTouchTime;
static uint32_t lastPollTime;
static uint32_t lastResponseTime = 0;
static uint32_t lastActionTime = 0;							// the last time anything significant happened
static FirmwareFeatures firmwareFeatures = 0;
static bool isDimmed = false;								// true if we have dimmed the display
static bool screensaverActive = false;						// true if screensaver is active
static bool isDelta = false;
static size_t numAxes = MIN_AXES;
static int32_t beepFrequency = 0, beepLength = 0;
static uint32_t messageSeq = 0;
static uint32_t newMessageSeq = 0;
static uint32_t fileSize = 0;
static uint8_t visibleAxesCounted = 0;
static int8_t lastSpindle = -1;
static int8_t lastTool = -1;
static uint8_t mountedVolumesCounted = 0;
static uint32_t remoteUpTime = 0;
static bool initialized = false;
static uint32_t printerPollInterval = defaultPrinterPollInterval;

const ColourScheme *colours = &colourSchemes[0];

Alert currentAlert;
uint32_t lastAlertSeq = 0;

struct FlashData
{
	// The magic value should be changed whenever the layout of the NVRAM changes
	// We now use a different magic value for each display size, to force the "touch the spot" screen to be displayed when you change the display size
	static const uint32_t magicVal = 0x3AB63A40 + DISPLAY_TYPE;
	static const uint32_t muggleVal = 0xFFFFFFFF;

	uint32_t magic;
	uint32_t baudRate;
	uint16_t xmin;
	uint16_t xmax;
	uint16_t ymin;
	uint16_t ymax;
	DisplayOrientation lcdOrientation;
	DisplayOrientation touchOrientation;
	uint8_t touchVolume;
	uint8_t language;
	uint8_t colourScheme;
	uint8_t brightness;
	DisplayDimmerType displayDimmerType;
	uint8_t infoTimeout;
	uint32_t screensaverTimeout;
	uint8_t babystepAmountIndex;
	uint16_t feedrate;
	uint8_t padding[1];
	char dummy;								// must be at a multiple of 4 bytes from the start because flash is read/written in whole dwords

	FlashData() : magic(muggleVal) { }
	bool operator==(const FlashData& other);
	bool operator!=(const FlashData& other) { return !operator==(other); }
	bool IsValid() const;
	void SetInvalid() { magic = muggleVal; }
	void SetDefaults();
	void Load();
	void Save() const;
};

#if SAM4S
// FlashData must fit in user signature flash area
static_assert(sizeof(FlashData) <= 512, "Flash data too large");
#else
// FlashData must fit in the area we have reserved
static_assert(sizeof(FlashData) <= FLASH_DATA_LENGTH, "Flash data too large");
#endif

FlashData nvData, savedNvData;

static PrinterStatus status = PrinterStatus::connecting;

enum ReceivedDataEvent
{
	rcvUnknown = 0,

	// Keys for push messages
	rcvPushMessage,
	rcvPushResponse,
	rcvPushSeq,
	rcvPushBeepDuration,
	rcvPushBeepFrequency,

	// Keys for M20 response
	rcvM20Dir,
	rcvM20Err,
	rcvM20Files,

	// Keys for M36 respons
	rcvM36Filament,
	rcvM36Filename,
	rcvM36GeneratedBy,
	rcvM36Height,
	rcvM36LastModified,
	rcvM36LayerHeight,
	rcvM36PrintTime,
	rcvM36SimulatedTime,
	rcvM36Size,

	// Keys for M409 response
	rcvKey,
	rcvFlags,
	rcvResult,

	// Available keys
	rcvOMKeyNoKey,
	rcvOMKeyBoards,
	rcvOMKeyDirectories,
	rcvOMKeyFans,
	rcvOMKeyHeat,
	rcvOMKeyInputs,
	rcvOMKeyJob,
	rcvOMKeyLimits,
	rcvOMKeyMove,
	rcvOMKeyNetwork,
	rcvOMKeyReply,
	rcvOMKeyScanner,
	rcvOMKeySensors,
	rcvOMKeySeqs,
	rcvOMKeySpindles,
	rcvOMKeyState,
	rcvOMKeyTools,
	rcvOMKeyVolumes,

	// Keys in "live" response
	rcvLiveFansActualValue,

	rcvLiveHeatHeatersActive,
	rcvLiveHeatHeatersCurrent,
	rcvLiveHeatHeatersStandby,
	rcvLiveHeatHeatersState,

	rcvLiveJobFilePosition,
	rcvLiveJobTimesLeftFilament,
	rcvLiveJobTimesLeftFile,
	rcvLiveJobTimesLeftLayer,

	rcvMoveAxesHomed,
	rcvLiveMoveAxesMachinePosition,
	rcvLiveMoveAxesUserPosition,

	rcvLiveSensorsProbeValue,

	rcvLiveSeqsBoards,
	rcvLiveSeqsDirectories,
	rcvLiveSeqsFans,
	rcvLiveSeqsHeat,
	rcvLiveSeqsInputs,
	rcvLiveSeqsJob,
	rcvLiveSeqsMove,
	rcvLiveSeqsNetwork,
	rcvLiveSeqsReply,
	rcvLiveSeqsScanner,
	rcvLiveSeqsSensors,
	rcvLiveSeqsSpindles,
	rcvLiveSeqsState,
	rcvLiveSeqsTools,
	rcvLiveSeqsVolumes,

	rcvLiveSpindlesCurrent,

	rcvLiveStateCurrentTool,
	rcvLiveStateStatus,
	rcvLiveStateUptime,

	// Keys for boards response
	rcvBoardsFirmwareName,

	// Keys for heat response
	rcvHeatBedHeaters,
	rcvHeatChamberHeaters,

	// Keys for job response
	rcvJobFileFilename,
	rcvJobFileSize,

	// Keys for move response
	rcvMoveAxesBabystep,
	rcvMoveAxesLetter,
	rcvMoveAxesVisible,
	rcvMoveAxesWorkplaceOffsets,
	rcvMoveExtrudersFactor,
	rcvMoveKinematicsName,
	rcvMoveSpeedFactor,
	rcvMoveWorkplaceNumber,

	// Keys for network response
	rcvNetworkName,
	rcvNetworkInterfacesActualIP,

	// Keys for spindles respons
	rcvSpindlesActive,
	rcvSpindlesMax,
	rcvSpindlesTool,

	// Keys from state response
	rcvStateMessageBox,
	rcvStateMessageBoxAxisControls,
	rcvStateMessageBoxMessage,
	rcvStateMessageBoxMode,
	rcvStateMessageBoxSeq,
	rcvStateMessageBoxTimeout,
	rcvStateMessageBoxTitle,

	// Keys from tools response
	rcvToolsExtruders,
	rcvToolsHeaters,
	rcvToolsOffsets,
	rcvToolsNumber,
	rcvLiveToolsState,

	// Keys for volumes response
	rcvVolumesMounted,
};

struct FieldTableEntry
{
	ReceivedDataEvent rde;
	const char* varName;
};

// The following tables will be sorted once on startup so entries can be better grouped for code maintenance
// A '^' character indicates the position of an _ecv_array index, and a ':' character indicates the start of a sub-field name
static FieldTableEntry fieldTable[] =
{
	// M409 common fields
	{ rcvKey, 							"key" },

	// M409 F"d99f" response
	{ rcvLiveFansActualValue,			"fans^:actualValue" },

	{ rcvLiveHeatHeatersActive,			"heat:heaters^:active" },
	{ rcvLiveHeatHeatersCurrent,		"heat:heaters^:current" },
	{ rcvLiveHeatHeatersStandby,		"heat:heaters^:standby" },
	{ rcvLiveHeatHeatersState,			"heat:heaters^:state" },

	{ rcvLiveJobFilePosition,			"job:filePosition" },
	{ rcvLiveJobTimesLeftFilament,		"job:timesLeft:filament" },
	{ rcvLiveJobTimesLeftFile,			"job:timesLeft:file" },
	{ rcvLiveJobTimesLeftLayer,			"job:timesLeft:layer" },

	{ rcvLiveMoveAxesMachinePosition,	"move:axes^:machinePosition" },
	{ rcvLiveMoveAxesUserPosition,		"move:axes^:userPosition" },

	{ rcvLiveSensorsProbeValue,			"sensors:probes^:value^" },

	{ rcvLiveSeqsBoards,				"seqs:boards" },
	{ rcvLiveSeqsDirectories,			"seqs:directories" },
	{ rcvLiveSeqsFans,					"seqs:fans" },
	{ rcvLiveSeqsHeat,					"seqs:heat" },
	{ rcvLiveSeqsInputs,				"seqs:inputs" },
	{ rcvLiveSeqsJob,					"seqs:job" },
	{ rcvLiveSeqsMove,					"seqs:move" },
	{ rcvLiveSeqsNetwork,				"seqs:network" },
	{ rcvLiveSeqsReply,					"seqs:reply" },
	{ rcvLiveSeqsScanner,				"seqs:scanner" },
	{ rcvLiveSeqsSensors,				"seqs:sensors" },
	{ rcvLiveSeqsSpindles,				"seqs:spindles" },
	{ rcvLiveSeqsState,					"seqs:state" },
	{ rcvLiveSeqsTools,					"seqs:tools" },
	{ rcvLiveSeqsVolumes,				"seqs:volumes" },

	{ rcvLiveSpindlesCurrent,			"spindles^:current" },

	{ rcvLiveStateCurrentTool,			"state:currentTool" },
	{ rcvLiveStateStatus,				"state:status" },
	{ rcvLiveStateUptime,				"state:upTime" },

	{ rcvLiveToolsState, 				"tools^:state" },

	// M409 K"boards" response
	{ rcvBoardsFirmwareName, 			"boards^:firmwareName" },

	// M409 K"heat" response
	{ rcvHeatBedHeaters,				"heat:bedHeaters^" },
	{ rcvHeatChamberHeaters,			"heat:chamberHeaters^" },

	// M409 K"job" response
	{ rcvJobFileFilename, 				"job:file:fileName" },
	{ rcvJobFileSize, 					"job:file:size" },

	// M409 K"move" response
	{ rcvMoveAxesBabystep, 				"move:axes^:babystep" },
	{ rcvMoveAxesHomed,					"move:axes^:homed" },
	{ rcvMoveAxesLetter,	 			"move:axes^:letter" },
	{ rcvMoveAxesVisible, 				"move:axes^:visible" },
	{ rcvMoveAxesWorkplaceOffsets, 		"move:axes^:workplaceOffsets^" },
	{ rcvMoveExtrudersFactor, 			"move:extruders^:factor" },
	{ rcvMoveKinematicsName, 			"move:kinematics:name" },
	{ rcvMoveSpeedFactor, 				"move:speedFactor" },
	{ rcvMoveWorkplaceNumber, 			"move:workplaceNumber" },

	// M409 K"network" response
	{ rcvNetworkName, 					"network:name" },
	{ rcvNetworkInterfacesActualIP,		"network:interfaces^:actualIP" },

	// M409 K"spindles" response
	{ rcvSpindlesActive, 				"spindles^:active" },
	{ rcvSpindlesMax, 					"spindles^:max" },
	{ rcvSpindlesTool, 					"spindles^:tool" },

	// M409 K"state" response
	{ rcvStateMessageBox,				"state:messageBox" },
	{ rcvStateMessageBoxAxisControls,	"state:messageBox:axisControls" },
	{ rcvStateMessageBoxMessage,		"state:messageBox:message" },
	{ rcvStateMessageBoxMode,			"state:messageBox:mode" },
	{ rcvStateMessageBoxSeq,			"state:messageBox:seq" },
	{ rcvStateMessageBoxTimeout,		"state:messageBox:timeout" },
	{ rcvStateMessageBoxTitle,			"state:messageBox:title" },

	// M409 K"tools" response
	{ rcvToolsExtruders,				"tools^:extruders^" },
	{ rcvToolsHeaters,					"tools^:heaters^" },
	{ rcvToolsNumber, 					"tools^:number" },
	{ rcvToolsOffsets, 					"tools^:offsets^" },

	// M409 K"volumes" response
	{ rcvVolumesMounted, 				"volumes^:mounted" },

	// M20 response
	{ rcvM20Dir,						"dir" },
	{ rcvM20Err,						"err" },
	{ rcvM20Files,						"files^" },

	// M36 response
	{ rcvM36Filament,					"filament^" },
	{ rcvM36Filename,					"fileName" },
	{ rcvM36GeneratedBy,				"generatedBy" },
	{ rcvM36Height,						"height" },
	{ rcvM36LastModified,				"lastModified" },
	{ rcvM36LayerHeight,				"layerHeight" },
	{ rcvM36PrintTime,					"printTime" },
	{ rcvM36SimulatedTime,				"simulatedTime" },
	{ rcvM36Size,						"size" },

	// Push messages
	{ rcvPushMessage,					"message" },
	{ rcvPushResponse,					"resp" },
	{ rcvPushSeq,						"seq" },
	{ rcvPushBeepDuration,				"beep_length" },
	{ rcvPushBeepFrequency,				"beep_freq" },
};

// This table must be kept in case-insensitive alphabetical order of the search string.
const FieldTableEntry keyResponseTypeTable[] =
{
	{ rcvOMKeyNoKey, 			"" },
	{ rcvOMKeyBoards,			"boards" },
	{ rcvOMKeyDirectories,		"directories" },
	{ rcvOMKeyFans,				"fans" },
	{ rcvOMKeyHeat,				"heat" },
	{ rcvOMKeyInputs,			"inputs" },
	{ rcvOMKeyJob,				"job" },
	{ rcvOMKeyLimits,			"limits" },
	{ rcvOMKeyMove,				"move" },
	{ rcvOMKeyNetwork,			"network" },
	{ rcvOMKeyReply,			"reply" },
	{ rcvOMKeyScanner,			"scanner" },
	{ rcvOMKeySensors,			"sensors" },
	{ rcvOMKeySeqs,				"seqs" },
	{ rcvOMKeySpindles,			"spindles" },
	{ rcvOMKeyState,			"state" },
	{ rcvOMKeyTools,			"tools" },
	{ rcvOMKeyVolumes,			"volumes" },
};


static ReceivedDataEvent currentResponseType = rcvUnknown;

struct Seqs
{
	uint16_t boards;
	uint16_t directories;
	uint16_t fans;
	uint16_t heat;
	uint16_t inputs;
	uint16_t job;
	uint16_t move;
	uint16_t network;
	uint16_t scanner;
	uint16_t sensors;
	uint16_t spindles;
	uint16_t state;
	uint16_t tools;
	uint16_t volumes;

	uint16_t updateBoards	: 1,
		 updateDirectories	: 1,
		 updateFans			: 1,
		 updateHeat			: 1,
		 updateInputs		: 1,
		 updateJob			: 1,
		 updateMove			: 1,
		 updateNetwork		: 1,
		 updateScanner		: 1,
		 updateSensors		: 1,
		 updateSpindles		: 1,
		 updateState		: 1,
		 updateTools		: 1,
		 updateVolumes		: 1;
} seqs;


void resetSeqs()
{
	seqs.boards 			=
	seqs.directories		=
	seqs.fans 				=
	seqs.heat 				=
	seqs.inputs 			=
	seqs.job 				=
	seqs.move 				=
	seqs.network 			=
	seqs.scanner 			=
	seqs.sensors 			=
	seqs.spindles 			=
	seqs.state 				=
	seqs.tools 				=
	seqs.volumes 			= (uint16_t)((1 << 16)-1);

	seqs.updateBoards		=
	seqs.updateDirectories	=
	seqs.updateFans 		=
	seqs.updateHeat 		=
	seqs.updateInputs 		=
	seqs.updateJob 			=
	seqs.updateMove 		=
	seqs.updateNetwork 		=
	seqs.updateScanner 		=
	seqs.updateSensors 		=
	seqs.updateSpindles 	=
	seqs.updateState 		=
	seqs.updateTools 		=
	seqs.updateVolumes 		= false;

	Reconnect();
}

struct OMRequestParams {
	const char * _ecv_array const key;
	const char * _ecv_array const flags = "v";
};

static const OMRequestParams noKeyParams =			{""};
static const OMRequestParams boardsParams =			{"boards"};
static const OMRequestParams directoriesParams =	{"directories"};
static const OMRequestParams fansParams =			{"fans"};
static const OMRequestParams heatParams =			{"heat"};
static const OMRequestParams inputsParams =			{"inputs"};
static const OMRequestParams jobParams =			{"job"};
static const OMRequestParams moveParams =			{"move"};
static const OMRequestParams networkParams =		{"network"};
static const OMRequestParams scannerParams =		{"scanner"};
static const OMRequestParams sensorsParams =		{"sensors"};
static const OMRequestParams spindlesParams =		{"spindles"};
static const OMRequestParams stateParams =			{"state", "vn"};
static const OMRequestParams toolsParams =			{"tools"};
static const OMRequestParams volumesParams =		{"volumes"};

const OMRequestParams* GetNextToPoll()
{
	if (seqs.updateNetwork)
	{
		return &networkParams;
	}
	if (seqs.updateBoards)
	{
		return &boardsParams;
	}
	if (seqs.updateMove)
	{
		return &moveParams;
	}
	if (seqs.updateHeat)
	{
		return &heatParams;
	}
	if (seqs.updateTools)
	{
		return &toolsParams;
	}
	if (seqs.updateSpindles)
	{
		return &spindlesParams;
	}
	if (seqs.updateDirectories)
	{
		return &directoriesParams;
	}
	if (seqs.updateFans)
	{
		return &fansParams;
	}
	if (seqs.updateInputs)
	{
		return &inputsParams;
	}
	if (seqs.updateJob)
	{
		return &jobParams;
	}
	if (seqs.updateScanner)
	{
		return &scannerParams;
	}
	if (seqs.updateSensors)
	{
		return &sensorsParams;
	}
	if (seqs.updateState)
	{
		return &stateParams;
	}
	if (seqs.updateVolumes)
	{
		return &volumesParams;
	}

	return nullptr;
}

bool FlashData::IsValid() const
{
	return magic == magicVal
		&& touchVolume <= Buzzer::MaxVolume
		&& brightness >= Buzzer::MinBrightness
		&& brightness <= Buzzer::MaxBrightness
		&& language < UI::GetNumLanguages()
		&& colourScheme < NumColourSchemes
		&& displayDimmerType < DisplayDimmerType::NumTypes
		&& babystepAmountIndex > 0;
}

bool FlashData::operator==(const FlashData& other)
{
	return magic == other.magic
		&& baudRate == other.baudRate
		&& xmin == other.xmin
		&& xmax == other.xmax
		&& ymin == other.ymin
		&& ymax == other.ymax
		&& lcdOrientation == other.lcdOrientation
		&& touchOrientation == other.touchOrientation
		&& touchVolume == other.touchVolume
		&& language == other.language
		&& colourScheme == other.colourScheme
		&& brightness == other.brightness
		&& displayDimmerType == other.displayDimmerType
		&& infoTimeout == other.infoTimeout
		&& screensaverTimeout == other.screensaverTimeout
		&& babystepAmountIndex == other.babystepAmountIndex
		&& feedrate == other.feedrate;
}

void FlashData::SetDefaults()
{
	baudRate = DefaultBaudRate;
	xmin = 0;
	xmax = DisplayX - 1;
	ymin = 0;
	ymax = DisplayY - 1;
	lcdOrientation = DefaultDisplayOrientAdjust;
	touchOrientation = DefaultTouchOrientAdjust;
	touchVolume = Buzzer::DefaultVolume;
	brightness = Buzzer::DefaultBrightness;
	language = 0;
	colourScheme = 1;
	displayDimmerType = DisplayDimmerType::always;
	infoTimeout = DefaultInfoTimeout;
	screensaverTimeout = DefaultScreensaverTimeout;
	babystepAmountIndex = DefaultBabystepAmountIndex;
	feedrate = DefaultFeedrate;
	magic = magicVal;
}

// Load parameters from flash memory
void FlashData::Load()
{
	magic = 0xFFFFFFFF;				// to make sure we know if the read failed
#if SAM4S
	flash_read_user_signature(&(this->magic), (&(this->dummy) - reinterpret_cast<const char*>(&(this->magic)))/sizeof(uint32_t));
#else
	FlashStorage::read(0, &(this->magic), &(this->dummy) - reinterpret_cast<const char*>(&(this->magic)));
#endif
}

// Save parameters to flash memory
void FlashData::Save() const
{
#if SAM4S
	flash_erase_user_signature();
	flash_write_user_signature(&(this->magic), (&(this->dummy) - reinterpret_cast<const char*>(&(this->magic)))/sizeof(uint32_t));
#else
	FlashStorage::write(0, &(this->magic), &(this->dummy) - reinterpret_cast<const char*>(&(this->magic)));
#endif
}

// Return the host firmware features
FirmwareFeatures GetFirmwareFeatures()
{
	return firmwareFeatures;
}

// Strip the drive letter prefix from a file path if the host firmware doesn't support it
const char* _ecv_array CondStripDrive(const char* _ecv_array arg)
{
	return ((firmwareFeatures & noDriveNumber) != 0 && isdigit(arg[0]) && arg[1] == ':')
			? arg + 2
			: arg;
}

#if DEBUG
# define STRINGIFY(x)	#x
# define TOSTRING(x)	STRINGIFY(x)
# define ShowLine		debugField->SetValue(TOSTRING(__LINE__)); debugField->Refresh(true, 0, 0)
#else
# define ShowLine		(void)0
#endif

struct FileList
{
	int listNumber;
	size_t scrollOffset;
	String<100> path;
};

void Delay(uint32_t milliSeconds)
{
	const uint32_t now = SystemTick::GetTickCount();
	while (SystemTick::GetTickCount() - now < milliSeconds) { }
}

bool PrintInProgress()
{
	return status == PrinterStatus::printing || status == PrinterStatus::paused || status == PrinterStatus::pausing || status == PrinterStatus::resuming || status == PrinterStatus::simulating;
}

// Search an ordered table for a matching string
ReceivedDataEvent bsearch(const FieldTableEntry _ecv_array table[], size_t numElems, const char* key)
{
	size_t low = 0u, high = numElems;
	while (high > low)
	{
		const size_t mid = (high - low)/2 + low;
		const int t = strcasecmp(key, table[mid].varName);
		if (t == 0)
		{
			return table[mid].rde;
		}
		if (t > 0)
		{
			low = mid + 1u;
		}
		else
		{
			high = mid;
		}
	}
	return (low < numElems && strcasecmp(key, table[low].varName) == 0) ? table[low].rde : rcvUnknown;
}

// Return true if sending a command or file list request to the printer now is a good idea.
// We don't want to send these when the printer is busy with a previous command, because they will block normal status requests.
bool OkToSend()
{
	return status == PrinterStatus::idle || status == PrinterStatus::printing || status == PrinterStatus::paused || status == PrinterStatus::off;
}

// Return the printer status
PrinterStatus GetStatus()
{
	return status;
}

// Initialise the LCD and user interface. The non-volatile data must be set up before calling this.
void InitLcd()
{
	lcd.InitLCD(nvData.lcdOrientation, IS_24BIT, IS_ER);				// set up the LCD
	colours = &colourSchemes[nvData.colourScheme];
	UI::CreateFields(nvData.language, *colours, nvData.infoTimeout);	// create all the fields
	lcd.fillScr(black);													// make sure the memory is clear
	Delay(100);															// give the LCD time to update
	RestoreBrightness();												// turn the display on
}

// Ignore touches for a long time
void DelayTouchLong()
{
	lastTouchTime = SystemTick::GetTickCount();
	ignoreTouchTime = longTouchDelay;
}

// Ignore touches for a short time instead of the long time we already asked for
void ShortenTouchDelay()
{
	ignoreTouchTime = shortTouchDelay;
}

void TouchBeep()
{
	Buzzer::Beep(touchBeepFrequency, touchBeepLength, nvData.touchVolume);
}

void ErrorBeep()
{
	while (Buzzer::Noisy()) { }
	Buzzer::Beep(errorBeepFrequency, errorBeepLength, nvData.touchVolume);
}

// Draw a spot and wait until the user touches it, returning the touch coordinates in tx and ty.
// The alternative X and Y locations are so that the caller can allow for the touch panel being possibly inverted.
void DoTouchCalib(PixelNumber x, PixelNumber y, PixelNumber altX, PixelNumber altY, bool wantY, uint16_t& rawRslt)
{
	const PixelNumber touchCircleRadius = DisplayY/32;
	const PixelNumber touchCalibMaxError = DisplayY/6;

	lcd.setColor(colours->labelTextColour);
	lcd.fillCircle(x, y, touchCircleRadius);

	for (;;)
	{
		uint16_t tx, ty, rawX, rawY;
		if (touch.read(tx, ty, &rawX, &rawY))
		{
			if (   (abs((int)tx - (int)x) <= touchCalibMaxError || abs((int)tx - (int)altX) <= touchCalibMaxError)
				&& (abs((int)ty - (int)y) <= touchCalibMaxError || abs((int)ty - (int)altY) <= touchCalibMaxError)
			   )
			{
				TouchBeep();
				rawRslt = (wantY) ? rawY : rawX;
				break;
			}
		}
	}

	lcd.setColor(colours->defaultBackColour);
	lcd.fillCircle(x, y, touchCircleRadius);
}

void CalibrateTouch()
{
	DisplayField *oldRoot = mgr.GetRoot();
	mgr.SetRoot(touchCalibInstruction);
	mgr.Refresh(true);

	touch.init(DisplayX, DisplayY, DefaultTouchOrientAdjust);				// initialize the driver and clear any existing calibration

	// Draw spots on the edges of the screen, one at a time, and ask the user to touch them.
	// For the first two, we allow for the touch panel being the wrong way round.
	DoTouchCalib(DisplayX/2, touchCalibMargin, DisplayX/2, DisplayY - 1 - touchCalibMargin, true, nvData.ymin);
	if (nvData.ymin >= 4096/2)
	{
		touch.adjustOrientation(ReverseY);
		nvData.ymin = 4095 - nvData.ymin;
	}
	DoTouchCalib(DisplayX - touchCalibMargin - 1, DisplayY/2, touchCalibMargin, DisplayY/2, false, nvData.xmax);
	if (nvData.xmax < 4096/2)
	{
		touch.adjustOrientation(ReverseX);
		nvData.xmax = 4095 - nvData.xmax;
	}
	DoTouchCalib(DisplayX/2, DisplayY - 1 - touchCalibMargin, DisplayX/2, DisplayY - 1 - touchCalibMargin, true, nvData.ymax);
	DoTouchCalib(touchCalibMargin, DisplayY/2, touchCalibMargin, DisplayY/2, false, nvData.xmin);

	nvData.touchOrientation = touch.getOrientation();
	touch.calibrate(nvData.xmin, nvData.xmax, nvData.ymin, nvData.ymax, touchCalibMargin);

	mgr.SetRoot(oldRoot);
	mgr.Refresh(true);
}

bool IsSaveNeeded()
{
	return nvData != savedNvData;
}

void MirrorDisplay()
{
	nvData.lcdOrientation = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ ReverseX);
	lcd.setOrientation(nvData.lcdOrientation, IS_ER, true);
}

void InvertDisplay()
{
	nvData.lcdOrientation = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ (ReverseX | ReverseY));
	lcd.setOrientation(nvData.lcdOrientation, IS_ER, true);
}

void LandscapeDisplay(const bool withTouch)
{
	lcd.fillScr(black);
	lcd.setOrientation(nvData.lcdOrientation, IS_ER, true);
	if (withTouch)
	{
		touch.init(DisplayX, DisplayY, nvData.touchOrientation);
		touch.calibrate(nvData.xmin, nvData.xmax, nvData.ymin, nvData.ymax, touchCalibMargin);
	}
}

void PortraitDisplay(const bool withTouch)
{
	DisplayOrientation portrait = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ (SwapXY | ReverseX));
	lcd.fillScr(black);
	lcd.setOrientation(portrait, IS_ER, true);
	if (withTouch)
	{
		DisplayOrientation portraitTouch = static_cast<DisplayOrientation>(nvData.touchOrientation ^ (SwapXY | ReverseX));
		touch.init(DisplayXP, DisplayYP, portraitTouch);
		touch.calibrate(nvData.ymin, nvData.ymax, nvData.xmin, nvData.xmax, touchCalibMargin);
	}
}

void SetBaudRate(uint32_t rate)
{
	nvData.baudRate = rate;
	SerialIo::Init(rate);
}

extern void SetBrightness(int percent)
{
	nvData.brightness = constrain<int>(percent, Buzzer::MinBrightness, Buzzer::MaxBrightness);
	RestoreBrightness();
}

void DeactivateScreensaver() {
	if (screensaverActive) {
		UI::DeactivateScreensaver();
		screensaverActive = false;
		printerPollInterval = defaultPrinterPollInterval;
	}
}

extern void RestoreBrightness()
{
	Buzzer::SetBacklight(nvData.brightness);
	lastActionTime = SystemTick::GetTickCount();
	isDimmed = false;
	DeactivateScreensaver();
}

extern void DimBrightness()
{
	if (   (nvData.displayDimmerType == DisplayDimmerType::always)
		|| (nvData.displayDimmerType == DisplayDimmerType::onIdle && (status == PrinterStatus::idle || status == PrinterStatus::off))
	   )
	{
		Buzzer::SetBacklight(nvData.brightness/8);
		isDimmed = true;
	}
}

void ActivateScreensaver()
{
	if (!screensaverActive)
	{
		if (!isDimmed)
		{
			Buzzer::SetBacklight(nvData.brightness/4);	// If the user disabled dimming we do it still here
			isDimmed = true;
		}
		screensaverActive = true;
		UI::ActivateScreensaver();
		printerPollInterval = slowPrinterPollInterval;
	}
	else
	{
		UI::AnimateScreensaver();
	}
}

DisplayDimmerType GetDisplayDimmerType()
{
	return nvData.displayDimmerType;
}

void SetDisplayDimmerType(DisplayDimmerType newType)
{
	nvData.displayDimmerType = newType;
}

void SetVolume(uint8_t newVolume)
{
	nvData.touchVolume = newVolume;
}

void SetInfoTimeout(uint8_t newInfoTimeout)
{
	nvData.infoTimeout = newInfoTimeout;
}

void SetScreensaverTimeout(uint32_t screensaverTimeout)
{
	nvData.screensaverTimeout = screensaverTimeout;
}

bool SetColourScheme(uint8_t newColours)
{
	const bool ret = (newColours != nvData.colourScheme);
	nvData.colourScheme = newColours;
	return ret;
}

// Set the language, returning true if it has changed
bool SetLanguage(uint8_t newLanguage)
{
	const bool ret = (newLanguage != nvData.language);
	nvData.language = newLanguage;
	return ret;
}

uint32_t GetBaudRate()
{
	return nvData.baudRate;
}

uint32_t GetVolume()
{
	return nvData.touchVolume;
}

int GetBrightness()
{
	return (int)nvData.brightness;
}

uint32_t GetScreensaverTimeout() {
	return nvData.screensaverTimeout;
}

uint8_t GetBabystepAmountIndex()
{
	return nvData.babystepAmountIndex;
}

void SetBabystepAmountIndex(uint8_t babystepAmountIndex)
{
	nvData.babystepAmountIndex = babystepAmountIndex;
}

uint16_t GetFeedrate()
{
	return nvData.feedrate;
}

void SetFeedrate(uint16_t feedrate)
{
	nvData.feedrate = feedrate;
}

// Factory reset
void FactoryReset()
{
	while (Buzzer::Noisy()) { }
	nvData.SetInvalid();
	nvData.Save();
	savedNvData = nvData;
	Buzzer::Beep(touchBeepFrequency, 400, Buzzer::MaxVolume);		// long beep to acknowledge it
	while (Buzzer::Noisy()) { }
	Reset();														// reset the processor
}

// Save settings
void SaveSettings()
{
	while (Buzzer::Noisy()) { }
	nvData.Save();
	// To make sure it worked, load the settings again
	savedNvData.Load();
}

// This is called when the status changes
void SetStatus(const char * sts)
{
	PrinterStatus newStatus = PrinterStatus::connecting;
	if (!initialized)
	{
		newStatus = PrinterStatus::panelInitializing;
	}
	else
	{
		const PrinterStatusMapEntry key = (PrinterStatusMapEntry) {sts, PrinterStatus::connecting};
		const PrinterStatusMapEntry * statusFromMap =
				(PrinterStatusMapEntry *) bsearch(
						&key,
						printerStatusMap,
						ARRAY_SIZE(printerStatusMap),
						sizeof(PrinterStatusMapEntry),
						[](auto a, auto b)
						{
							return strcasecmp(((PrinterStatusMapEntry*) a)->key, ((PrinterStatusMapEntry*)b)->key);
						});
		if (statusFromMap != nullptr)
		{
			newStatus = statusFromMap->val;
		}
	}

	if (newStatus != status)
	{
		if (isDimmed)
		{
			RestoreBrightness();
		}
		UI::ChangeStatus(status, newStatus);

		if (status == PrinterStatus::configuring || (status == PrinterStatus::connecting && newStatus != PrinterStatus::configuring))
		{
			MessageLog::AppendMessage("Connected");
		}

		status = newStatus;
		UI::UpdatePrintingFields();
	}
}

// Set the status back to "Connecting"
void Reconnect()
{
	initialized = false;
	SetStatus(nullptr);
}

// Try to get an integer value from a string. If it is actually a floating point value, round it.
bool GetInteger(const char s[], int32_t &rslt)
{
	if (s[0] == 0) return false;			// empty string

	const char* endptr;
	rslt = (int) StrToI32(s, &endptr);
	if (*endptr == 0) return true;			// we parsed an integer

	const float d = SafeStrtof(s, &endptr);		// try parsing a floating point number
	if (*endptr == 0)
	{
		rslt = (int)((d < 0.0) ? d - 0.5 : d + 0.5);
		return true;
	}
	return false;
}

// Try to get an unsigned integer value from a string
bool GetUnsignedInteger(const char s[], uint32_t &rslt)
{
	if (s[0] == 0) return false;			// empty string
	const char* endptr;
	rslt = (int) StrToU32(s, &endptr);
	return *endptr == 0;
}

// Try to get a floating point value from a string. if it is actually a floating point value, round it.
bool GetFloat(const char s[], float &rslt)
{
	if (s[0] == 0) return false;			// empty string
	const char* endptr;
	rslt = SafeStrtof(s, &endptr);
	return *endptr == 0;					// we parsed a float
}

// Try to get a bool value from a string.
bool GetBool(const char s[], bool &rslt)
{
	if (s[0] == 0) return false;			// empty string

	rslt = (strcasecmp(s, "true") == 0);
	return true;
}

void StartReceivedMessage()
{
	ShowLine;
	newMessageSeq = messageSeq;
	MessageLog::BeginNewMessage();
	FileManager::BeginNewMessage();
	currentAlert.flags = 0;
	ShowLine;
}

void SeqsRequestDone(const ReceivedDataEvent rde)
{
	switch (rde)
	{
	case rcvOMKeyBoards:
		seqs.updateBoards = false;
		break;
	case rcvOMKeyDirectories:
		seqs.updateDirectories = false;
		break;
	case rcvOMKeyFans:
		seqs.updateFans = false;
		break;
	case rcvOMKeyHeat:
		seqs.updateHeat = false;
		break;
	case rcvOMKeyInputs:
		seqs.updateInputs = false;
		break;
	case rcvOMKeyJob:
		seqs.updateJob = false;
		break;
	case rcvOMKeyMove:
		seqs.updateMove = false;
		break;
	case rcvOMKeyNetwork:
		seqs.updateNetwork = false;
		break;
	case rcvOMKeyScanner:
		seqs.updateScanner = false;
		break;
	case rcvOMKeySensors:
		seqs.updateSensors = false;
		break;
	case rcvOMKeySpindles:
		seqs.updateSpindles = false;
		break;
	case rcvOMKeyState:
		seqs.updateState = false;
		break;
	case rcvOMKeyTools:
		seqs.updateTools = false;
		break;
	case rcvOMKeyVolumes:
		seqs.updateVolumes = false;
		break;
	default:
		break;

	}
}

void EndReceivedMessage()
{
	ShowLine;
	lastResponseTime = SystemTick::GetTickCount();
	SeqsRequestDone(currentResponseType);
	currentResponseType = rcvUnknown;

	if (newMessageSeq != messageSeq)
	{
		messageSeq = newMessageSeq;
		MessageLog::DisplayNewMessage();
	}
	FileManager::EndReceivedMessage();
	if ((currentAlert.flags & Alert::GotMode) != 0 && currentAlert.mode < 0)
	{
		UI::ClearAlert();
	}
	else if (currentAlert.flags == Alert::GotAll && currentAlert.seq != lastAlertSeq)
	{
		UI::ProcessAlert(currentAlert);
		lastAlertSeq = currentAlert.seq;
	}
	ShowLine;
}

void UpdateSeqs(const ReceivedDataEvent rde, const int32_t ival)
{
	switch (rde)
	{
#if FETCH_BOARDS
	case rcvLiveSeqsBoards:
		if (seqs.boards != ival)
		{
			seqs.boards = ival;
			seqs.updateBoards = true;
		}
		break;
#endif
#if FETCH_DIRECTORIES
	case rcvLiveSeqsDirectories:
		if (seqs.directories != ival)
		{
			seqs.directories = ival;
			seqs.updateDirectories = true;
		}
		break;
#endif
#if FETCH_FANS
	case rcvLiveSeqsFans:
		if (seqs.fans != ival)
		{
			seqs.fans = ival;
			seqs.updateFans = true;
		}
		break;
#endif
#if FETCH_HEAT
	case rcvLiveSeqsHeat:
		if (seqs.heat != ival)
		{
			seqs.heat = ival;
			seqs.updateHeat = true;
		}
		break;
#endif
#if FETCH_INPUTS
	case rcvLiveSeqsInputs:
		if (seqs.inputs != ival)
		{
			seqs.inputs = ival;
			seqs.updateInputs = true;
		}
		break;
#endif
#if FETCH_JOB
	case rcvLiveSeqsJob:
		if (seqs.job != ival)
		{
			seqs.job = ival;
			seqs.updateJob = true;
		}
		break;
#endif
#if FETCH_MOVE
	case rcvLiveSeqsMove:
		if (seqs.move != ival)
		{
			seqs.move = ival;
			seqs.updateMove = true;
		}
		break;
#endif
#if FETCH_NETWORK
	case rcvLiveSeqsNetwork:
		if (seqs.network != ival)
		{
			seqs.network = ival;
			seqs.updateNetwork = true;
		}
		break;
#endif
#if FETCH_SCANNER
	case rcvLiveSeqsScanner:
		if (seqs.scanner != ival)
		{
			seqs.scanner = ival;
			seqs.updateScanner = true;
		}
		break;
#endif
#if FETCH_SENSORS
	case rcvLiveSeqsSensors:
		if (seqs.sensors != ival)
		{
			seqs.sensors = ival;
			seqs.updateSensors = true;
		}
		break;
#endif
#if FETCH_SPINDLES
	case rcvLiveSeqsSpindles:
		if (seqs.spindles != ival)
		{
			seqs.spindles = ival;
			seqs.updateSpindles = true;
		}
		break;
#endif
#if FETCH_STATE
	case rcvLiveSeqsState:
		if (seqs.state != ival)
		{
			seqs.state = ival;
			seqs.updateState = true;
		}
		break;
#endif
#if FETCH_TOOLS
	case rcvLiveSeqsTools:
		if (seqs.tools != ival)
		{
			seqs.tools = ival;
			seqs.updateTools = true;
		}
		break;
#endif
#if FETCH_VOLUMES
	case rcvLiveSeqsVolumes:
		if (seqs.volumes != ival)
		{
			seqs.volumes = ival;
			seqs.updateVolumes = true;
		}
		break;
#endif
	default:
		break;
	}
}

const OMRequestParams* GetOMRequestParams()
{
	switch (currentResponseType)
	{
	case rcvOMKeyNoKey:
		return &noKeyParams;
	case rcvOMKeyBoards:
		return &boardsParams;
	case rcvOMKeyFans:
		return &fansParams;
	case rcvOMKeyHeat:
		return &heatParams;
	case rcvOMKeyInputs:
		return &inputsParams;
	case rcvOMKeyJob:
		return &jobParams;
	case rcvOMKeyMove:
		return &moveParams;
	case rcvOMKeyNetwork:
		return &networkParams;
	case rcvOMKeyScanner:
		return &scannerParams;
	case rcvOMKeySensors:
		return &sensorsParams;
	case rcvOMKeySpindles:
		return &spindlesParams;
	case rcvOMKeyState:
		return &stateParams;
	case rcvOMKeyTools:
		return &toolsParams;
	case rcvOMKeyVolumes:
		return &volumesParams;
	default:
		return nullptr;
	}
}

// Public functions called by the SerialIo module
void ProcessReceivedValue(StringRef id, const char data[], const size_t indices[])
{
	if (stringStartsWith(id.c_str(), "result"))
	{
		auto requestParams = GetOMRequestParams();
		if (requestParams != nullptr)
		{
			// We might either get something like:
			// * "result[optional modified]:[key]:[field]" for a live response or
			// * "result[optional modified]:[field]" for a detailed response
			// If live response remove "result:"
			// else replace "result" by "key" (do NOT replace anything beyond "result" as there might be an _ecv_array modifier)

			id.Erase(0, 6);		// Erase the string "result"
			if (currentResponseType == rcvOMKeyNoKey)
			{
				id.Erase(0);	// Also erase the colon
			}
			else
			{
				id.Prepend(requestParams->key);		// Prepend the key of the current response
			}
		}
	}

	const ReceivedDataEvent rde = bsearch(fieldTable, ARRAY_SIZE(fieldTable), id.c_str());
	switch (rde)
	{
	// M409 section
	case rcvKey:
		ShowLine;
		{
			currentResponseType = bsearch(keyResponseTypeTable, ARRAY_SIZE(keyResponseTypeTable), data);
			switch (currentResponseType) {
			case rcvOMKeyMove:
				visibleAxesCounted = 0;
				break;
			case rcvOMKeySpindles:
				lastSpindle = -1;
				break;
			case rcvOMKeyTools:
				lastTool = -1;
				break;
			case rcvOMKeyVolumes:
				mountedVolumesCounted = 0;
				break;
			default:
				break;
			}
		}
		break;

	// Live response
	case rcvLiveFansActualValue:
		ShowLine;
		if (indices[0] == 0)			// currently we only handle one fan
		{
			float f;
			bool b = GetFloat(data, f);
			if (b && f >= 0.0 && f <= 1.0)
			{
				UI::UpdateFanPercent((int)((f * 100.0f) + 0.5f));
			}
		}
		break;
	case rcvLiveHeatHeatersActive:
		ShowLine;
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UI::UpdateActiveTemperature(indices[0], ival);
			}
		}
		break;

	case rcvLiveHeatHeatersCurrent:
		ShowLine;
		{
			float fval;
			if (GetFloat(data, fval))
			{
				ShowLine;
				UI::UpdateCurrentTemperature(indices[0], fval);
			}
		}
		break;

	case rcvLiveHeatHeatersStandby:
		ShowLine;
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UI::UpdateStandbyTemperature(indices[0], ival);
			}
		}
		break;

	case rcvLiveHeatHeatersState:
		ShowLine;
		{
			const HeaterStatusMapEntry key = (HeaterStatusMapEntry) {data, HeaterStatus::off};
			const HeaterStatusMapEntry * statusFromMap =
					(HeaterStatusMapEntry *) bsearch(
							&key,
							heaterStatusMap,
							ARRAY_SIZE(heaterStatusMap),
							sizeof(HeaterStatusMapEntry),
							[](auto a, auto b)
							{
								return strcasecmp(((HeaterStatusMapEntry*) a)->key, ((HeaterStatusMapEntry*)b)->key);
							});
			const HeaterStatus status = (statusFromMap != nullptr) ? statusFromMap->val : HeaterStatus::off;
			UI::UpdateHeaterStatus(indices[0], status);
		}
		break;

	case rcvLiveJobFilePosition:
		{
			if (PrintInProgress() && fileSize > 0)
			{
				uint32_t ival;
				if (GetUnsignedInteger(data, ival))
				{
					UI::SetPrintProgressPercent((unsigned int)(((ival*100.0f)/fileSize) + 0.5));
				}

			}
		}
		break;

	case rcvLiveJobTimesLeftFilament:
	case rcvLiveJobTimesLeftFile:
	case rcvLiveJobTimesLeftLayer:
		ShowLine;
		{
			int32_t i;
			bool b = GetInteger(data, i);
			if (b && i >= 0 && i < 10 * 24 * 60 * 60 && PrintInProgress())
			{
				UI::UpdateTimesLeft((rde == rcvLiveJobTimesLeftFilament) ? 1 : (rde == rcvLiveJobTimesLeftLayer) ? 2 : 0, i);
			}
		}
		break;

	case rcvMoveAxesHomed:
		ShowLine;
		{
			bool isHomed;
			if (indices[0] < MaxTotalAxes && GetBool(data, isHomed))
			{
				UI::UpdateHomedStatus(indices[0], isHomed);
			}
		}
		break;

	case rcvLiveMoveAxesUserPosition:
		ShowLine;
		{
			float fval;
			if (GetFloat(data, fval))
			{
				UI::UpdateAxisPosition(indices[0], fval);
			}
		}
		break;

	case rcvLiveSensorsProbeValue:
		{
			if (indices[0] == 0 && indices[1] == 0)			// currently we only handle one probe with one value
			UI::UpdateZProbe(data);
		}
		break;

	case rcvLiveSeqsBoards:
	case rcvLiveSeqsDirectories:
	case rcvLiveSeqsFans:
	case rcvLiveSeqsHeat:
	case rcvLiveSeqsInputs:
	case rcvLiveSeqsJob:
	case rcvLiveSeqsMove:
	case rcvLiveSeqsNetwork:
	case rcvLiveSeqsReply:
	case rcvLiveSeqsScanner:
	case rcvLiveSeqsSensors:
	case rcvLiveSeqsSpindles:
	case rcvLiveSeqsState:
	case rcvLiveSeqsTools:
	case rcvLiveSeqsVolumes:
		ShowLine;
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UpdateSeqs(rde, ival);
			}

		}
		break;

	case rcvLiveSpindlesCurrent:
		{
			uint32_t current;
			if (GetUnsignedInteger(data, current))
			{
				UI::SetSpindleCurrent(indices[0], current);
			}
		}
		break;

	case rcvLiveStateCurrentTool:
		if (status == PrinterStatus::connecting || status == PrinterStatus::panelInitializing)
		{
			break;
		}
		{
			int32_t tool;
			if (GetInteger(data, tool))
			{
				UI::SetCurrentTool(tool);
			}
		}
		break;

	case rcvLiveStateStatus:
		ShowLine;
		{
			SetStatus(data);
		}
		break;

	case rcvLiveStateUptime:
		ShowLine;
		{
			uint32_t uival;
			if (GetUnsignedInteger(data, uival))
			{
				// Controller was restarted
				if (uival < remoteUpTime)
				{
					resetSeqs();
					UI::ResetBedAndChamber();
				}
				remoteUpTime = uival;
			}
		}
		break;

	case rcvLiveToolsState:
		{
			const ToolStatusMapEntry key = (ToolStatusMapEntry) {data, ToolStatus::off};
			const ToolStatusMapEntry * statusFromMap =
					(ToolStatusMapEntry *) bsearch(
							&key,
							toolStatusMap,
							ARRAY_SIZE(toolStatusMap),
							sizeof(ToolStatusMapEntry),
							[](auto a, auto b)
							{
								return strcasecmp(((ToolStatusMapEntry*) a)->key, ((ToolStatusMapEntry*)b)->key);
							});
			const ToolStatus status = (statusFromMap != nullptr) ? statusFromMap->val : ToolStatus::off;
			UI::UpdateToolStatus(indices[0], status);
		}
		break;

	// Boards section
	case rcvBoardsFirmwareName:
		if (indices[0] == 0)			// currently we only handle the first board
		{
			for (size_t i = 0; i < ARRAY_SIZE(firmwareTypes); ++i)
			{
				if (stringStartsWith(data, firmwareTypes[i].name))
				{
					const FirmwareFeatures newFeatures = firmwareTypes[i].features;
					if (newFeatures != firmwareFeatures)
					{
						firmwareFeatures = newFeatures;
						UI::FirmwareFeaturesChanged(firmwareFeatures);
						FileManager::FirmwareFeaturesChanged();
					}
					break;
				}
			}
		}
		break;

	// Heat section
	case rcvHeatBedHeaters:
		{
			static bool seen;
			if (indices[0] == 0)
			{
				seen = false;
			}
			int32_t h;
			if (!seen && GetInteger(data, h) && h > -1)
			{
				seen = true;
				UI::SetBedOrChamberHeater(h, indices[0]);
			}
		}
		break;

	case rcvHeatChamberHeaters:
		{
			static bool seen;
			if (indices[0] == 0)
			{
				seen = false;
			}
			int32_t h;
			if (!seen && GetInteger(data, h) && h > -1)
			{
				seen = true;
				UI::SetBedOrChamberHeater(h, indices[0], false);
			}
		}
		break;

	// Job section
	case rcvJobFileFilename:
		UI::PrintingFilenameChanged(data);
		break;

	case rcvJobFileSize:
		{
			uint32_t ival;
			if (GetUnsignedInteger(data, ival))
			{
				fileSize = ival;
			}
			else
			{
				fileSize = 0;
			}
		}
		break;

	// Move section
	case rcvMoveAxesBabystep:
		{
			float f;
			if (GetFloat(data, f))
			{
				UI::SetBabystepOffset(indices[0], f);
			}
		}
		break;

	case rcvMoveAxesLetter:
		{
			UI::SetAxisLetter(indices[0], data[0]);
		}
		break;

	case rcvMoveAxesVisible:
		{
			bool visible;
			if (GetBool(data, visible))
			{
				UI::SetAxisVisible(indices[0], visible);
				if (visible)
				{
					++visibleAxesCounted;
				}
			}

		}
		break;

	case rcvMoveAxesWorkplaceOffsets:
		{
			float offset;
			if (GetFloat(data, offset))
			{
				UI::SetAxisWorkplaceOffset(indices[0], indices[1], offset);
			}
		}
		break;

	case rcvMoveExtrudersFactor:
		ShowLine;
		{
			float fval;
			if (GetFloat(data, fval))
			{
				UI::UpdateExtrusionFactor(indices[0], (int)((fval * 100.0f) + 0.5));
			}
		}
		break;

	case rcvMoveKinematicsName:
		if (status != PrinterStatus::configuring && status != PrinterStatus::connecting)
		{
			isDelta = (strcasecmp(data, "delta") == 0);
			UI::UpdateGeometry(numAxes, isDelta);
		}
		break;

	case rcvMoveSpeedFactor:
		{
			float fval;
			if (GetFloat(data, fval))
			{
				UI::UpdateSpeedPercent((int) ((fval * 100.0f) + 0.5f));
			}
		}
		break;

	case rcvMoveWorkplaceNumber:
		{
			uint32_t workplaceNumber;
			if (GetUnsignedInteger(data, workplaceNumber))
			{
				UI::SetCurrentWorkplaceNumber(workplaceNumber);
			}
		}
		break;

	// Network section
	case rcvNetworkName:
		if (status != PrinterStatus::configuring && status != PrinterStatus::connecting)
		{
			UI::UpdateMachineName(data);
		}
		break;
	case rcvNetworkInterfacesActualIP:
		{
			// Only look at the first valid IP
			if (indices[0] > 0)
			{
				return;
			}
			UI::UpdateIP(data);
		}
		break;

	// Spindles section
	case rcvSpindlesActive:
		{
			uint32_t active;
			if (GetUnsignedInteger(data, active))
			{
				UI::SetSpindleActive(indices[0], active);
			}

			for (size_t i = lastSpindle + 1; i < indices[0]; ++i)
			{
				OM::RemoveSpindle(i, false);
			}
			lastSpindle = indices[0];
		}
		break;

	case rcvSpindlesMax:
		// fans also has a field "result^:max"
		if (currentResponseType != rcvOMKeySpindles)
		{
			break;
		}
		{
			uint32_t max;
			if (GetUnsignedInteger(data, max))
			{
				UI::SetSpindleMax(indices[0], max);
			}
		}
		break;

	case rcvSpindlesTool:
		{
			int32_t toolNumber;
			if (GetInteger(data, toolNumber))
			{
				UI::SetSpindleTool(indices[0], toolNumber);
			}
		}
		break;

	// State section
	case rcvStateMessageBox:
		// Nessage box has been dealt with somewhere else
		if (data[0] == 0)
		{
			UI::ClearAlert();
		}
		break;

	case rcvStateMessageBoxAxisControls:
		if (GetUnsignedInteger(data, currentAlert.controls))
		{
			currentAlert.flags |= Alert::GotControls;
		}
		break;

	case rcvStateMessageBoxMessage:
		currentAlert.text.copy(data);
		currentAlert.flags |= Alert::GotText;
		break;

	case rcvStateMessageBoxMode:
		if (GetInteger(data, currentAlert.mode))
		{
			currentAlert.flags |= Alert::GotMode;
		}
		break;

	case rcvStateMessageBoxSeq:
		if (GetUnsignedInteger(data, currentAlert.seq))
		{
			currentAlert.flags |= Alert::GotSeq;
		}
		break;

	case rcvStateMessageBoxTimeout:
		if (GetFloat(data, currentAlert.timeout))
		{
			currentAlert.flags |= Alert::GotTimeout;
		}
		break;

	case rcvStateMessageBoxTitle:
		currentAlert.title.copy(data);
		currentAlert.flags |= Alert::GotTitle;
		break;

	// Tools section
	case rcvToolsExtruders:
		{
			if (indices[1] > 0)
			{
				return;
			}
			int32_t extruder;
			if (GetInteger(data, extruder))
			{
				UI::SetToolExtruder(indices[0], extruder);
			}
		}
		break;

	case rcvToolsHeaters:
		{
			if (indices[1] > 0)
			{
				return;
			}
			int32_t heater;
			if (GetInteger(data, heater))
			{
				UI::SetToolHeater(indices[0], heater);
			}
		}
		break;

	case rcvToolsNumber:
		{
			for (size_t i = lastTool + 1; i < indices[0]; ++i)
			{
				OM::RemoveTool(i, false);
			}
			lastTool = indices[0];
		}
		break;

	case rcvToolsOffsets:
		{
			float offset;
			if (GetFloat(data, offset))
			{
				UI::SetToolOffset(indices[0], indices[1], offset);
			}
		}
		break;

	// Volumes section
	case rcvVolumesMounted:
		{
			bool mounted;
			if (GetBool(data, mounted) && mounted)
			{
				++mountedVolumesCounted;
			}
		}
		break;

	// Push messages
	case rcvPushResponse:
		MessageLog::SaveMessage(data);
		break;

	case rcvPushMessage:
		if (data[0] == 0)
		{
			UI::ClearAlert();
		}
		else
		{
			UI::ProcessSimpleAlert(data);
		}
		break;

	case rcvPushSeq:
		GetUnsignedInteger(data, newMessageSeq);
		break;

	case rcvPushBeepDuration:
		GetInteger(data, beepLength);
		break;

	case rcvPushBeepFrequency:
		GetInteger(data, beepFrequency);
		break;

	// M20 section
	case rcvM20Dir:
		FileManager::ReceiveDirectoryName(data);
		break;

	case rcvM20Err:
		{
			int32_t i;
			if (GetInteger(data, i))
			{
				if (i >= 0)
				{
					FileManager::ReceiveErrorCode(i);
				}
				else if (i == -1)
				{
					// RRF ran out of buffers
				}
			}
		}
		break;

	case rcvM20Files:
		ShowLine;
		if (indices[0] == 0)
		{
			FileManager::BeginReceivingFiles();
		}
		FileManager::ReceiveFile(data);
		break;

	// M36 section
	case rcvM36Filament:
		ShowLine;
		{
			static float totalFilament = 0.0;
			if (indices[0] == 0)
			{
				totalFilament = 0.0;
			}
			float f;
			if (GetFloat(data, f))
			{
				totalFilament += f;
				UI::UpdateFileFilament((int)totalFilament);
			}
		}
		break;
	case rcvM36Filename:
		break;

	case rcvM36GeneratedBy:
		UI::UpdateFileGeneratedByText(data);
		break;

	case rcvM36Height:
		{
			float f;
			if (GetFloat(data, f))
			{
				UI::UpdateFileObjectHeight(f);
			}
		}
		break;

	case rcvM36LastModified:
		UI::UpdateFileLastModifiedText(data);
		break;

	case rcvM36LayerHeight:
		{
			float f;
			if (GetFloat(data, f))
			{
				UI::UpdateFileLayerHeight(f);
			}
		}
		break;

	case rcvM36PrintTime:
	case rcvM36SimulatedTime:
		{
			int32_t sz;
			if (GetInteger(data, sz) && sz > 0)
			{
				UI::UpdatePrintTimeText((uint32_t)sz, rde == rcvM36SimulatedTime);
			}
		}
		break;

	case rcvM36Size:
		{
			int32_t sz;
			if (GetInteger(data, sz))
			{
				UI::UpdateFileSize(sz);
			}
		}
		break;

	default:
		break;
	}
	ShowLine;
}

// Public function called when the serial I/O module finishes receiving an array of values
void ProcessArrayEnd(const char id[], const size_t indices[])
{
	if (indices[0] == 0 && strcmp(id, "files^") == 0)
	{
		FileManager::BeginReceivingFiles();				// received an empty file list - need to tell the file manager about it
	}
	else if (currentResponseType == rcvOMKeyMove && strcasecmp(id, "move:axes^") == 0)
	{
		OM::RemoveAxis(indices[0], true);
		numAxes = constrain<unsigned int>(visibleAxesCounted, MIN_AXES, MaxTotalAxes);
		UI::UpdateGeometry(numAxes, isDelta);
	}
	else if (currentResponseType == rcvOMKeySpindles)
	{
		if (strcasecmp(id, "spindles^") == 0)
		{
			OM::RemoveSpindle(lastSpindle + 1, true);
			UI::AllToolsSeen();
		}
	}
	else if (currentResponseType == rcvOMKeyTools)
	{
		if (strcasecmp(id, "tools^") == 0)
		{
			OM::RemoveTool(lastTool + 1, true);
			UI::AllToolsSeen();
		}
		else if (strcasecmp(id, "tools^:extruders^") == 0 && indices[1] == 0)
		{
			UI::SetToolExtruder(indices[0], -1);			// No extruder defined for this tool
		}
		else if (strcasecmp(id, "tools^:heaters^") == 0 && indices[1] == 0)
		{
			UI::SetToolHeater(indices[0], -1);				// No heater defined for this tool
		}
	}
	else if (currentResponseType == rcvOMKeyVolumes && strcasecmp(id, "volumes^") == 0)
	{
		FileManager::SetNumVolumes(mountedVolumesCounted);
	}
}

void ParserErrorEncountered()
{
	// TODO: Handle parser errors
}

// Update those fields that display debug information
void UpdateDebugInfo()
{
	freeMem->SetValue(GetFreeMemory());
}

#if 0
void SelfTest()
{
	// Measure the 3.3V supply against the internal reference

	// Do internal and external loopback tests on the serial port

	// Initialize fields with the widest expected values so that we can make sure they fit
	currentTemps[0]->SetValue(129.0);
	currentTemps[1]->SetValue(299.0);
	currentTemps[2]->SetValue(299.0);
	activeTemps[0]->SetValue(120);
	activeTemps[1]->SetValue(280);
	activeTemps[2]->SetValue(280);
	standbyTemps[1]->SetValue(280);
	standbyTemps[2]->SetValue(280);
	axisPos[0]->SetValue(220.9);
	axisPos[1]->SetValue(220.9);
	axisPos[2]->SetValue(199.99);
	zProbe->SetValue("1023 (1023)");
	spd->SetValue(169);
	extrusionFactors[0]->SetValue(169);
	extrusionFactors[1]->SetValue(169);
}
#endif

/**
 * \brief Application entry point.
 *
 * \return Unused (ANSI-C compatibility).
 */
int main(void)
{
    SystemInit();						// set up the clock etc.
    InitMemory();

	matrix_set_system_io(CCFG_SYSIO_SYSIO4 | CCFG_SYSIO_SYSIO5 | CCFG_SYSIO_SYSIO6 | CCFG_SYSIO_SYSIO7);	// enable PB4-PB7 pins
	pmc_enable_periph_clk(ID_PIOA);		// enable the PIO clock
	pmc_enable_periph_clk(ID_PIOB);		// enable the PIO clock
	pmc_enable_periph_clk(ID_PWM);		// enable the PWM clock
#if SAM4S
	pmc_enable_periph_clk(ID_UART0);	// enable UART1 clock
#else
	pmc_enable_periph_clk(ID_UART1);	// enable UART1 clock
#endif

	Buzzer::Init();						// init the buzzer, must also call this before the backlight can be used

	wdt_init (WDT, WDT_MR_WDRSTEN, 1000, 1000);
	SysTick_Config(SystemCoreClock / 1000);
	lastTouchTime = SystemTick::GetTickCount();

	firmwareFeatures = firmwareTypes[0].features;		// assume RepRapFirmware until we hear otherwise

	// Read parameters from flash memory
	nvData.Load();
	if (nvData.IsValid())
	{
		// The touch panel has already been calibrated
		InitLcd();
		touch.init(DisplayX, DisplayY, nvData.touchOrientation);
		touch.calibrate(nvData.xmin, nvData.xmax, nvData.ymin, nvData.ymax, touchCalibMargin);
		savedNvData = nvData;
	}
	else
	{
		// The touch panel has not been calibrated, and we do not know which way up it is
		nvData.SetDefaults();
		InitLcd();
		CalibrateTouch();							// this includes the touch driver initialisation
		SaveSettings();
	}

	// Set up the baud rate
	SerialIo::Init(nvData.baudRate);

	MessageLog::Init();

#ifdef OEM
	// Display the splash screen unless it was a software reset (we use software reset to change the language or colour scheme)
	if (rstc_get_reset_cause(RSTC) != RSTC_SOFTWARE_RESET)
	{
		lcd.fillScr(black);
		lcd.drawCompressedBitmapBottomToTop(0, 0, DISPLAY_X, DISPLAY_Y, splashScreenImage);
		Delay(5000);								// hold it there for 5 seconds
	}
#else
	// Display the splash screen if one has been appended to the file, unless it was a software reset (we use software reset to change the language or colour scheme)
	// The splash screen data comprises the number of X pixels, then the number of Y pixels, then the data
	if (rstc_get_reset_cause(RSTC) != RSTC_SOFTWARE_RESET && _esplash[0] == DISPLAY_X && _esplash[1] == DISPLAY_Y)
	{
		lcd.fillScr(black);
		lcd.drawCompressedBitmapBottomToTop(0, 0, DISPLAY_X, DISPLAY_Y, _esplash + 2);
		const uint32_t now = SystemTick::GetTickCount();
		do
		{
			uint16_t x, y;
			if (touch.read(x, y))
			{
				break;
			}
		} while (SystemTick::GetTickCount() - now < 5000);		// hold it there for 5 seconds or until touched
	}
#endif

	mgr.Refresh(true);								// draw the screen for the first time
	UI::UpdatePrintingFields();

	SerialIo::SendString("M409 F\"d99f\"\n");		// Get initial status
	lastPollTime = SystemTick::GetTickCount();

	// Hide all tools and heater related columns initially
	UI::AllToolsSeen();

	debugField->Show(DEBUG != 0);					// show the debug field only if debugging is enabled

	// Display the Control tab. This also refreshes the display.
	UI::ShowDefaultPage();

	// Sort the fieldTable
	qsort(
			fieldTable,
			ARRAY_SIZE(fieldTable),
			sizeof(FieldTableEntry),
			[](const void* a, const void* b)
			{
				return strcasecmp(((FieldTableEntry*) a)->varName, ((FieldTableEntry*) b)->varName);
			});

	resetSeqs();

//	lastResponseTime = SystemTick::GetTickCount();	// pretend we just received a response

	lastActionTime = SystemTick::GetTickCount();

	for (;;)
	{
		ShowLine;

		// 1. Check for input from the serial port and process it.
		// This calls back into functions StartReceivedMessage, ProcessReceivedValue, ProcessArrayLength and EndReceivedMessage.
		SerialIo::CheckInput();
		ShowLine;

		// 2. if displaying the message log, update the times
		UI::Spin();
		ShowLine;

		// 3. Check for a touch on the touch panel.
		if (SystemTick::GetTickCount() - lastTouchTime >= ignoreTouchTime)
		{
			UI::OnButtonPressTimeout();

			uint16_t x, y;
			if (touch.read(x, y))
			{
#if 0
				touchX->SetValue((int)x);	//debug
				touchY->SetValue((int)y);	//debug
#endif
				if (isDimmed || screensaverActive)
				{
					RestoreBrightness();
					DelayTouchLong();			// ignore further touches for a while
				}
				else
				{
					lastActionTime = SystemTick::GetTickCount();
					ButtonPress bp = mgr.FindEvent(x, y);
					if (bp.IsValid())
					{
						DelayTouchLong();		// by default, ignore further touches for a long time
						if (bp.GetEvent() != evAdjustVolume)
						{
							TouchBeep();		// give audible feedback of the touch, unless adjusting the volume
						}
						UI::ProcessTouch(bp);
					}
					else
					{
						bp = mgr.FindEventOutsidePopup(x, y);
						if (bp.IsValid())
						{
							UI::ProcessTouchOutsidePopup(bp);
						}
					}
				}
			}
			else if (SystemTick::GetTickCount() - lastActionTime >= DimDisplayTimeout)
			{
				if (!isDimmed && UI::CanDimDisplay()){
					DimBrightness();				// it might not actually dim the display, depending on various flags
				}
				uint32_t screensaverTimeout = GetScreensaverTimeout();
				if (screensaverTimeout > 0 && SystemTick::GetTickCount() - lastActionTime >= screensaverTimeout)
				{
					ActivateScreensaver();
				}
 			}
		}
		ShowLine;

		// 4. Refresh the display
		UpdateDebugInfo();
		mgr.Refresh(false);
		ShowLine;

		// 5. Generate a beep if asked to
		if (beepFrequency != 0 && beepLength != 0)
		{
			if (beepFrequency >= 100 && beepFrequency <= 10000 && beepLength > 0)
			{
				if (beepLength > 20000)
				{
					beepLength = 20000;			// limit the beep to 20 seconds
				}
				Buzzer::Beep(beepFrequency, beepLength, Buzzer::MaxVolume);
			}
			beepFrequency = beepLength = 0;
		}
		ShowLine;

		// 6. If it is time, poll the printer status.
		// When the printer is executing a homing move or other file macro, it may stop responding to polling requests.
		// Under these conditions, we slow down the rate of polling to avoid building up a large queue of them.
		const uint32_t now = SystemTick::GetTickCount();
		if (   UI::DoPolling()										// don't poll while we are in the Setup page
		    && now - lastPollTime >= printerPollInterval			// if we haven't polled the printer too recently...
			&& now - lastResponseTime >= printerResponseInterval	// and we haven't had a response too recently
		   )
		{
			if (now - lastPollTime > now - lastResponseTime)		// if we've had a response since the last poll
			{
				auto nextToPoll = GetNextToPoll();
				if (nextToPoll != nullptr)
				{
					// Once we get here the first time we will work all seqs once
					initialized = true;

					SerialIo::Sendf("M409 K\"%s\" F\"%s\"\n", nextToPoll->key, nextToPoll->flags);
				}
				else {

					// First check for specific info we need to fetch
					bool done = FileManager::ProcessTimers();

					// Otherwise just send a normal poll command
					if (!done)
					{
						SerialIo::SendString("M409 F\"d99f\"\n");
					}
				}
				lastPollTime = SystemTick::GetTickCount();
			}
            else if (now - lastPollTime >= printerPollTimeout)      // last response was most likely incomplete start over
            {
                SerialIo::SendString("M409 F\"d99f\"\n");
                lastPollTime = SystemTick::GetTickCount();
            }
		}
		ShowLine;
	}
}

void PrintDebugText(const char *x)
{
	fwVersionField->SetValue(x);
}

// Pure virtual function call handler, to avoid pulling in large chunks of the standard library
extern "C" void __cxa_pure_virtual() { while (1); }

// End
