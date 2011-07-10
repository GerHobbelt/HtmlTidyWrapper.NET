========================================================================
    DYNAMIC LINK LIBRARY : HtmlTidyWrapper Project Overview
========================================================================

This file contains a summary of what you will find in each of the files that
make up your HtmlTidyWrapper application.

HtmlTidyWrapper.vcproj
    This is the main project file. 

HtmlTidyWrapper.cpp
    This is the main DLL source file, written in managed C++.

HtmlTidyWrapper.h
    This file is empty for now.

AssemblyInfo.cpp
	Contains custom attributes for modifying assembly metadata.

/////////////////////////////////////////////////////////////////////////////

Other notes:

HtmlTidyWrapper comes with two sample applications, one written in C#, the other in VB.NET.

Both show some of the basic features of the .NET HtmlTidyWrapper code. The C# example
is a little more extensive and shows the use of the XmlDocument tree as returned by the
wrapper.


IMPORTANT NOTES:

HtmlTidyWrapper requires the use of the hebbut.net patched htmltidy library: that source code
offers additional interface methods and a few other tweaks to the 'tidy' original published by 
W3C/sourceforge.

Visit hebbut.net and go to the 'Public Offerings' section to fetch the additional source code
and/or pre-compiled binaries.


Portability:

HtmlTidyWrapper is .NET (2.0) compatible code and should run on any supported platform. 
However, the libtidy code is 'native', so you must compile/otherwise provide a suitably
built DLL for that (tidydll.dll). That means that when you are running on a 64-bit Windows
platform, you must use another tidydll.dll built than anyone out there running old-fashioned
32-bit Windows.

/////////////////////////////////////////////////////////////////////////////
