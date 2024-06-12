; Define the application info
[Setup]
; Name of the application
AppName=MultiDev-Launcher
; Version of the application
AppVersion=1.0
; Publisher information
AppPublisher=Konstantinos Tourtsakis
AppPublisherURL=https://github.com/KonstantinosTourtsakis/MultiDev-Launcher-Thesis
; Default installation directory (Program Files for x64)
DefaultDirName={commonpf64}\MultiDev-Launcher
; Directory for storing the start menu shortcuts
DefaultGroupName=MultiDev-Launcher
; Output directory for the installer
OutputDir=.
; Output filename
OutputBaseFilename=MultiDev_Launcher_x64_installer
; Setup icon file
SetupIconFile=..\QtWidgetsApplication1\WinControllerNavigator.ico
; Compression method
Compression=lzma
; Solid compression
SolidCompression=yes
; Languages supported
;DefaultLanguage=english
; Use a language dialog
ShowLanguageDialog=yes

; Define the languages
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "greek"; MessagesFile: "compiler:Languages\Greek.isl"

; Define the files to be installed
[Files]
; Source and destination of the main executable
Source: "..\x64\Debug\QtWidgetsApplication1.exe"; DestDir: "{app}"; DestName: "MultiDev-Launcher.exe"; Flags: ignoreversion
Source: "..\x64\Debug\Qt6Cored.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\x64\Debug\Qt6Guid.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\x64\Debug\Qt6Widgetsd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\x64\Debug\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion
; Source and destination of the Read Me PDF file
Source: ".\ReadMe.pdf"; DestDir: "{app}"; Flags: ignoreversion

; Define the shortcuts to be created
[Icons]
; Shortcut in the start menu
Name: "{group}\MultiDev-Launcher"; Filename: "{app}\MultiDev-Launcher.exe"
; Conditional shortcut on the desktop
Name: "{commondesktop}\MultiDev-Launcher"; Filename: "{app}\MultiDev-Launcher.exe"; Tasks: desktopicon

; Define installation tasks
[Tasks]
; Task for creating a desktop icon
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

; Define uninstallation behavior
[UninstallDelete]
; Remove all files in the application directory on uninstall
Type: filesandordirs; Name: "{app}"

; Define post-install actions
[Run]
; Open the Read Me PDF file after installation
Filename: "{app}\ReadMe.pdf"; Description: "View the ReadMe file"; Flags: postinstall shellexec

; Define custom messages (optional, for better user experience)
[CustomMessages]
english.CreateDesktopIcon=Create a &desktop shortcut
english.AdditionalIcons=Additional shortcuts:
greek.CreateDesktopIcon=Δημιουργία συντόμευσης επιφάνειας εργασίας
greek.AdditionalIcons=Επιπρόσθετες συντομεύσεις :
