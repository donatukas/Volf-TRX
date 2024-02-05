cls
echo "Move print.h before run"
cppcheck . --cppcheck-build-dir=.cppcheck --showtime=summary --enable=all --force --platform=unix32 --suppressions-list=.cppcheck-suppressions -I Src -I Src\USBDevice -I WOLF\Inc -I WOLF-2\Inc -I WOLF-2\Src --file-filter=Src/* --file-filter=WOLF/Src/* --file-filter=WOLF-2/Src/* -UFRONTPANEL_BIG_V1 -UFRONTPANEL_LITE -UFRONTPANEL_LITE_V2_MINI -UFRONTPANEL_MINI -UFRONTPANEL_NONE -UFRONTPANEL_SMALL_V1 -UFRONTPANEL_WF_100D -UFRONTPANEL_X1 -ULAY_160x128 -ULAY_320x240 -ULAY_480x320 -USTM32F407xx -ULCD_HX8357B -ULCD_HX8357C -ULCD_ILI9341 -ULCD_ILI9481 -ULCD_ILI9486 -ULCD_NONE -ULCD_ST7735S -ULCD_ST7789 -ULCD_ILI9481_IPS -ULCD_SLOW 2> cppcheck-errors.txt
pause
