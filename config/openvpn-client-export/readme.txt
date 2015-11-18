pfSense OpenVPN Client Export Package
-------------------------------------

This package includes a webConfigurator interface that allows for easy
export of user based OpenVPN configurations and pre-configured windows
installer packages.

Contents
--------
client-export - tgz archive root path
client-export/vpn_openvpn_export.php - pfSense php interface code
client-export/template - installer template path
client-export/template/7zS.sfx - 7zip windows self extractor
client-export/template/config-import - 7zip sfx configuration
client-export/template/config-standard - 7zip sfx configuration
client-export/template/procchain.exe - process chain utility
client-export/template/openvpn-install.exe - openvpn installer
client-export/template/openvpn-postinstall.exe - post installer
client-export/template/procchain-import - procchain configuration
client-export/template/procchain-standard - procchain configuration
client-export/template/config - OpenVPN configuration import path
source/openvpn-postinstall.nsi - post install NSIS script
source/openvpn-postinstall.ico - post install icon
source/procchain.cpp - C++ source for process chain utility
openvpn-client-export.inc - pfSense php pagkage include file
openvpn-client-export.xml - pfSense xml package description

Configuration
-------------
Before the package can be used, place the OpenVPN installer of your
choice in the template directory and name it 'openvpn-install.exe'.
Then use tar to archive the entire client-export directory from the
root package directory using the following command ...

tar zcvf openvpn-client-export.tgz client-export

With the archive created, you will have three relevant files in the
root package directory ...

openvpn-client-export.inc
openvpn-client-export.tgz
openvpn-client-export.xml

These files are the only files required for distribution.
