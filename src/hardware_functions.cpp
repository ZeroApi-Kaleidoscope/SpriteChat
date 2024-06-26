#include "hardware_functions.h"

#include <QDebug>
#include <QtGlobal>

#include <stdio.h>

#if (defined(_WIN32) || defined(_WIN64))
// clang-format off
#include <windows.h>
#include <sddl.h>
// clang-format on

QString get_hdid()
{
  HANDLE hToken;
  HANDLE pHandle;
  PTOKEN_USER pToken;
  DWORD uSize = 0;
  LPWSTR HDIDParam;

  pHandle = GetCurrentProcess();
  OpenProcessToken(pHandle, TOKEN_QUERY, &hToken);
  if (!GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)1, NULL, 0, &uSize))
  {
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
      CloseHandle(hToken);
      return "gxsps32sa9fnwic92mfbs1";
    }
  }

  pToken = (PTOKEN_USER)GlobalAlloc(GPTR, uSize);

  if (!GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)1, pToken, uSize, &uSize))
  {
    CloseHandle(hToken);
    return "gxsps32sa9fnwic92mfbs2";
  }

  ConvertSidToStringSidW(pToken->User.Sid, &HDIDParam);
  QString returnHDID = QString::fromWCharArray(HDIDParam);
  CloseHandle(hToken);
  return returnHDID;
}
#elif defined(ANDROID)
#include <QtAndroidExtras/QtAndroid>

QString get_hdid()
{
  QAndroidJniObject appctx = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;").callObjectMethod("getApplicationContext", "()Landroid/content/Context;");
  QAndroidJniObject androidId = QAndroidJniObject::callStaticObjectMethod("android/provider/Settings$Secure", "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;", appctx.callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;").object<jobject>(), QAndroidJniObject::fromString("android_id").object<jstring>());
  return androidId.toString();
}
#elif QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
#if (defined(LINUX) || defined(__linux__))
#include <QSysInfo>

QByteArray machineId;

QString get_hdid()
{
  machineId = QSysInfo::machineUniqueId();

  if (machineId.isEmpty())
  {
    return "gxsps32sa9fnwic92mfbs2";
  }
  return QString(machineId);
}

#elif defined __APPLE__

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

QString get_hdid()
{
  CFStringRef serial;
  char buffer[64] = {0};
  QString hdid;
  io_service_t platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("IOPlatformExpertDevice"));
  if (platformExpert)
  {
    CFTypeRef serialNumberAsCFString = IORegistryEntryCreateCFProperty(platformExpert, CFSTR(kIOPlatformSerialNumberKey), kCFAllocatorDefault, 0);
    if (serialNumberAsCFString)
    {
      serial = (CFStringRef)serialNumberAsCFString;
    }
    if (CFStringGetCString(serial, buffer, 64, kCFStringEncodingUTF8))
    {
      hdid = buffer;
    }

    IOObjectRelease(platformExpert);
  }
  return hdid;
}

#else

#error This operating system is unsupported for hardware functions.

#endif

#else
#include <QSysInfo>

QByteArray machineId;

QString get_hdid()
{
  machineId = QSysInfo::machineUniqueId();
  return QString(machineId);
}

#endif
