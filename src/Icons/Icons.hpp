/*
 * Icons.h
 *
 * Created: 29/11/2015 12:11:34
 *  Author: David
 */


#ifndef ICONS_H_
#define ICONS_H_

extern const uint16_t IconPaletteLight[];
extern const uint16_t IconPaletteDark[];

extern const uint8_t IconNozzle1[], IconNozzle2[], IconNozzle3[], IconNozzle4[];

#if DISPLAY_X == 800
extern const uint8_t IconNozzle5[], IconNozzle6[];
#endif

extern const uint8_t IconHomeAll[], IconHomeX[], IconHomeY[], IconHomeZ[], IconHomeU[], IconBedComp[];

#if DISPLAY_X == 800
extern const uint8_t IconHomeV[], IconHomeW[];
#endif

extern const uint8_t IconBed[], IconOk[], IconCancel[], IconFiles[], IconKeyboard[], IconTrash[];
extern const uint8_t IconBackspace[], IconUp[], IconDown[], IconEnter[];

extern const uint8_t Xmax2min[], Xmin2max[], Ymax2min[], Ymin2max[], Zmax2min[];

extern const uint8_t SetToolOffsetXY[], SetToolOffsetZ[];

#endif /* ICONS_H_ */
