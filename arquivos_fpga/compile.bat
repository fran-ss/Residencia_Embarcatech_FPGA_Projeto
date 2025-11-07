@echo off
set OSSCAD=C:\OSS-CAD-SUITE
set TOP=top
set LPF=motor_controller.lpf

:: Garante que o ambiente OSS-CAD-SUITE esteja carregado
call "%OSSCAD%\environment.bat"

:: Muda para o diretório atual do script
cd /d %~dp0

echo [1/4] Synth (Yosys)
yosys -p "read_verilog -sv %TOP%.sv; synth_ecp5 -top %TOP% -json %TOP%.json"

IF ERRORLEVEL 1 GOTO :SYNTH_ERROR

echo.
echo [2/4] P (Nextpnr)
:: --45k e CABGA381 são as opções corretas para Colorlight i9
nextpnr-ecp5 --json "%TOP%.json" --textcfg "%TOP%.config" --lpf "%LPF%" --45k --package CABGA381 --speed 6

IF ERRORLEVEL 1 GOTO :PNR_ERROR

echo.
echo [3/4] Pack (Ecppack)
ecppack --compress "%TOP%.config" "%TOP%.bit"

IF ERRORLEVEL 1 GOTO :PACK_ERROR

echo.
echo [4/4] Program (RAM)
:: Grava na SRAM. Use -f para forçar (flash) se quiser gravar na memória flash.
openFPGALoader -b colorlight-i9 "%TOP%.bit"

IF ERRORLEVEL 1 GOTO :PROGRAM_WARNING

GOTO :END

:SYNTH_ERROR
echo.
echo ERRO: Falha na Síntese. Verifique o arquivo %TOP%.sv.
GOTO :END

:PNR_ERROR
echo.
echo ERRO: Falha no Place & Route. Verifique o arquivo %LPF%.
GOTO :END

:PACK_ERROR
echo.
echo ERRO: Falha ao gerar Bitstream.

:PROGRAM_WARNING
echo.
echo AVISO: Falha na gravação do FPGA. Verifique a conexão JTAG/USB e os drivers.
echo         O arquivo %TOP%.bit foi gerado com sucesso.

:END
echo.
echo === DONE ===