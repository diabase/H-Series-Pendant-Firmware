/*
 * ProbeWorkpieceIcons.cpp
 *
 * Created: 2020-05-13
 *  Author: Manuel
 */

#include "asf.h"
#include "Configuration.hpp"
#include "Icons.hpp"

#if LARGE_FONT

extern const uint8_t Xmax2min[] =
{	30, 30,	// width, height
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 
	0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 
	0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 
	0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x11, 0x11, 0x11, 0x10, 0x03, 0x30, 0x00, 0x33, 0x00, 0x33, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x03, 0x33, 0x00, 0x00, 0x03, 0x33, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x03, 0x33, 0x30, 0x00, 0x33, 0x33, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x33, 0x33, 0x03, 0x33, 0x30, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x03, 0x33, 0x33, 0x33, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x33, 0x33, 0x30, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x03, 0x33, 0x33, 0x33, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x33, 0x33, 0x03, 0x33, 0x30, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x03, 0x33, 0x30, 0x00, 0x33, 0x33, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x03, 0x33, 0x00, 0x00, 0x03, 0x33, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x03, 0x30, 0x00, 0x00, 0x00, 0x33, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
};
extern const uint8_t Xmin2max[] =
{	30, 30,	// width, height
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x33, 0x00, 0x00, 0x00, 0x03, 0x30, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x33, 0x30, 0x00, 0x00, 0x33, 0x30, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x33, 0x33, 0x00, 0x03, 0x33, 0x30, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x03, 0x33, 0x30, 0x33, 0x33, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x33, 0x33, 0x33, 0x30, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x03, 0x33, 0x33, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x33, 0x33, 0x33, 0x30, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x03, 0x33, 0x30, 0x33, 0x33, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x33, 0x33, 0x00, 0x03, 0x33, 0x30, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x33, 0x30, 0x00, 0x00, 0x33, 0x30, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x33, 0x00, 0x00, 0x00, 0x03, 0x30, 0x01, 0x11, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 
	0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 
	0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 
	0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
};
extern const uint8_t Ymax2min[] =
{	30, 30,	// width, height
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 
	0x00, 0x00, 0x33, 0x00, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 
	0x00, 0x00, 0x33, 0x30, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 
	0x00, 0x00, 0x33, 0x33, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 
	0x00, 0x00, 0x03, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0x33, 0x33, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0x00, 0x00, 0x03, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 
	0x00, 0x00, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 
	0x00, 0x00, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 
	0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
};
extern const uint8_t Ymin2max[] =
{	30, 30,	// width, height
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 
	0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 
	0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 
	0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 
	0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 
	0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
};
extern const uint8_t Zmax2min[] =
{	30, 30,	// width, height
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x03, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 
	0x00, 0x00, 0x03, 0x30, 0x03, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 
	0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 
	0x00, 0x00, 0x03, 0x30, 0x00, 0x03, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0x00, 0x00, 0x03, 0x30, 0x00, 0x33, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0x30, 0x03, 0x33, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0x30, 0x33, 0x30, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0x00, 0x00, 0x03, 0x33, 0x33, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 
	0x00, 0x00, 0x03, 0x33, 0x30, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 
	0x00, 0x00, 0x03, 0x33, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 
	0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
};

#else

#endif