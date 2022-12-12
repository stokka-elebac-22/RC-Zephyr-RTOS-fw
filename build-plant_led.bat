rmdir /q /s build
cpplint --linelength=120 --filter=-whitespace/tab --recursive .\src\
west build -b particle_xenon
for /f %%i in ('git rev-parse --short^=8 HEAD') do set GITHASH=%%i
scp build\zephyr\zephyr.signed.bin debian@beaglebone.local:\tmp\otafw\10201-%GITHASH%-zephyr.bin
@REM west flash --hex-file bootloader\mcuboot-e73.hex -r nrfjprog
@REM west flash -r nrfjprog
@rem scp build\zephyr\zephyr.signed.bin pi@threadrouter.local:\var\otafw\10202-%GITHASH%-zephyr.bin
