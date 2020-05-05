@echo off
cls
pushd %~dp0
	devtools\bin\vpc.exe /dmc +game /mksln dmcsource.sln /2013
popd
@pause