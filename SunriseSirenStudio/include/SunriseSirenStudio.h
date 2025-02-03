#ifndef _sunrise_siren_studio_h
#define _sunrise_siren_studio_h

#include <gtk/gtk.h>
#include <curl/curl.h>
#include <json-c/json.h>

char* argv0;

gchar* hostname[32];
gchar* username[32];
gchar* password[32];

json_object *clock_status;
json_object *clock_sensors;

GdkRGBA *default_color;
GdkRGBA *alarm_color;
GdkRGBA *custom_colors[5];

// MainWindow
GtkWindow *MainWindow;
// Clock settings tab
// Colors
GtkButton *DefaultColor;
GtkButton *AlarmColor;

// Alarms
GtkSwitch *AlarmEnable[7];
GtkSpinButton *AlarmHour[7];
GtkSpinButton *AlarmMinute[7];

// Advanced
GtkSwitch *EnableLeadingZero;
GtkSwitch *EnableDST;
GtkSpinButton *ClockReturn;
GtkSpinButton *LDRMin;
GtkSpinButton *LDRMax;

GtkButton *ClockUpdate;

// Custom mode tab
// Basic configuration
GtkButton *CustomColor[4];
GtkButton *CustomColor_Colon;
GtkComboBox *CustomDigit[4];
GtkEntry *CustomDigitEntry[4];
GtkButton *CustomUpdate;

// Custom digit generator
GtkToggleButton *CustomSegmentNumber[21];
GtkButton *CustomDigitAbout;
GtkEntry *CustomDigitApplyIndex;
GtkButton *CustomDigitApply;
GtkButton *CustomDigitEnableAll;
GtkButton *CustomDigitDisableAll;

// Miscellaneous tab
// Sensor information
GtkLabel *LDRReading;
GtkLabel *SHT21TemperatureReading;
GtkLabel *SHT21HumidityReading;
GtkButton *SensorRefresh;

// about/danger zone
GtkButton *AboutProgram;
GtkButton *RebootClock;
GtkButton *Reconfigure;

// ConnectionWindow
GtkWindow *ConnectionWindow;
GtkEntry *ConnectHostname;
GtkEntry *ConnectUsername;
GtkEntry *ConnectPassword;
GtkButton *ConnectConfirm;

// ErrorWindow
GtkWindow *ErrorWindow;
GtkButton *CloseAfterError;
GtkButton *RetryAfterError;
GtkButton *ReconfigureAfterError;

// RebootDialog
GtkDialog *RebootDialog;

enum TargetWindow;

void quit();
void reboot();

static void onActivate(GtkApplication *app, gpointer user_data);

int main(int argc, char* argv[]);

#endif
