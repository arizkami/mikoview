#!/usr/bin/env python3
"""
MikoView Installer Builder
Creates NSIS installers for Windows and .deb packages for Linux
"""

import os
import sys
import shutil
import subprocess
import argparse
import json
import tempfile
from pathlib import Path
from typing import Dict, List, Optional

class InstallerBuilder:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root).resolve()
        self.build_dir = self.project_root / "build"
        self.dist_dir = self.project_root / "dist"
        self.tools_dir = self.project_root / "tools"
        
        # Project metadata
        self.app_name = "MikoView"
        self.app_version = "1.0.0"
        self.app_description = "Modern desktop application framework"
        self.app_vendor = "MikoView Team"
        self.app_url = "https://github.com/arizkami/mikoview"
        self.app_license = "MIT"
        
    def ensure_directories(self):
        """Create necessary directories"""
        self.dist_dir.mkdir(exist_ok=True)
        (self.dist_dir / "windows").mkdir(exist_ok=True)
        (self.dist_dir / "linux").mkdir(exist_ok=True)
        
    def get_version_from_cmake(self) -> str:
        """Extract version from CMakeLists.txt"""
        cmake_file = self.project_root / "CMakeLists.txt"
        if cmake_file.exists():
            with open(cmake_file, 'r') as f:
                content = f.read()
                # Look for project version
                import re
                match = re.search(r'project\([^)]*VERSION\s+(\d+\.\d+\.\d+)', content, re.IGNORECASE)
                if match:
                    return match.group(1)
        return self.app_version
    
    def create_nsis_installer(self, architecture: str = "x64") -> bool:
        """Create NSIS installer for Windows"""
        print(f"Creating NSIS installer for {architecture}...")
        
        # Check if NSIS is available
        try:
            subprocess.run(["makensis", "/VERSION"], check=True, capture_output=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("Error: NSIS not found. Please install NSIS and add it to PATH.")
            return False
        
        # Create NSIS script
        nsis_script = self.create_nsis_script(architecture)
        nsis_file = self.dist_dir / "windows" / f"{self.app_name}-installer.nsi"
        
        with open(nsis_file, 'w', encoding='utf-8') as f:
            f.write(nsis_script)
        
        # Build installer
        try:
            result = subprocess.run([
                "makensis",
                "/DVERSION=" + self.get_version_from_cmake(),
                "/DARCH=" + architecture,
                str(nsis_file)
            ], check=True, capture_output=True, text=True)
            
            print(f"NSIS installer created successfully!")
            print(f"Output: {self.dist_dir / 'windows' / f'{self.app_name}-{self.get_version_from_cmake()}-{architecture}-setup.exe'}")
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"Error creating NSIS installer: {e}")
            print(f"NSIS output: {e.stdout}")
            print(f"NSIS errors: {e.stderr}")
            return False
    
    def create_nsis_script(self, architecture: str) -> str:
        """Generate NSIS installer script"""
        version = self.get_version_from_cmake()
        
        return f'''
; MikoView NSIS Installer Script
; Generated automatically by makeinstaller.py

!define APP_NAME "{self.app_name}"
!define APP_VERSION "{version}"
!define APP_DESCRIPTION "{self.app_description}"
!define APP_VENDOR "{self.app_vendor}"
!define APP_URL "{self.app_url}"
!define APP_LICENSE "{self.app_license}"
!define ARCH "{architecture}"

; Installer properties
Name "${{APP_NAME}} ${{APP_VERSION}}"
OutFile "${{APP_NAME}}-${{APP_VERSION}}-${{ARCH}}-setup.exe"
InstallDir "$PROGRAMFILES64\\${{APP_NAME}}"
InstallDirRegKey HKLM "Software\\${{APP_VENDOR}}\\${{APP_NAME}}" "InstallDir"
RequestExecutionLevel admin

; Modern UI
!include "MUI2.nsh"
!include "x64.nsh"

; Interface settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\\..\\example\\assets\\icon.ico"
!define MUI_UNICON "..\\..\\example\\assets\\icon.ico"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\\..\\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Languages
!insertmacro MUI_LANGUAGE "English"

; Version information
VIProductVersion "${{APP_VERSION}}.0"
VIAddVersionKey "ProductName" "${{APP_NAME}}"
VIAddVersionKey "ProductVersion" "${{APP_VERSION}}"
VIAddVersionKey "CompanyName" "${{APP_VENDOR}}"
VIAddVersionKey "FileDescription" "${{APP_DESCRIPTION}}"
VIAddVersionKey "FileVersion" "${{APP_VERSION}}"
VIAddVersionKey "LegalCopyright" "© 2024 ${{APP_VENDOR}}"

; Installer sections
Section "Core Application" SecCore
    SectionIn RO
    
    SetOutPath "$INSTDIR"
    
    ; Main executable and libraries
    File "..\\..\\build\\Release\\${{APP_NAME}}.exe"
    File "..\\..\\build\\Release\\*.dll"
    
    ; CEF files
    File /r "..\\..\\build\\Release\\cef\\"
    
    ; Resources
    File /r "..\\..\\build\\Release\\resources\\"
    
    ; Example application
    File "..\\..\\build\\Release\\example.exe"
    
    ; Create uninstaller
    WriteUninstaller "$INSTDIR\\Uninstall.exe"
    
    ; Registry entries
    WriteRegStr HKLM "Software\\${{APP_VENDOR}}\\${{APP_NAME}}" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "Software\\${{APP_VENDOR}}\\${{APP_NAME}}" "Version" "${{APP_VERSION}}"
    
    ; Add/Remove Programs entry
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${{APP_NAME}}" "DisplayName" "${{APP_NAME}} ${{APP_VERSION}}"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${{APP_NAME}}" "UninstallString" "$INSTDIR\\Uninstall.exe"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${{APP_NAME}}" "DisplayIcon" "$INSTDIR\\${{APP_NAME}}.exe"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${{APP_NAME}}" "Publisher" "${{APP_VENDOR}}"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${{APP_NAME}}" "DisplayVersion" "${{APP_VERSION}}"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${{APP_NAME}}" "URLInfoAbout" "${{APP_URL}}"
    WriteRegDWORD HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${{APP_NAME}}" "NoModify" 1
    WriteRegDWORD HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${{APP_NAME}}" "NoRepair" 1
SectionEnd

Section "Desktop Shortcut" SecDesktop
    CreateShortcut "$DESKTOP\\${{APP_NAME}}.lnk" "$INSTDIR\\${{APP_NAME}}.exe"
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
    CreateDirectory "$SMPROGRAMS\\${{APP_NAME}}"
    CreateShortcut "$SMPROGRAMS\\${{APP_NAME}}\\${{APP_NAME}}.lnk" "$INSTDIR\\${{APP_NAME}}.exe"
    CreateShortcut "$SMPROGRAMS\\${{APP_NAME}}\\Example App.lnk" "$INSTDIR\\example.exe"
    CreateShortcut "$SMPROGRAMS\\${{APP_NAME}}\\Uninstall.lnk" "$INSTDIR\\Uninstall.exe"
SectionEnd

Section "Development Headers" SecDev
    SetOutPath "$INSTDIR\\include"
    File /r "..\\..\\mikoview\\*.hpp"
    
    SetOutPath "$INSTDIR\\lib"
    File "..\\..\\build\\Release\\mikoview_framework.lib"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${{SecCore}} "Core application files (required)"
    !insertmacro MUI_DESCRIPTION_TEXT ${{SecDesktop}} "Create desktop shortcut"
    !insertmacro MUI_DESCRIPTION_TEXT ${{SecStartMenu}} "Create start menu shortcuts"
    !insertmacro MUI_DESCRIPTION_TEXT ${{SecDev}} "Development headers and libraries"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Uninstaller
Section "Uninstall"
    ; Remove files
    Delete "$INSTDIR\\${{APP_NAME}}.exe"
    Delete "$INSTDIR\\example.exe"
    Delete "$INSTDIR\\*.dll"
    Delete "$INSTDIR\\Uninstall.exe"
    
    ; Remove directories
    RMDir /r "$INSTDIR\\cef"
    RMDir /r "$INSTDIR\\resources"
    RMDir /r "$INSTDIR\\include"
    RMDir /r "$INSTDIR\\lib"
    RMDir "$INSTDIR"
    
    ; Remove shortcuts
    Delete "$DESKTOP\\${{APP_NAME}}.lnk"
    RMDir /r "$SMPROGRAMS\\${{APP_NAME}}"
    
    ; Remove registry entries
    DeleteRegKey HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${{APP_NAME}}"
    DeleteRegKey HKLM "Software\\${{APP_VENDOR}}\\${{APP_NAME}}"
SectionEnd

; Functions
Function .onInit
    ; Check if 64-bit Windows
    ${{If}} "${{ARCH}}" == "x64"
        ${{IfNot}} ${{RunningX64}}
            MessageBox MB_OK "This installer requires 64-bit Windows."
            Abort
        ${{EndIf}}
    ${{EndIf}}
FunctionEnd
'''
    
    def create_deb_package(self, architecture: str = "amd64") -> bool:
        """Create .deb package for Linux"""
        print(f"Creating .deb package for {architecture}...")
        
        # Check if dpkg-deb is available
        try:
            subprocess.run(["dpkg-deb", "--version"], check=True, capture_output=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("Error: dpkg-deb not found. Please install dpkg-dev.")
            return False
        
        version = self.get_version_from_cmake()
        package_name = f"{self.app_name.lower()}"
        package_dir = self.dist_dir / "linux" / f"{package_name}_{version}_{architecture}"
        
        # Clean and create package directory
        if package_dir.exists():
            shutil.rmtree(package_dir)
        package_dir.mkdir(parents=True)
        
        # Create directory structure
        debian_dir = package_dir / "DEBIAN"
        usr_dir = package_dir / "usr"
        bin_dir = usr_dir / "bin"
        lib_dir = usr_dir / "lib" / package_name
        share_dir = usr_dir / "share"
        doc_dir = share_dir / "doc" / package_name
        applications_dir = share_dir / "applications"
        icons_dir = share_dir / "icons" / "hicolor" / "256x256" / "apps"
        
        for dir_path in [debian_dir, bin_dir, lib_dir, doc_dir, applications_dir, icons_dir]:
            dir_path.mkdir(parents=True, exist_ok=True)
        
        # Create control file
        control_content = self.create_deb_control(package_name, version, architecture)
        with open(debian_dir / "control", 'w') as f:
            f.write(control_content)
        
        # Create postinst script
        postinst_content = self.create_deb_postinst()
        postinst_file = debian_dir / "postinst"
        with open(postinst_file, 'w') as f:
            f.write(postinst_content)
        postinst_file.chmod(0o755)
        
        # Create prerm script
        prerm_content = self.create_deb_prerm()
        prerm_file = debian_dir / "prerm"
        with open(prerm_file, 'w') as f:
            f.write(prerm_content)
        prerm_file.chmod(0o755)
        
        # Copy application files
        build_release = self.build_dir / "Release"
        if not build_release.exists():
            build_release = self.build_dir
        
        # Main executable
        exe_src = build_release / f"{self.app_name}"
        if exe_src.exists():
            shutil.copy2(exe_src, lib_dir / f"{self.app_name}")
            (lib_dir / f"{self.app_name}").chmod(0o755)
        
        # Example executable
        example_src = build_release / "example"
        if example_src.exists():
            shutil.copy2(example_src, lib_dir / "example")
            (lib_dir / "example").chmod(0o755)
        
        # Libraries and resources
        for pattern in ["*.so*", "cef", "resources"]:
            for item in build_release.glob(pattern):
                if item.is_file():
                    shutil.copy2(item, lib_dir)
                elif item.is_dir():
                    shutil.copytree(item, lib_dir / item.name, dirs_exist_ok=True)
        
        # Create launcher script
        launcher_script = f'''#!/bin/bash
cd /usr/lib/{package_name}
exec ./{self.app_name} "$@"
'''
        launcher_file = bin_dir / package_name
        with open(launcher_file, 'w') as f:
            f.write(launcher_script)
        launcher_file.chmod(0o755)
        
        # Desktop file
        desktop_content = self.create_desktop_file(package_name)
        with open(applications_dir / f"{package_name}.desktop", 'w') as f:
            f.write(desktop_content)
        
        # Icon
        icon_src = self.project_root / "example" / "assets" / "icon.png"
        if icon_src.exists():
            shutil.copy2(icon_src, icons_dir / f"{package_name}.png")
        
        # Documentation
        for doc_file in ["LICENSE", "README.md"]:
            doc_src = self.project_root / doc_file
            if doc_src.exists():
                shutil.copy2(doc_src, doc_dir)
        
        # Create copyright file
        copyright_content = f'''Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: {self.app_name}
Upstream-Contact: {self.app_vendor}
Source: {self.app_url}

Files: *
Copyright: 2024 {self.app_vendor}
License: {self.app_license}

License: {self.app_license}
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 .
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 .
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
'''
        with open(doc_dir / "copyright", 'w') as f:
            f.write(copyright_content)
        
        # Build package
        deb_file = self.dist_dir / "linux" / f"{package_name}_{version}_{architecture}.deb"
        try:
            subprocess.run([
                "dpkg-deb", "--build", "--root-owner-group",
                str(package_dir), str(deb_file)
            ], check=True, capture_output=True)
            
            print(f".deb package created successfully!")
            print(f"Output: {deb_file}")
            
            # Verify package
            result = subprocess.run(["dpkg-deb", "--info", str(deb_file)], 
                                  capture_output=True, text=True)
            print("Package info:")
            print(result.stdout)
            
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"Error creating .deb package: {e}")
            return False
    
    def create_deb_control(self, package_name: str, version: str, architecture: str) -> str:
        """Generate Debian control file"""
        return f'''Package: {package_name}
Version: {version}
Section: devel
Priority: optional
Architecture: {architecture}
Depends: libc6, libstdc++6, libx11-6, libxrandr2, libxss1, libgconf-2-4, libxcomposite1, libxcursor1, libxdamage1, libxi6, libxtst6, libnss3, libcups2, libxrandr2, libasound2, libpangocairo-1.0-0, libatk1.0-0, libcairo-gobject2, libgtk-3-0, libgdk-pixbuf2.0-0
Maintainer: {self.app_vendor}
Description: {self.app_description}
 {self.app_description}
 .
 MikoView is a modern desktop application framework that combines
 the power of CEF (Chromium Embedded Framework) with SDL for creating
 cross-platform desktop applications using web technologies.
Homepage: {self.app_url}
'''
    
    def create_deb_postinst(self) -> str:
        """Generate Debian postinst script"""
        return '''#!/bin/bash
set -e

# Update desktop database
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database -q
fi

# Update icon cache
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -q -t -f /usr/share/icons/hicolor
fi

exit 0
'''
    
    def create_deb_prerm(self) -> str:
        """Generate Debian prerm script"""
        return '''#!/bin/bash
set -e

exit 0
'''
    
    def create_desktop_file(self, package_name: str) -> str:
        """Generate .desktop file"""
        return f'''[Desktop Entry]
Version=1.0
Type=Application
Name={self.app_name}
Comment={self.app_description}
Exec={package_name}
Icon={package_name}
Terminal=false
Categories=Development;IDE;
Keywords=framework;desktop;web;development;
StartupNotify=true
'''
    
    def build_project(self, config: str = "Release") -> bool:
        """Build the project before packaging"""
        print(f"Building project in {config} mode...")
        
        if not self.build_dir.exists():
            self.build_dir.mkdir()
        
        # Configure with CMake
        try:
            subprocess.run([
                "cmake", "-S", str(self.project_root), "-B", str(self.build_dir),
                f"-DCMAKE_BUILD_TYPE={config}"
            ], check=True)
            
            # Build
            subprocess.run([
                "cmake", "--build", str(self.build_dir), 
                "--config", config, "--parallel"
            ], check=True)
            
            print("Project built successfully!")
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"Error building project: {e}")
            return False
    
    def clean_dist(self):
        """Clean distribution directory"""
        if self.dist_dir.exists():
            shutil.rmtree(self.dist_dir)
        self.ensure_directories()

