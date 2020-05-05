This is the source code of Deathmatch Classic: Source.



Mapbase - Source 2013

This repository contains the code and game files of the Source 2013 modification known as Mapbase.

The projected texture fixes within the "ASW_PROJECTED_TEXTURES" preprocessor and most of the shader changes involving projected textures are cumulative changes from Insolence's repository using fixes from Alien Swarm, G-String, and City 17: Episode One. (https://github.com/95Navigator/insolence-2013, https://github.com/Biohazard90/g-string_2013, https://github.com/KyleGospo/City-17-Episode-One-Source)
The original code for phong reflections on LightmappedGeneric-derived shaders are from City 17: Episode One. (https://github.com/KyleGospo/City-17-Episode-One-Source)
The Alien Swarm-based radial fog and rope code as well as the multiple skybox support are from Half-Life 2: Downfall. (https://github.com/DownFall-Team/DownFall)
The dynamic RTT shadow angles code is from Saul Rennison on the VDC. (https://developer.valvesoftware.com/wiki/Dynamic_RTT_shadow_angles_in_Source_2007)
The vortigaunt LOS fix is from Half-Life 2: Community Edition (dky.tehkingd.u in particular). (https://gitlab.com/RaraCerberus/HL2CE)
The parallax corrected cubemap code (which is partly in Mapbase's code, but not yet usable) was originally created by Brian Charles. (https://developer.valvesoftware.com/wiki/Parallax_Corrected_Cubemaps)
Various other code and contributions were based off of pull requests in the Source 2013 SDK (https://github.com/ValveSoftware/source-sdk-2013/pulls) and snippets on the Valve Developer Community (http://developer.valvesoftware.com/).

All of the work mentioned above was open source when it was borrowed.

More credits can be found here:
https://github.com/mapbase-source/source-sdk-2013/wiki/Mapbase-Credits

Please see the Source SDK 2013 license below:



               SOURCE 1 SDK LICENSE

Source SDK Copyright(c) Valve Corp.  

THIS DOCUMENT DESCRIBES A CONTRACT BETWEEN YOU AND VALVE 
CORPORATION ("Valve").  PLEASE READ IT BEFORE DOWNLOADING OR USING 
THE SOURCE ENGINE SDK ("SDK"). BY DOWNLOADING AND/OR USING THE 
SOURCE ENGINE SDK YOU ACCEPT THIS LICENSE. IF YOU DO NOT AGREE TO 
THE TERMS OF THIS LICENSE PLEASE DON’T DOWNLOAD OR USE THE SDK.  

  You may, free of charge, download and use the SDK to develop a modified Valve game 
running on the Source engine.  You may distribute your modified Valve game in source and 
object code form, but only for free. Terms of use for Valve games are found in the Steam 
Subscriber Agreement located here: http://store.steampowered.com/subscriber_agreement/ 

  You may copy, modify, and distribute the SDK and any modifications you make to the 
SDK in source and object code form, but only for free.  Any distribution of this SDK must 
include this LICENSE file and thirdpartylegalnotices.txt.  
 
  Any distribution of the SDK or a substantial portion of the SDK must include the above 
copyright notice and the following: 

    DISCLAIMER OF WARRANTIES.  THE SOURCE SDK AND ANY 
    OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED 
    "AS IS".  VALVE AND ITS SUPPLIERS DISCLAIM ALL 
    WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS 
    OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED 
    WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, 
    TITLE AND FITNESS FOR A PARTICULAR PURPOSE.  

    LIMITATION OF LIABILITY.  IN NO EVENT SHALL VALVE OR 
    ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
    INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER 
    (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF 
    BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF 
    BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) 
    ARISING OUT OF THE USE OF OR INABILITY TO USE THE 
    ENGINE AND/OR THE SDK, EVEN IF VALVE HAS BEEN 
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
 
       
If you would like to use the SDK for a commercial purpose, please contact Valve at 
sourceengine@valvesoftware.com.
