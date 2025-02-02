#ifndef _sunrise_siren_studio_h
#define _sunrise_siren_studio_h

#include <gtk/gtk.h>

GdkRGBA *default_color;
GdkRGBA *alarm_color;
GdkRGBA *custom_colors[5];

// MainWindow
GtkWidget *MainWindow;
// Clock settings tab
// Colors
GtkWidget *DefaultColor;
GtkWidget *AlarmColor;

// Alarms
GtkWidget *AlarmEnable[7];
GtkWidget *AlarmHour[7];
GtkWidget *AlarmMinute[7];

// Advanced
GtkWidget *EnableLeadingZero;
GtkWidget *EnableDST;
GtkWidget *ClockReturn;
GtkWidget *LDRMin;
GtkWidget *LDRMax;

GtkWidget *ClockUpdate;

// Custom mode tab
// Basic configuration
GtkWidget *CustomColor[4];
GtkWidget *CustomColor_Colon;
GtkWidget *CustomDigit[4];
GtkWidget *CustomDigitEntry[4];
GtkWidget *CustomUpdate;

// Custom digit generator
GtkWidget *CustomSegmentNumber[21];
GtkWidget *CustomDigitAbout;
GtkWidget *CustomDigitApplyIndex;
GtkWidget *CustomDigitApply;
GtkWidget *CustomDigitEnableAll;
GtkWidget *CustomDigitDisableAll;

// Miscellaneous tab
// Sensor information
GtkWidget *LDRReading;
GtkWidget *SHT21TemperatureReading;
GtkWidget *SHT21HumidityReading;
GtkWidget *SensorRefresh;

// about/danger zone
GtkWidget *AboutProgram;
GtkWidget *RebootClock;
GtkWidget *Reconfigure;

// ConnectionWindow
GtkWindow *ConnectionWindow;
GtkEntry *ConnectHostname;
GtkEntry *ConnectUsername;
GtkEntry *ConnectPassword;
GtkButton *ConnectConfirm;

// ErrorWindow
GtkWidget *ErrorWindow;
GtkWidget *CloseAfterError;
GtkWidget *ReconfigureAfterError;

static void onActivate(GtkApplication *app, gpointer user_data);

int main(int argc, char* argv[]);

#endif
