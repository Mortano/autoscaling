#pragma once

#ifdef EXPORT_AS_API
#define AS_API __declspec(dllexport)
#else
#define AS_API __declspec(dllimport)
#endif
