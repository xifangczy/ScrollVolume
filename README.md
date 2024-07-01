# ScrollVolume

使用鼠标滚轮调节音量

Adjust the volume using the mouse wheel.

# 使用方法 Usage

打开程序，后台监听快捷键 默认：ctrl+shift+alt+V 按下后 鼠标滚轮会变成音量调节。

Open the program, it will listen for the hotkey in the background. Default: ctrl+shift+alt+V. After pressing it, the mouse wheel will become a volume control.


# 参数 Parameters

## -hotkey

定义快捷键 Define the hotkey.

示例 Example:

ScrollVolume.exe -hotkey CONTROL+SHIFT+m

就能改变快捷键为 ctrl+shift+m

This will change the hotkey to ctrl+shift+m

## -taskbar

开关 鼠标放入任务栏 可滚轮调节音量功能

Toggle the feature to adjust the volume using the mouse wheel when the cursor is over the taskbar.


示例 Example:

ScrollVolume.exe -taskbar on

ScrollVolume.exe -taskbar off

# 开机启动 Startup

把程序放入目录内 Place the program in the directory
C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp
