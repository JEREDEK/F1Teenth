#ifndef SCREEN_HANDLE
#define SCREEN_HANDLE

void buildScreen();
void buildVoltageScreen();
void screenInit();
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
float getVolt(float analogIn, float R1, float R2, int CellID);
float getCurr(float analogIn);
void drawVolt(char* txt, float analogIn, float R1, float R2, int x, int y, int CellID);
void drawVoltBig(float analogIn, float R1, float R2, int x, int y, int CellID);
void checkForWarnings();
void buzzer();
void drawLines();
void drawSoC(char* txt, float analogIn, float R1, float R2, int x, int y);

#endif