def main():
    parser = argparse.ArgumentParser(description="MikoView Installer Builder")
    parser.add_argument("--platform", choices=["windows", "linux", "all"], 
                       default="all", help="Target platform")
    parser.add_argument("--arch", choices=["x64", "x86", "amd64", "i386"], 
                       default="x64", help="Target architecture")
    parser.add_argument("--build", action="store_true", 
                       help="Build project before packaging")
    parser.add_argument("--clean", action="store_true", 
                       help="Clean distribution directory")
    parser.add_argument("--config", choices=["Debug", "Release"], 
                       default="Release", help="Build configuration")
    
    args = parser.parse_args()
    
    # Get project root (parent of tools directory)
    project_root = Path(__file__).parent.parent
    builder = InstallerBuilder(project_root)
    
    if args.clean:
        print("Cleaning distribution directory...")
        builder.clean_dist()
    
    builder.ensure_directories()
    
    if args.build:
        if not builder.build_project(args.config):
            sys.exit(1)
    
    success = True
    
    if args.platform in ["windows", "all"]:
        if sys.platform == "win32" or args.platform == "windows":
            arch = "x64" if args.arch in ["x64", "amd64"] else "x86"
            if not builder.create_nsis_installer(arch):
                success = False
        else:
            print("Skipping Windows installer (not on Windows platform)")
    
    if args.platform in ["linux", "all"]:
        if sys.platform.startswith("linux") or args.platform == "linux":
            arch = "amd64" if args.arch in ["x64", "amd64"] else "i386"
            if not builder.create_deb_package(arch):
                success = False
        else:
            print("Skipping Linux package (not on Linux platform)")
    
    if success:
        print("\nInstaller creation completed successfully!")
        print(f"Output directory: {builder.dist_dir}")
    else:
        print("\nSome installers failed to create.")
        sys.exit(1)

if __name__ == "__main__":
    main()