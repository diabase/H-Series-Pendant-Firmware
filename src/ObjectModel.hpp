/*
 * ObjectModel.hpp
 *
 *  Created on: 7 Sep 2020
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_HPP_
#define SRC_OBJECTMODEL_HPP_

#include <cstdint>

#undef array
#include <functional>
#define array _ecv_array // Reinstate the eCv definitions of 'array'

#include "ToolStatus.hpp"
#include "UserInterfaceConstants.hpp"

namespace OM {
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
		uint8_t index = 0;
		float babystep = 0.0f;
		char letter[2] = {'\0', '\0'};
		float workplaceOffsets[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
		uint16_t homed : 1,
			visible : 1,
			slot : 6,
			slotP : 6;
		Axis* next = nullptr;
	};

	struct Spindle
	{
		// Index within configured spindles
		uint8_t index = 0;
		float active = 0.0;
		float max = 0.0;
		int8_t tool = -1;
		Spindle* next = nullptr;
	};

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

	struct BedOrChamber
	{
		// Index within configured heaters
		uint8_t index = 0;
		// Id of heater
		int8_t heater = -1;
		// Slot for display on panel
		uint8_t slot = MaxHeaters;
		uint8_t slotP = MaxPendantTools;
	};

	Axis* FindAxis(std::function<bool(Axis*)> filter);
	Axis* GetAxis(size_t index);
	Axis* GetAxisInSlot(size_t slot);
	Axis* GetOrCreateAxis(size_t index);
	void IterateAxes(std::function<void(Axis*)> func);
	bool IterateAxesWhile(std::function<bool(Axis*)> func);

	Spindle* GetSpindle(size_t index);
	Spindle* GetOrCreateSpindle(size_t index);
	Spindle* GetSpindleForTool(size_t toolNumber);
	void IterateSpindles(std::function<void(Spindle*)> func);
	bool IterateSpindlesWhile(std::function<bool(Spindle*)> func);

	Tool* GetTool(size_t index);
	Tool* GetOrCreateTool(size_t index);
	Tool* GetToolForExtruder(size_t extruder);
	Tool* GetToolForHeater(size_t heater);
	void IterateTools(std::function<void(Tool*)> func);
	bool IterateToolsWhile(std::function<bool(Tool*)> func);

	void RemoveAxis(size_t index, bool allFollowing);
	void RemoveSpindle(size_t index, bool allFollowing);
	void RemoveTool(size_t index, bool allFollowing);
}


#endif /* SRC_OBJECTMODEL_HPP_ */
